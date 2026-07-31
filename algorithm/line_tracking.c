#include "algorithm/line_tracking.h"

#include <string.h>

#include "algorithm/pid.h"
#include "config/firmware_config.h"

static const uint16_t gWhite[LINE_SENSOR_COUNT] = {
    1921U, 1514U, 1830U, 1604U, 1850U, 1607U
};
static const uint16_t gBlack[LINE_SENSOR_COUNT] = {
    3038U, 2797U, 3242U, 2400U, 2899U, 2336U
};
static const float gPosition[LINE_SENSOR_COUNT] = {
    -1.0f, -0.6f, -0.2f, 0.2f, 0.6f, 1.0f
};

static LineTrackingStatus gStatus;
static PidController gShadowPid;
static bool gNeedsReacquire;

static float normalize(uint16_t value, uint16_t white, uint16_t black)
{
    float result;

    if (value <= white) return 0.0f;
    if (value >= black) return 1.0f;
    result = (float) (value - white) / (float) (black - white);
    return result;
}

void LineTracking_init(uint32_t nowMs)
{
    const PidConfig config = {
        .kp = LINE_TRACKING_SHADOW_KP,
        .ki = LINE_TRACKING_SHADOW_KI,
        .kd = LINE_TRACKING_SHADOW_KD,
        .outputMin = -1.0f,
        .outputMax = 1.0f,
        .integralMin = -0.5f,
        .integralMax = 0.5f,
        .derivativeAlpha = 0.8f,
    };

    memset(&gStatus, 0, sizeof(gStatus));
    gStatus.lastUpdateMs = nowMs;
    gStatus.lineLost = true;
    gNeedsReacquire = true;
    Pid_init(&gShadowPid, &config);
}

void LineTracking_service(uint32_t nowMs)
{
    LineSensorSample sample = LineSensors_snapshot();
    float weighted = 0.0f;
    float signal = 0.0f;
    float dtSeconds;
    uint8_t index;

    if (sample.errorCount != gStatus.sensorErrors) {
        gStatus.sensorErrors = sample.errorCount;
        memset(gStatus.normalized, 0, sizeof(gStatus.normalized));
        gStatus.position = 0.0f;
        gStatus.centeredError = 0.0f;
        gStatus.confidence = 0.0f;
        gStatus.signalSum = 0.0f;
        gStatus.shadowCorrection = 0.0f;
        gStatus.valid = false;
        gStatus.lineLost = true;
        gStatus.lastUpdateMs = nowMs;
        gNeedsReacquire = true;
    }
    if (sample.sequence == gStatus.sequence) return;

    for (index = 0U; index < LINE_SENSOR_COUNT; index++) {
        gStatus.normalized[index] = normalize(
            sample.analog[index], gWhite[index], gBlack[index]);
        weighted += gStatus.normalized[index] * gPosition[index];
        signal += gStatus.normalized[index];
    }

    gStatus.sequence = sample.sequence;
    gStatus.sensorErrors = sample.errorCount;
    gStatus.signalSum = signal;
    gStatus.confidence = signal / (float) LINE_SENSOR_COUNT;
    gStatus.valid = sample.valid;
    gStatus.lineLost = !sample.valid ||
        (signal < LINE_TRACKING_MIN_SIGNAL_SUM);

    if (gStatus.lineLost) {
        gStatus.position = 0.0f;
        gStatus.centeredError = 0.0f;
        gStatus.shadowCorrection = 0.0f;
        gNeedsReacquire = true;
    } else {
        gStatus.position = weighted / signal;
        gStatus.centeredError =
            gStatus.position - LINE_TRACKING_CENTER_OFFSET;
        if (gNeedsReacquire) {
            Pid_reset(&gShadowPid, gStatus.centeredError);
            gStatus.shadowCorrection = 0.0f;
            gNeedsReacquire = false;
        } else {
            dtSeconds = (float) (nowMs - gStatus.lastUpdateMs) / 1000.0f;
            gStatus.shadowCorrection = Pid_step(&gShadowPid, 0.0f,
                gStatus.centeredError, dtSeconds, true);
        }
    }
    gStatus.lastUpdateMs = nowMs;
}

const LineTrackingStatus *LineTracking_getStatus(void)
{
    return &gStatus;
}

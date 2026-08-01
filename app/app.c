#include "app/app.h"

#include <string.h>

#include "app/req002.h"
#include "app/motor_test.h"
#include "algorithm/line_tracking.h"
#include "bsp/board_safety.h"
#include "bsp/timebase.h"
#include "drivers/diag_console.h"
#include "drivers/encoders.h"
#include "drivers/line_sensors.h"
#include "drivers/motor_driver.h"
#include "drivers/nrf24_ptx.h"
#include "drivers/oled_ssd1306.h"
#include "drivers/start_button.h"

static StartButton gStartButton;
static Req002State gLastOledReq002State;

static OledReq002State oledStateFor(Req002State state)
{
    switch (state) {
        case REQ002_STATE_ARMED:
        case REQ002_STATE_DEPART_A:
        case REQ002_STATE_LAP_ACTIVE:
        case REQ002_STATE_RETURN_MARKER:
        case REQ002_STATE_STOPPING:
            return OLED_REQ002_RUNNING;
        case REQ002_STATE_COMPLETE:
            return OLED_REQ002_COMPLETE;
        case REQ002_STATE_FAULT:
        case REQ002_STATE_BLOCKED_CALIBRATION:
            return OLED_REQ002_FAULT;
        case REQ002_STATE_IDLE:
        case REQ002_STATE_SAFE_LOCKED:
        default:
            return OLED_REQ002_READY;
    }
}

static bool oledUpdateAllowed(Req002State state)
{
    return (state == REQ002_STATE_IDLE) ||
        (state == REQ002_STATE_SAFE_LOCKED) ||
        (state == REQ002_STATE_BLOCKED_CALIBRATION) ||
        (state == REQ002_STATE_COMPLETE) ||
        (state == REQ002_STATE_FAULT);
}

void App_init(void)
{
    uint32_t nowMs;

    MotorDriver_init();
    BoardSafety_init();
    Timebase_init();
    nowMs = Timebase_nowMs();
    MotorTest_init(nowMs);
    StartButton_init(&gStartButton, nowMs);
    LineSensors_init(nowMs);
    LineTracking_init(nowMs);
    Encoders_init();
    Req002_init(nowMs);
    gLastOledReq002State = Req002_getStatus()->state;
    (void) OledSsd1306_showReq002Status(OLED_REQ002_READY, 0U, false);
    DiagConsole_init();
    DiagConsole_reportBoot();
    Nrf24Ptx_init();
}

void App_service(void)
{
    uint32_t nowMs = Timebase_nowMs();
    LineSensorSample lineSample;
    const LineTrackingStatus *lineTracking;
    Req002TrackingInput req002Tracking;

    BoardSafety_service();
    MotorTest_service(nowMs);
    StartButton_poll(&gStartButton, nowMs);
    LineSensors_service(nowMs);
    nowMs = Timebase_nowMs();
    LineTracking_service(nowMs);
    lineSample = LineSensors_snapshot();
    lineTracking = LineTracking_getStatus();
    memcpy(req002Tracking.analog, lineSample.analog,
        sizeof(req002Tracking.analog));
    req002Tracking.centeredError = lineTracking->centeredError;
    req002Tracking.steeringCorrection = lineTracking->shadowCorrection;
    req002Tracking.confidence = lineTracking->confidence;
    req002Tracking.signalSum = lineTracking->signalSum;
    req002Tracking.sampleSequence = lineSample.sequence;
    req002Tracking.trackingSequence = lineTracking->sequence;
    req002Tracking.lastSuccessMs = lineSample.lastSuccessMs;
    req002Tracking.sensorErrorCount = lineSample.errorCount;
    req002Tracking.digitalBits = lineSample.digitalBits;
    req002Tracking.i2cFailureStage = lineSample.lastFailureStage;
    req002Tracking.sensorValid = lineSample.valid;
    req002Tracking.trackingValid = lineTracking->valid;
    req002Tracking.lineLost = lineTracking->lineLost;
    Req002_service(nowMs, StartButton_takePress(&gStartButton),
        &req002Tracking);
    {
        const Req002Status *req002 = Req002_getStatus();
        if (req002->state != gLastOledReq002State) {
            gLastOledReq002State = req002->state;
            if (oledUpdateAllowed(req002->state)) {
                (void) OledSsd1306_showReq002Status(
                    oledStateFor(req002->state), req002->elapsedMs,
                    req002->tracking.dataValid);
            }
        }
    }
    DiagConsole_service();
    Nrf24Ptx_service();
}
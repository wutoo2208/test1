#include "app/req002.h"

#include <string.h>

#include "bsp/timebase.h"
#include "config/firmware_config.h"

#define REQ002_TRACKING_STALE_AFTER_MS \
    (LINE_SENSOR_RETRY_PERIOD_MS + LINE_SENSOR_POLL_PERIOD_MS)

static Req002Status gStatus;

static Req002BlockReason trackingBlockReason(
    const Req002ControlDecision *decision)
{
    switch (decision->state) {
        case REQ002_TRACKING_I2C_FAILURE:
            return REQ002_BLOCK_TRACKING_I2C_FAILURE;
        case REQ002_TRACKING_TIMEOUT:
            return REQ002_BLOCK_TRACKING_TIMEOUT;
        case REQ002_TRACKING_SIGNAL_INSUFFICIENT:
            return REQ002_BLOCK_SIGNAL_INSUFFICIENT;
        case REQ002_TRACKING_LINE_LOST:
            return REQ002_BLOCK_LINE_LOST;
        case REQ002_TRACKING_VALID_OBSERVATION:
            return REQ002_BLOCK_NONE;
        case REQ002_TRACKING_NOT_READY:
        default:
            return REQ002_BLOCK_TRACKING_INVALID;
    }
}

static Req002BlockReason firstInvalidGate(
    const Req002ControlDecision *decision)
{
    Req002BlockReason trackingReason;

    if (REQ002_CALIBRATION_VALID == 0U) {
        return REQ002_BLOCK_CALIBRATION_INVALID;
    }
    trackingReason = trackingBlockReason(decision);
    if (trackingReason != REQ002_BLOCK_NONE) return trackingReason;
    if (REQ002_ACTUATION_GATE_VALID == 0U) {
        return REQ002_BLOCK_ACTUATION_GATE_INVALID;
    }
    if (REQ002_PHYSICAL_PARAMETERS_VALID == 0U) {
        return REQ002_BLOCK_PHYSICAL_PARAMETERS_INVALID;
    }
    if (REQ002_ACTUATOR_ADAPTER_ENABLED == 0U) {
        return REQ002_BLOCK_ACTUATOR_ADAPTER_DISABLED;
    }
    return REQ002_BLOCK_NONE;
}

static void freezeElapsed(uint32_t nowMs)
{
    if (gStatus.timerRunning) {
        gStatus.frozenElapsedMs = nowMs - gStatus.startMs;
        gStatus.elapsedMs = gStatus.frozenElapsedMs;
        gStatus.timerRunning = false;
        gStatus.elapsedFrozen = true;
    }
}

static void tryStart(uint32_t nowMs)
{
    Req002BlockReason reason = firstInvalidGate(&gStatus.tracking);

    gStatus.buttonAttempts++;
    if (reason != REQ002_BLOCK_NONE) {
        gStatus.state = REQ002_STATE_BLOCKED_CALIBRATION;
        gStatus.blockReason = reason;
        gStatus.timerRunning = false;
        gStatus.elapsedFrozen = true;
        gStatus.frozenElapsedMs = 0U;
        gStatus.elapsedMs = 0U;
        return;
    }

    /* This path remains unreachable while the approved gates are false. */
    gStatus.state = REQ002_STATE_ARMED;
    gStatus.blockReason = REQ002_BLOCK_NONE;
    gStatus.startMs = nowMs;
    gStatus.elapsedMs = 0U;
    gStatus.frozenElapsedMs = 0U;
    gStatus.timerRunning = true;
    gStatus.elapsedFrozen = false;
}

Req002ControlDecision Req002_evaluateTracking(
    uint32_t nowMs, const Req002TrackingInput *input)
{
    Req002ControlDecision decision;

    memset(&decision, 0, sizeof(decision));
    decision.state = REQ002_TRACKING_NOT_READY;
    decision.nominalPeriodMs = LINE_SENSOR_POLL_PERIOD_MS;
    decision.staleAfterMs = REQ002_TRACKING_STALE_AFTER_MS;
    decision.steeringRequest = 0.0f;
    decision.calibrated = (REQ002_CALIBRATION_VALID != 0U);
    decision.motionAuthorized = false;
    decision.actuatorLocked = true;

    if (input == NULL) return decision;

    decision.snapshot = *input;
    decision.sampleAgeMs = nowMs - input->lastSuccessMs;
    decision.i2cFailed = (input->sensorErrorCount != 0U) &&
        !input->sensorValid;
    decision.timedOut = (input->sampleSequence != 0U) &&
        (decision.sampleAgeMs > decision.staleAfterMs);
    decision.signalInsufficient = input->sensorValid && input->trackingValid &&
        (input->signalSum < LINE_TRACKING_MIN_SIGNAL_SUM);
    decision.lineLost = input->lineLost;

    if (decision.i2cFailed) {
        decision.state = REQ002_TRACKING_I2C_FAILURE;
    } else if (decision.timedOut) {
        decision.state = REQ002_TRACKING_TIMEOUT;
    } else if (!input->sensorValid || !input->trackingValid ||
        (input->sampleSequence == 0U)) {
        decision.state = REQ002_TRACKING_NOT_READY;
    } else if (decision.signalInsufficient) {
        decision.state = REQ002_TRACKING_SIGNAL_INSUFFICIENT;
    } else if (decision.lineLost) {
        decision.state = REQ002_TRACKING_LINE_LOST;
    } else {
        decision.state = REQ002_TRACKING_VALID_OBSERVATION;
        decision.dataValid = true;
    }

    return decision;
}

void Req002_init(uint32_t nowMs)
{
    memset(&gStatus, 0, sizeof(gStatus));
    gStatus.state = REQ002_STATE_IDLE;
    gStatus.tracking = Req002_evaluateTracking(nowMs, NULL);
    gStatus.blockReason = firstInvalidGate(&gStatus.tracking);
    gStatus.startMs = nowMs;
    gStatus.timeoutMs = REQ002_TIMEOUT_MS;
    gStatus.elapsedFrozen = true;
    gStatus.actuatorLocked = true;
    gStatus.pidConfigured = (REQ002_PID_ENABLED != 0U);
}

void Req002_service(uint32_t nowMs, bool buttonPress,
    const Req002TrackingInput *tracking)
{
    gStatus.tracking = Req002_evaluateTracking(nowMs, tracking);

    if (buttonPress && ((gStatus.state == REQ002_STATE_IDLE) ||
        (gStatus.state == REQ002_STATE_SAFE_LOCKED) ||
        (gStatus.state == REQ002_STATE_BLOCKED_CALIBRATION) ||
        (gStatus.state == REQ002_STATE_COMPLETE))) {
        tryStart(nowMs);
    }

    if (gStatus.timerRunning) {
        gStatus.elapsedMs = nowMs - gStatus.startMs;
        if (Timebase_reached(nowMs, gStatus.startMs + gStatus.timeoutMs)) {
            freezeElapsed(nowMs);
            gStatus.state = REQ002_STATE_FAULT;
        }
    }

    /* Uncalibrated observations never advance the run or create commands. */
    gStatus.actuatorLocked = true;
    gStatus.tracking.motionAuthorized = false;
    gStatus.tracking.actuatorLocked = true;
    gStatus.tracking.steeringRequest = 0.0f;
}

const Req002Status *Req002_getStatus(void)
{
    return &gStatus;
}

const char *Req002_stateName(Req002State state)
{
    switch (state) {
        case REQ002_STATE_IDLE: return "IDLE";
        case REQ002_STATE_SAFE_LOCKED: return "SAFE_LOCKED";
        case REQ002_STATE_BLOCKED_CALIBRATION: return "BLOCKED_CALIBRATION";
        case REQ002_STATE_ARMED: return "ARMED";
        case REQ002_STATE_DEPART_A: return "DEPART_A";
        case REQ002_STATE_LAP_ACTIVE: return "LAP_ACTIVE";
        case REQ002_STATE_RETURN_MARKER: return "RETURN_MARKER";
        case REQ002_STATE_STOPPING: return "STOPPING";
        case REQ002_STATE_COMPLETE: return "COMPLETE";
        case REQ002_STATE_FAULT:
        default: return "FAULT";
    }
}

const char *Req002_blockReasonName(Req002BlockReason reason)
{
    switch (reason) {
        case REQ002_BLOCK_NONE: return "NONE";
        case REQ002_BLOCK_CALIBRATION_INVALID:
            return "CALIBRATION_INVALID";
        case REQ002_BLOCK_TRACKING_INVALID:
            return "TRACKING_INVALID";
        case REQ002_BLOCK_TRACKING_I2C_FAILURE:
            return "TRACKING_I2C_FAILURE";
        case REQ002_BLOCK_TRACKING_TIMEOUT:
            return "TRACKING_TIMEOUT";
        case REQ002_BLOCK_SIGNAL_INSUFFICIENT:
            return "SIGNAL_INSUFFICIENT";
        case REQ002_BLOCK_LINE_LOST:
            return "LINE_LOST";
        case REQ002_BLOCK_ACTUATION_GATE_INVALID:
            return "ACTUATION_GATE_INVALID";
        case REQ002_BLOCK_PHYSICAL_PARAMETERS_INVALID:
            return "PHYSICAL_PARAMETERS_INVALID";
        case REQ002_BLOCK_ACTUATOR_ADAPTER_DISABLED:
        default: return "ACTUATOR_ADAPTER_DISABLED";
    }
}

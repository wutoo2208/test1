#include "app/req002.h"

#include <string.h>

#include "bsp/board_safety.h"
#include "bsp/timebase.h"
#include "config/firmware_config.h"
#include "drivers/motor_driver.h"

#define REQ002_TRACKING_STALE_AFTER_MS \
    (LINE_SENSOR_RETRY_PERIOD_MS + LINE_SENSOR_POLL_PERIOD_MS)

static Req002Status gStatus;

#if REQ002_ACTUATION_BUILD
static uint32_t gNextControlMs;
static uint32_t gDepartSinceMs;
static uint32_t gMarkerSinceMs;
static uint32_t gLeftPulseAccumulator;
static uint32_t gRightPulseAccumulator;
static bool gDepartPending;
static bool gMarkerPending;
#endif

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

static void setLocked(bool locked)
{
    gStatus.actuatorLocked = locked;
    gStatus.tracking.actuatorLocked = locked;
    gStatus.tracking.motionAuthorized = !locked;
    if (locked) {
        gStatus.leftDemandPermille = 0U;
        gStatus.rightDemandPermille = 0U;
    }
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
        decision.steeringRequest = input->steeringCorrection;
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
    gStatus.pidConfigured = (REQ002_PID_ENABLED != 0U);
    setLocked(true);
#if REQ002_ACTUATION_BUILD
    gDepartPending = false;
    gMarkerPending = false;
    gLeftPulseAccumulator = 0U;
    gRightPulseAccumulator = 0U;
#endif
}

void Req002_abort(uint32_t nowMs)
{
    BoardSafety_stop(BOARD_SAFETY_STOP_OPERATOR);
    freezeElapsed(nowMs);
    if ((gStatus.state != REQ002_STATE_IDLE) &&
        (gStatus.state != REQ002_STATE_COMPLETE)) {
        gStatus.state = REQ002_STATE_SAFE_LOCKED;
    }
    gStatus.blockReason = REQ002_BLOCK_NONE;
    setLocked(true);
#if REQ002_ACTUATION_BUILD
    gDepartPending = false;
    gMarkerPending = false;
    gLeftPulseAccumulator = 0U;
    gRightPulseAccumulator = 0U;
#endif
}

#if REQ002_ACTUATION_BUILD
static bool isActiveState(Req002State state)
{
    return (state == REQ002_STATE_DEPART_A) ||
        (state == REQ002_STATE_LAP_ACTIVE) ||
        (state == REQ002_STATE_RETURN_MARKER);
}

static uint8_t blackCount(uint8_t bits)
{
    uint8_t count = 0U;

    bits &= 0x3FU;
    while (bits != 0U) {
        count = (uint8_t) (count + (bits & 1U));
        bits >>= 1U;
    }
    return count;
}

static bool markerDetected(const Req002ControlDecision *decision)
{
    return decision->dataValid &&
        (blackCount(decision->snapshot.digitalBits) >=
            REQ002_MARKER_MIN_BLACK);
}

static uint16_t clampDemand(float demand)
{
    if (demand <= 0.0f) return 0U;
    if (demand >= 1000.0f) return 1000U;
    return (uint16_t) demand;
}

static void stopWithFault(uint32_t nowMs, Req002BlockReason reason)
{
    BoardSafety_stop(BOARD_SAFETY_STOP_FAULT);
    freezeElapsed(nowMs);
    gStatus.state = REQ002_STATE_FAULT;
    gStatus.blockReason = reason;
    setLocked(true);
}

static void completeRun(uint32_t nowMs)
{
    gStatus.state = REQ002_STATE_STOPPING;
    BoardSafety_stop(BOARD_SAFETY_STOP_OPERATOR);
    freezeElapsed(nowMs);
    gStatus.returnMarkerSeen = true;
    gStatus.state = REQ002_STATE_COMPLETE;
    gStatus.blockReason = REQ002_BLOCK_NONE;
    setLocked(true);
}

static void tryStart(uint32_t nowMs)
{
    Req002BlockReason reason = firstInvalidGate(&gStatus.tracking);

    gStatus.buttonAttempts++;
    if ((reason != REQ002_BLOCK_NONE) ||
        !markerDetected(&gStatus.tracking) ||
        BoardSafety_faultLatched()) {
        BoardSafety_stop(BOARD_SAFETY_STOP_OPERATOR);
        gStatus.state = REQ002_STATE_BLOCKED_CALIBRATION;
        gStatus.blockReason = (reason != REQ002_BLOCK_NONE) ? reason :
            REQ002_BLOCK_TRACKING_INVALID;
        gStatus.timerRunning = false;
        gStatus.elapsedFrozen = true;
        gStatus.frozenElapsedMs = 0U;
        gStatus.elapsedMs = 0U;
        setLocked(true);
        return;
    }

    BoardSafety_stop(BOARD_SAFETY_STOP_OPERATOR);
    gStatus.state = REQ002_STATE_DEPART_A;
    gStatus.blockReason = REQ002_BLOCK_NONE;
    gStatus.startMs = nowMs;
    gStatus.elapsedMs = 0U;
    gStatus.frozenElapsedMs = 0U;
    gStatus.timerRunning = true;
    gStatus.elapsedFrozen = false;
    gStatus.startMarkerSeen = true;
    gStatus.departedStartMarker = false;
    gStatus.returnMarkerSeen = false;
    gStatus.controlSequence = 0U;
    gNextControlMs = nowMs;
    gDepartPending = false;
    gMarkerPending = false;
    gLeftPulseAccumulator = 0U;
    gRightPulseAccumulator = 0U;
    setLocked(false);
}

static bool applyControl(uint32_t nowMs)
{
    float correction;
    float leftDemand;
    float rightDemand;
    uint16_t leftOutput = 0U;
    uint16_t rightOutput = 0U;

    if (!Timebase_reached(nowMs, gNextControlMs)) return true;
    gNextControlMs = nowMs + REQ002_CONTROL_PERIOD_MS;

    if ((nowMs - gStatus.startMs) < REQ002_START_KICK_MS) {
        gStatus.leftDemandPermille = 1000U;
        gStatus.rightDemandPermille = 1000U;
        leftOutput = 1000U;
        rightOutput = 1000U;
    } else {
        correction = gStatus.tracking.steeringRequest;
        if (correction > 1.0f) correction = 1.0f;
        if (correction < -1.0f) correction = -1.0f;

        /* Verified sign: line left gives positive correction, so slow the
         * left wheel and speed the right wheel to steer left. */
        leftDemand = (float) REQ002_BASE_PULSE_PERMILLE -
            (correction * (float) REQ002_TURN_PULSE_PERMILLE);
        rightDemand = (float) REQ002_BASE_PULSE_PERMILLE +
            (correction * (float) REQ002_TURN_PULSE_PERMILLE);
        gStatus.leftDemandPermille = clampDemand(leftDemand);
        gStatus.rightDemandPermille = clampDemand(rightDemand);

        gLeftPulseAccumulator += gStatus.leftDemandPermille;
        gRightPulseAccumulator += gStatus.rightDemandPermille;
        if (gLeftPulseAccumulator >= 1000U) {
            gLeftPulseAccumulator -= 1000U;
            leftOutput = 1000U;
        }
        if (gRightPulseAccumulator >= 1000U) {
            gRightPulseAccumulator -= 1000U;
            rightOutput = 1000U;
        }
    }

    gStatus.controlSequence++;
    return MotorDriver_setVehicleForwardDuties(leftOutput, rightOutput) ==
        MOTOR_DRIVER_OK;
}
#endif

void Req002_service(uint32_t nowMs, bool buttonPress,
    const Req002TrackingInput *tracking)
{
    gStatus.tracking = Req002_evaluateTracking(nowMs, tracking);

#if REQ002_ACTUATION_BUILD
    if (buttonPress && isActiveState(gStatus.state)) {
        Req002_abort(nowMs);
        return;
    }
    if (buttonPress && ((gStatus.state == REQ002_STATE_IDLE) ||
        (gStatus.state == REQ002_STATE_SAFE_LOCKED) ||
        (gStatus.state == REQ002_STATE_BLOCKED_CALIBRATION) ||
        (gStatus.state == REQ002_STATE_COMPLETE))) {
        tryStart(nowMs);
    }

    if (!isActiveState(gStatus.state)) {
        setLocked(true);
        return;
    }

    gStatus.elapsedMs = nowMs - gStatus.startMs;
    if (Timebase_reached(nowMs, gStatus.startMs + gStatus.timeoutMs)) {
        stopWithFault(nowMs, REQ002_BLOCK_TRACKING_TIMEOUT);
        return;
    }
    if (!gStatus.tracking.dataValid) {
        stopWithFault(nowMs, trackingBlockReason(&gStatus.tracking));
        return;
    }
    setLocked(false);

    if (gStatus.state == REQ002_STATE_DEPART_A) {
        if (markerDetected(&gStatus.tracking)) {
            gDepartPending = false;
        } else if (!gDepartPending) {
            gDepartPending = true;
            gDepartSinceMs = nowMs;
        } else if (Timebase_reached(nowMs,
                gDepartSinceMs + REQ002_DEPART_CONFIRM_MS)) {
            gStatus.departedStartMarker = true;
            gStatus.state = REQ002_STATE_LAP_ACTIVE;
            gDepartPending = false;
        }
    } else if (gStatus.state == REQ002_STATE_LAP_ACTIVE) {
        if (markerDetected(&gStatus.tracking)) {
            BoardSafety_stop(BOARD_SAFETY_STOP_OPERATOR);
            setLocked(true);
            gStatus.state = REQ002_STATE_RETURN_MARKER;
            gMarkerSinceMs = nowMs;
            gMarkerPending = true;
            return;
        }
    } else if (gStatus.state == REQ002_STATE_RETURN_MARKER) {
        BoardSafety_stop(BOARD_SAFETY_STOP_OPERATOR);
        setLocked(true);
        if (!markerDetected(&gStatus.tracking)) {
            gStatus.state = REQ002_STATE_LAP_ACTIVE;
            gMarkerPending = false;
            gLeftPulseAccumulator = 0U;
            gRightPulseAccumulator = 0U;
            gNextControlMs = nowMs;
        } else if (gMarkerPending && Timebase_reached(nowMs,
                gMarkerSinceMs + REQ002_MARKER_CONFIRM_MS)) {
            completeRun(nowMs);
        }
        return;
    }

    if (!applyControl(nowMs)) {
        stopWithFault(nowMs, REQ002_BLOCK_ACTUATION_GATE_INVALID);
    }
#else
    if (buttonPress) {
        gStatus.buttonAttempts++;
        gStatus.state = REQ002_STATE_BLOCKED_CALIBRATION;
        gStatus.blockReason = firstInvalidGate(&gStatus.tracking);
    }
    setLocked(true);
#endif
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
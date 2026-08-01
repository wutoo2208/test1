#include "app/req002.h"

#include <string.h>

#include "algorithm/pid.h"

#include "bsp/board_safety.h"
#include "bsp/timebase.h"
#include "config/firmware_config.h"
#include "drivers/encoders.h"
#include "drivers/motor_driver.h"

#define REQ002_TRACKING_STALE_AFTER_MS \
    (LINE_SENSOR_RETRY_PERIOD_MS + LINE_SENSOR_POLL_PERIOD_MS)

static Req002Status gStatus;

#if REQ002_ACTUATION_BUILD
typedef enum {
    SPEED_BALANCE_MODE_DISABLED = 0,
    SPEED_BALANCE_MODE_STRAIGHT,
    SPEED_BALANCE_MODE_RIGHT_TURN
} SpeedBalanceMode;

static uint32_t gNextControlMs;
static uint32_t gDepartSinceMs;
static uint32_t gMarkerSinceMs;
static bool gDepartPending;
static bool gMarkerPending;
static bool gTrackingFaultPending;
static uint32_t gTrackingFaultSinceMs;
static Req002BlockReason gTrackingFaultReason;
static PidController gSpeedBalancePi;
static PidController gTurnLeftPi;
static SpeedBalanceMode gSpeedBalanceMode;
static uint32_t gSpeedBalanceSampleCount;
static float gSpeedBalanceTrimPermille;
static bool gEncoderFeedbackMissing;
static uint32_t gEncoderFeedbackMissingSinceMs;
static bool gEncoderFeedbackFaulted;
static bool gSharpRightTurnPending;
static bool gSharpRightTurnActive;
static bool gSharpRightTurnLatched;
static uint32_t gSharpRightTurnPendingSinceMs;
static uint32_t gSharpRightTurnStartMs;
static bool gFinishBrakeEngaged;

static void speedBalanceReset(void);
static void sharpRightTurnReset(void);
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
#if REQ002_ACTUATION_BUILD
    PidConfig speedPiConfig;
#endif

    memset(&gStatus, 0, sizeof(gStatus));
    gStatus.state = REQ002_STATE_IDLE;
    gStatus.tracking = Req002_evaluateTracking(nowMs, NULL);
    gStatus.blockReason = firstInvalidGate(&gStatus.tracking);
    gStatus.startMs = nowMs;
    gStatus.timeoutMs = REQ002_TIMEOUT_MS;
    gStatus.elapsedFrozen = true;
    gStatus.pidConfigured = (REQ002_PID_ENABLED != 0U);
#if REQ002_ACTUATION_BUILD
    speedPiConfig.kp = REQ002_SPEED_PI_KP;
    speedPiConfig.ki = REQ002_SPEED_PI_KI;
    speedPiConfig.kd = 0.0f;
    speedPiConfig.outputMin = -REQ002_SPEED_PI_OUTPUT_LIMIT;
    speedPiConfig.outputMax = REQ002_SPEED_PI_OUTPUT_LIMIT;
    speedPiConfig.integralMin = -REQ002_SPEED_PI_INTEGRAL_LIMIT;
    speedPiConfig.integralMax = REQ002_SPEED_PI_INTEGRAL_LIMIT;
    speedPiConfig.derivativeAlpha = 1.0f;
    Pid_init(&gSpeedBalancePi, &speedPiConfig);

    /* The turn controller may only add a small amount to the outer left
     * wheel. It reuses the field-tested gains and 5% output limit, but its
     * one-sided limits prevent it from weakening the 93% turn feed-forward. */
    speedPiConfig.outputMin = 0.0f;
    speedPiConfig.integralMin = 0.0f;
    Pid_init(&gTurnLeftPi, &speedPiConfig);
    speedBalanceReset();
    sharpRightTurnReset();
    gFinishBrakeEngaged = false;
#endif
    setLocked(true);
#if REQ002_ACTUATION_BUILD
    gDepartPending = false;
    gMarkerPending = false;
    gTrackingFaultPending = false;
    gTrackingFaultSinceMs = nowMs;
    gTrackingFaultReason = REQ002_BLOCK_NONE;
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
    speedBalanceReset();
    sharpRightTurnReset();
    gFinishBrakeEngaged = false;
    gDepartPending = false;
    gMarkerPending = false;
    gTrackingFaultPending = false;
    gTrackingFaultSinceMs = nowMs;
    gTrackingFaultReason = REQ002_BLOCK_NONE;
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

static uint16_t clampDemand(float demand, uint16_t maximumPermille)
{
    if (demand <= 0.0f) return 0U;
    if (demand >= (float) maximumPermille) {
        return maximumPermille;
    }
    return (uint16_t) demand;
}

static void speedBalanceReset(void)
{
    EncoderSpeedShadow speed = Encoders_speedShadowSnapshot();

    gSpeedBalanceSampleCount = speed.sampleCount;
    gSpeedBalanceTrimPermille = 0.0f;
    gSpeedBalanceMode = SPEED_BALANCE_MODE_DISABLED;
    gEncoderFeedbackMissing = false;
    gEncoderFeedbackMissingSinceMs = 0U;
    gEncoderFeedbackFaulted = false;
    Pid_reset(&gSpeedBalancePi, 0.0f);
    Pid_reset(&gTurnLeftPi, 0.0f);
}

static void sharpRightTurnReset(void)
{
    gSharpRightTurnPending = false;
    gSharpRightTurnActive = false;
    gSharpRightTurnLatched = false;
    gSharpRightTurnPendingSinceMs = 0U;
    gSharpRightTurnStartMs = 0U;
}

static bool sharpRightTurnRecovered(void)
{
    return gStatus.tracking.snapshot.centeredError <=
        REQ002_SHARP_RIGHT_EXIT_ERROR;
}

static bool sharpRightTurnRequested(void)
{
    return (gStatus.tracking.snapshot.centeredError >=
            REQ002_SHARP_RIGHT_ENTER_ERROR) &&
        (gStatus.tracking.steeringRequest < 0.0f);
}

static void sharpRightTurnUpdate(uint32_t nowMs, uint32_t elapsedMs)
{
    if (sharpRightTurnRecovered()) {
        sharpRightTurnReset();
        return;
    }

    if (gSharpRightTurnActive) {
        if (Timebase_reached(nowMs, gSharpRightTurnStartMs +
                REQ002_SHARP_RIGHT_MAX_MS)) {
            gSharpRightTurnActive = false;
        }
        return;
    }
    if (gSharpRightTurnLatched || (elapsedMs < REQ002_SOFT_START_MS)) {
        gSharpRightTurnPending = false;
        return;
    }

    if (!sharpRightTurnRequested()) {
        gSharpRightTurnPending = false;
        return;
    }
    if (!gSharpRightTurnPending) {
        gSharpRightTurnPending = true;
        gSharpRightTurnPendingSinceMs = nowMs;
        return;
    }
    if (Timebase_reached(nowMs, gSharpRightTurnPendingSinceMs +
            REQ002_SHARP_RIGHT_CONFIRM_MS)) {
        gSharpRightTurnPending = false;
        gSharpRightTurnActive = true;
        gSharpRightTurnLatched = true;
        gSharpRightTurnStartMs = nowMs;
        speedBalanceReset();
    }
}

static bool commandSharpRightTurn(void)
{
    /* Right 0% is intentional and bypasses the measured 1%..42% dead zone.
     * Disable both speed PI paths and their missing-feedback watchdog while
     * the inner wheel is deliberately unpowered. */
    speedBalanceReset();
    gStatus.leftDemandPermille =
        REQ002_SHARP_RIGHT_LEFT_PULSE_PERMILLE;
    gStatus.rightDemandPermille =
        REQ002_SHARP_RIGHT_RIGHT_PULSE_PERMILLE;
    gStatus.lastAppliedLeftDemandPermille = gStatus.leftDemandPermille;
    gStatus.lastAppliedRightDemandPermille = gStatus.rightDemandPermille;
    gStatus.controlSequence++;
    return MotorDriver_setVehicleForwardDuties(
        gStatus.leftDemandPermille, gStatus.rightDemandPermille) ==
        MOTOR_DRIVER_OK;
}

static bool applySharpRightTurn(uint32_t nowMs)
{
    if (!Timebase_reached(nowMs, gNextControlMs)) return true;
    gNextControlMs = nowMs + REQ002_CONTROL_PERIOD_MS;
    return commandSharpRightTurn();
}

static void speedBalanceWatchdog(uint32_t nowMs, bool feedbackMissing)
{
    uint32_t missingMs;

    if (!feedbackMissing) {
        gEncoderFeedbackMissing = false;
        gEncoderFeedbackMissingSinceMs = 0U;
        return;
    }
    if (!gEncoderFeedbackMissing) {
        gEncoderFeedbackMissing = true;
        gEncoderFeedbackMissingSinceMs = nowMs;
        gStatus.encoderFeedbackMissingEvents++;
    }

    missingMs = nowMs - gEncoderFeedbackMissingSinceMs;
    if (missingMs > gStatus.encoderFeedbackMissingMaxMs) {
        gStatus.encoderFeedbackMissingMaxMs = missingMs;
    }
    if (Timebase_reached(nowMs, gEncoderFeedbackMissingSinceMs +
            REQ002_ENCODER_FEEDBACK_FAULT_MS)) {
        gEncoderFeedbackFaulted = true;
    }
}

static float speedBalanceStep(SpeedBalanceMode mode, bool monitorEnabled,
    float targetRatio, uint32_t nowMs)
{
    EncoderSpeedShadow speed;
    PidController *controller;
    float leftNormalized;
    float rightNormalized;
    float measurement;
    float dtSeconds;
    float trimMagnitude;
    bool leftMissing;
    bool rightMissing;

    if (!monitorEnabled || (REQ002_SPEED_PI_ENABLED == 0U)) {
        speedBalanceReset();
        return 0.0f;
    }

    if (gSpeedBalanceMode != mode) {
        gSpeedBalanceMode = mode;
        gSpeedBalanceTrimPermille = 0.0f;
        Pid_reset(&gSpeedBalancePi, 0.0f);
        Pid_reset(&gTurnLeftPi, 0.0f);
    }

    speed = Encoders_speedShadowSnapshot();
    if (speed.initialized == 0U) {
        return gSpeedBalanceTrimPermille;
    }
    if (speed.sampleCount == gSpeedBalanceSampleCount) {
        if (gEncoderFeedbackMissing) {
            speedBalanceWatchdog(nowMs, true);
        }
        return gSpeedBalanceTrimPermille;
    }
    gSpeedBalanceSampleCount = speed.sampleCount;

    if (speed.windowMs == 0U) {
        return gSpeedBalanceTrimPermille;
    }

    leftMissing = speed.leftAbsDelta == 0U;
    rightMissing = speed.rightAbsDelta == 0U;
    speedBalanceWatchdog(nowMs, leftMissing || rightMissing);

    /* Feedback monitoring stays active through all steering states. PI output
     * is produced only for straight travel or the dominant right turn. */
    if (mode == SPEED_BALANCE_MODE_DISABLED) {
        gSpeedBalanceTrimPermille = 0.0f;
        return 0.0f;
    }

    /* A single stalled wheel is a valid zero-speed measurement. If both sides
     * remain at zero, hold the bounded trim and let the watchdog stop the run. */
    if (leftMissing && rightMissing) {
        return gSpeedBalanceTrimPermille;
    }

    leftNormalized = (float) speed.leftAbsDelta *
        REQ002_LEFT_ENCODER_TO_QEI_SCALE;
    rightNormalized = (float) speed.rightAbsDelta;
    measurement = leftNormalized - (rightNormalized * targetRatio);
    dtSeconds = (float) speed.windowMs * 0.001f;
    controller = (mode == SPEED_BALANCE_MODE_RIGHT_TURN) ?
        &gTurnLeftPi : &gSpeedBalancePi;
    gSpeedBalanceTrimPermille = Pid_step(controller, 0.0f,
        measurement, dtSeconds, false);

    gStatus.lastSpeedTrimPermille = gSpeedBalanceTrimPermille;
    trimMagnitude = (gSpeedBalanceTrimPermille >= 0.0f) ?
        gSpeedBalanceTrimPermille : -gSpeedBalanceTrimPermille;
    if (trimMagnitude > gStatus.peakSpeedTrimPermille) {
        gStatus.peakSpeedTrimPermille = trimMagnitude;
    }
    return gSpeedBalanceTrimPermille;
}
static void stopWithFault(uint32_t nowMs, Req002BlockReason reason)
{
    BoardSafety_stop(BOARD_SAFETY_STOP_FAULT);
    freezeElapsed(nowMs);
    gStatus.state = REQ002_STATE_FAULT;
    gStatus.blockReason = reason;
    speedBalanceReset();
    sharpRightTurnReset();
    gFinishBrakeEngaged = false;
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
    speedBalanceReset();
    sharpRightTurnReset();
    gFinishBrakeEngaged = false;
    setLocked(true);
}

static void serviceReturnMarker(uint32_t nowMs)
{
    setLocked(true);
    if (!gFinishBrakeEngaged && Timebase_reached(nowMs,
            gMarkerSinceMs + REQ002_FINISH_BRAKE_PREPARE_MS)) {
        MotorDriver_engageBrakeAll();
        gFinishBrakeEngaged = true;
    }

    /* Never let generic line-loss recovery overwrite terminal braking. If the
     * tracking sample becomes invalid, keep braking through the confirmation
     * window, then release to coast and report the real tracking fault. */
    if (!gStatus.tracking.dataValid) {
        if (Timebase_reached(nowMs,
                gMarkerSinceMs + REQ002_MARKER_CONFIRM_MS)) {
            Req002BlockReason reason = trackingBlockReason(&gStatus.tracking);

            MotorDriver_releaseBrakeAll();
            gFinishBrakeEngaged = false;
            stopWithFault(nowMs, reason);
        }
        return;
    }
    if (!markerDetected(&gStatus.tracking)) {
        MotorDriver_releaseBrakeAll();
        gFinishBrakeEngaged = false;
        gStatus.state = REQ002_STATE_LAP_ACTIVE;
        gMarkerPending = false;
        gNextControlMs = nowMs;
    } else if (gMarkerPending && Timebase_reached(nowMs,
            gMarkerSinceMs + REQ002_MARKER_CONFIRM_MS)) {
        MotorDriver_releaseBrakeAll();
        gFinishBrakeEngaged = false;
        completeRun(nowMs);
    }
}

static void tryStart(uint32_t nowMs)
{
    Req002BlockReason reason = firstInvalidGate(&gStatus.tracking);

    speedBalanceReset();
    sharpRightTurnReset();
    gFinishBrakeEngaged = false;
    gTrackingFaultPending = false;
    gTrackingFaultSinceMs = nowMs;
    gTrackingFaultReason = REQ002_BLOCK_NONE;
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
    gStatus.lastAppliedLeftDemandPermille = 0U;
    gStatus.lastAppliedRightDemandPermille = 0U;
    gStatus.lastSpeedTrimPermille = 0.0f;
    gStatus.peakSpeedTrimPermille = 0.0f;
    gStatus.encoderFeedbackMissingMaxMs = 0U;
    gStatus.encoderFeedbackMissingEvents = 0U;
    gNextControlMs = nowMs;
    gDepartPending = false;
    gMarkerPending = false;
    setLocked(false);
}

static bool applyTrackingRecovery(uint32_t nowMs)
{
    if (!Timebase_reached(nowMs, gNextControlMs)) return true;
    gNextControlMs = nowMs + REQ002_CONTROL_PERIOD_MS;

    /* A short invalid-line observation is usually a gap or edge transition.
     * Continue at reduced speed with a slight clockwise/right bias, but only
     * until the confirmation timer converts a persistent loss into FAULT. */
    speedBalanceReset();
    gStatus.leftDemandPermille =
        REQ002_TRACKING_RECOVERY_LEFT_PERMILLE;
    gStatus.rightDemandPermille =
        REQ002_TRACKING_RECOVERY_RIGHT_PERMILLE;
    gStatus.controlSequence++;
    return MotorDriver_setVehicleForwardDuties(
        gStatus.leftDemandPermille, gStatus.rightDemandPermille) ==
        MOTOR_DRIVER_OK;
}

static bool applyControl(uint32_t nowMs)
{
    float correction;
    float correctionMagnitude;
    float steeringCommand;
    float steeringMagnitude;
    float curveSlowdown;
    float turnAuthority;
    float leftBase;
    float rightBase;
    float leftDemand;
    float rightDemand;
    float speedTrim;
    float speedTargetRatio;
    float rightTurnMinimum;
    float rampScale = 1.0f;
    SpeedBalanceMode speedBalanceMode;
    bool feedbackMonitorEnabled;
    uint16_t demandLimitPermille;
    uint32_t elapsedMs;

    if (!Timebase_reached(nowMs, gNextControlMs)) return true;
    gNextControlMs = nowMs + REQ002_CONTROL_PERIOD_MS;

    elapsedMs = nowMs - gStatus.startMs;
    if (elapsedMs < REQ002_SOFT_START_MS) {
        rampScale = (float) elapsedMs / (float) REQ002_SOFT_START_MS;
    }

    sharpRightTurnUpdate(nowMs, elapsedMs);
    if (gSharpRightTurnActive) {
        return commandSharpRightTurn();
    }

    correction = gStatus.tracking.steeringRequest;
    if (correction > 1.0f) correction = 1.0f;
    if (correction < -1.0f) correction = -1.0f;

    correctionMagnitude = (correction >= 0.0f) ? correction : -correction;
    steeringCommand = correction;
    steeringMagnitude = correctionMagnitude;
    if ((correctionMagnitude > REQ002_SPEED_PI_STRAIGHT_THRESHOLD) &&
        (steeringMagnitude < REQ002_TURN_MIN_CORRECTION)) {
        steeringMagnitude = REQ002_TURN_MIN_CORRECTION;
        steeringCommand = (correction < 0.0f) ?
            -steeringMagnitude : steeringMagnitude;
    }

    demandLimitPermille =
        (correctionMagnitude <= REQ002_SPEED_PI_STRAIGHT_THRESHOLD) ?
        REQ002_MAX_PULSE_PERMILLE : REQ002_TURN_MAX_PULSE_PERMILLE;
    if (correction < 0.0f) {
        /* Clockwise course: line on the right requires the dominant turn.
         * Slow the vehicle harder and apply greater left/right differential. */
        curveSlowdown = steeringMagnitude *
            (float) REQ002_RIGHT_CURVE_SLOWDOWN_PERMILLE;
        turnAuthority = (float) REQ002_RIGHT_TURN_PULSE_PERMILLE;
    } else {
        /* Left steering remains only as a weaker recovery correction. */
        curveSlowdown = steeringMagnitude *
            (float) REQ002_LEFT_CURVE_SLOWDOWN_PERMILLE;
        turnAuthority = (float) REQ002_LEFT_TURN_PULSE_PERMILLE;
    }
    if (correctionMagnitude > REQ002_SPEED_PI_STRAIGHT_THRESHOLD) {
        /* Turn control must not inherit the large ground-load straight trim.
         * Start both wheels from a common base so either steering sign can
         * create an immediate and symmetric differential. */
        leftBase = (float) REQ002_TURN_BASE_PULSE_PERMILLE -
            curveSlowdown;
        rightBase = leftBase;
    } else {
        leftBase = (float) REQ002_BASE_PULSE_PERMILLE - curveSlowdown;
        rightBase = leftBase - (float) REQ002_RIGHT_TRIM_PERMILLE;
    }

    /* The large permanent trim applies only to straight ground travel. */
    leftDemand = rampScale *
        (leftBase - (steeringCommand * turnAuthority));
    rightDemand = rampScale *
        (rightBase + (steeringCommand * turnAuthority));

    if (correctionMagnitude > REQ002_SPEED_PI_STRAIGHT_THRESHOLD) {
        rightTurnMinimum = rampScale *
            (float) REQ002_TURN_RIGHT_MIN_PULSE_PERMILLE;
        if (rightDemand < rightTurnMinimum) {
            rightDemand = rightTurnMinimum;
        }
    }

    feedbackMonitorEnabled =
        (elapsedMs >= REQ002_SOFT_START_MS) &&
        (leftDemand >= (float) REQ002_SPEED_PI_MIN_DEMAND_PERMILLE) &&
        (rightDemand >= (float) REQ002_SPEED_PI_MIN_DEMAND_PERMILLE);
    speedBalanceMode = SPEED_BALANCE_MODE_DISABLED;
    speedTargetRatio = REQ002_LEFT_SPEED_TARGET_RATIO;
    if (feedbackMonitorEnabled &&
        (correctionMagnitude <= REQ002_SPEED_PI_STRAIGHT_THRESHOLD)) {
        speedBalanceMode = SPEED_BALANCE_MODE_STRAIGHT;
    } else if (feedbackMonitorEnabled &&
        (correction < -REQ002_SPEED_PI_STRAIGHT_THRESHOLD) &&
        (rightDemand > 0.0f)) {
        /* Preserve the commanded turn ratio while allowing encoder PI to add
         * at most 5% to the outer left wheel. The calibrated straight ratio
         * compensates for the different 1x/4x encoder paths and wheel load. */
        speedBalanceMode = SPEED_BALANCE_MODE_RIGHT_TURN;
        speedTargetRatio = REQ002_LEFT_SPEED_TARGET_RATIO *
            (leftDemand / rightDemand);
    }
    speedTrim = speedBalanceStep(speedBalanceMode,
        feedbackMonitorEnabled, speedTargetRatio, nowMs);
    if (gEncoderFeedbackFaulted) return false;

    if (speedBalanceMode == SPEED_BALANCE_MODE_STRAIGHT) {
        /* Positive trim means the right normalized count was higher. */
        leftDemand += speedTrim;
        rightDemand -= speedTrim;
    } else if (speedBalanceMode == SPEED_BALANCE_MODE_RIGHT_TURN) {
        /* Turn PI is one-sided: keep the 93% feed-forward and only boost the
         * outer left wheel when encoder feedback says it is lagging. */
        leftDemand += speedTrim;
    }
    gStatus.leftDemandPermille = clampDemand(leftDemand, demandLimitPermille);
    gStatus.rightDemandPermille = clampDemand(rightDemand, demandLimitPermille);
    gStatus.lastAppliedLeftDemandPermille = gStatus.leftDemandPermille;
    gStatus.lastAppliedRightDemandPermille = gStatus.rightDemandPermille;

    gStatus.controlSequence++;
    return MotorDriver_setVehicleForwardDuties(
        gStatus.leftDemandPermille, gStatus.rightDemandPermille) ==
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
        gTrackingFaultPending = false;
        setLocked(true);
        return;
    }

    gStatus.elapsedMs = nowMs - gStatus.startMs;
    if (Timebase_reached(nowMs, gStatus.startMs + gStatus.timeoutMs)) {
        stopWithFault(nowMs, REQ002_BLOCK_TRACKING_TIMEOUT);
        return;
    }
    if (gStatus.state == REQ002_STATE_RETURN_MARKER) {
        serviceReturnMarker(nowMs);
        return;
    }
    if (!gStatus.tracking.dataValid) {
        Req002BlockReason reason = trackingBlockReason(&gStatus.tracking);

        if ((reason == REQ002_BLOCK_SIGNAL_INSUFFICIENT) ||
            (reason == REQ002_BLOCK_LINE_LOST)) {
            if (!gTrackingFaultPending) {
                gTrackingFaultPending = true;
                gTrackingFaultSinceMs = nowMs;
            }
            gTrackingFaultReason = reason;
            if (gSharpRightTurnActive &&
                !Timebase_reached(nowMs, gSharpRightTurnStartMs +
                    REQ002_SHARP_RIGHT_MAX_MS)) {
                setLocked(false);
                if (!applySharpRightTurn(nowMs)) {
                    stopWithFault(nowMs,
                        REQ002_BLOCK_ACTUATION_GATE_INVALID);
                }
                return;
            }
            if (gSharpRightTurnActive) {
                gSharpRightTurnActive = false;
            }
            if (Timebase_reached(nowMs,
                    gTrackingFaultSinceMs +
                    REQ002_TRACKING_FAULT_CONFIRM_MS)) {
                stopWithFault(nowMs, gTrackingFaultReason);
                return;
            }

            setLocked(false);
            if (!applyTrackingRecovery(nowMs)) {
                stopWithFault(nowMs,
                    REQ002_BLOCK_ACTUATION_GATE_INVALID);
            }
            return;
        }

        stopWithFault(nowMs, reason);
        return;
    }
    gTrackingFaultPending = false;
    gTrackingFaultReason = REQ002_BLOCK_NONE;
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
            /* Begin terminal stopping immediately. Force both IN1 signals
             * high, then assert both IN2 signals after one complete PWM period
             * for DRV8870 1/1 electrical braking. */
            setLocked(true);
            speedBalanceReset();
            sharpRightTurnReset();
            MotorDriver_prepareBrakeAll();
            gFinishBrakeEngaged = false;
            gStatus.state = REQ002_STATE_RETURN_MARKER;
            gMarkerSinceMs = nowMs;
            gMarkerPending = true;
            return;
        }
    }

    if (!applyControl(nowMs)) {
        Req002BlockReason reason = gEncoderFeedbackFaulted ?
            REQ002_BLOCK_ENCODER_FEEDBACK_INVALID :
            REQ002_BLOCK_ACTUATION_GATE_INVALID;
        stopWithFault(nowMs, reason);
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
            return "ACTUATOR_ADAPTER_DISABLED";
        case REQ002_BLOCK_ENCODER_FEEDBACK_INVALID:
        default: return "ENCODER_FEEDBACK_INVALID";
    }
}

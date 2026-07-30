#include "app/req002.h"

#include "bsp/timebase.h"
#include "config/firmware_config.h"

static Req002Status gStatus;

static Req002BlockReason firstInvalidGate(void)
{
    if (REQ002_CALIBRATION_VALID == 0U) {
        return REQ002_BLOCK_CALIBRATION_INVALID;
    }
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
    Req002BlockReason reason = firstInvalidGate();

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

void Req002_init(uint32_t nowMs)
{
    gStatus.state = REQ002_STATE_IDLE;
    gStatus.blockReason = firstInvalidGate();
    gStatus.startMs = nowMs;
    gStatus.elapsedMs = 0U;
    gStatus.frozenElapsedMs = 0U;
    gStatus.timeoutMs = REQ002_TIMEOUT_MS;
    gStatus.buttonAttempts = 0U;
    gStatus.timerRunning = false;
    gStatus.elapsedFrozen = true;
    gStatus.startMarkerSeen = false;
    gStatus.departedStartMarker = false;
    gStatus.returnMarkerSeen = false;
    gStatus.actuatorLocked = true;
    gStatus.pidConfigured = (REQ002_PID_ENABLED != 0U);
}

void Req002_service(uint32_t nowMs, bool buttonPress, uint8_t rawLineBits)
{
    (void) rawLineBits;

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

    /*
     * Marker/departure fields intentionally remain scaffolding. Raw sensor
     * polarity/order and A-marker classification are uncalibrated, so raw
     * bits must not advance the run state.
     */
    gStatus.actuatorLocked = true;
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
        case REQ002_BLOCK_ACTUATION_GATE_INVALID:
            return "ACTUATION_GATE_INVALID";
        case REQ002_BLOCK_PHYSICAL_PARAMETERS_INVALID:
            return "PHYSICAL_PARAMETERS_INVALID";
        case REQ002_BLOCK_ACTUATOR_ADAPTER_DISABLED:
        default: return "ACTUATOR_ADAPTER_DISABLED";
    }
}

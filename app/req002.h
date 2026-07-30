#ifndef APP_REQ002_H_
#define APP_REQ002_H_

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    REQ002_STATE_IDLE = 0,
    REQ002_STATE_SAFE_LOCKED,
    REQ002_STATE_BLOCKED_CALIBRATION,
    REQ002_STATE_ARMED,
    REQ002_STATE_DEPART_A,
    REQ002_STATE_LAP_ACTIVE,
    REQ002_STATE_RETURN_MARKER,
    REQ002_STATE_STOPPING,
    REQ002_STATE_COMPLETE,
    REQ002_STATE_FAULT
} Req002State;

typedef enum {
    REQ002_BLOCK_NONE = 0,
    REQ002_BLOCK_CALIBRATION_INVALID,
    REQ002_BLOCK_ACTUATION_GATE_INVALID,
    REQ002_BLOCK_PHYSICAL_PARAMETERS_INVALID,
    REQ002_BLOCK_ACTUATOR_ADAPTER_DISABLED
} Req002BlockReason;

typedef struct {
    Req002State state;
    Req002BlockReason blockReason;
    uint32_t startMs;
    uint32_t elapsedMs;
    uint32_t frozenElapsedMs;
    uint32_t timeoutMs;
    uint32_t buttonAttempts;
    bool timerRunning;
    bool elapsedFrozen;
    bool startMarkerSeen;
    bool departedStartMarker;
    bool returnMarkerSeen;
    bool actuatorLocked;
    bool pidConfigured;
} Req002Status;

void Req002_init(uint32_t nowMs);
void Req002_service(uint32_t nowMs, bool buttonPress,
    uint8_t rawLineBits);
const Req002Status *Req002_getStatus(void);
const char *Req002_stateName(Req002State state);
const char *Req002_blockReasonName(Req002BlockReason reason);

#endif /* APP_REQ002_H_ */

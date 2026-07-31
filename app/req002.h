#ifndef APP_REQ002_H_
#define APP_REQ002_H_

#include <stdbool.h>
#include <stdint.h>

#define REQ002_LINE_SENSOR_COUNT (6U)

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
    REQ002_BLOCK_TRACKING_INVALID,
    REQ002_BLOCK_TRACKING_I2C_FAILURE,
    REQ002_BLOCK_TRACKING_TIMEOUT,
    REQ002_BLOCK_SIGNAL_INSUFFICIENT,
    REQ002_BLOCK_LINE_LOST,
    REQ002_BLOCK_ACTUATION_GATE_INVALID,
    REQ002_BLOCK_PHYSICAL_PARAMETERS_INVALID,
    REQ002_BLOCK_ACTUATOR_ADAPTER_DISABLED
} Req002BlockReason;

typedef enum {
    REQ002_TRACKING_NOT_READY = 0,
    REQ002_TRACKING_VALID_OBSERVATION,
    REQ002_TRACKING_I2C_FAILURE,
    REQ002_TRACKING_TIMEOUT,
    REQ002_TRACKING_SIGNAL_INSUFFICIENT,
    REQ002_TRACKING_LINE_LOST
} Req002TrackingObservationState;

typedef struct {
    uint16_t analog[REQ002_LINE_SENSOR_COUNT];
    float centeredError;
    float confidence;
    float signalSum;
    uint32_t sampleSequence;
    uint32_t trackingSequence;
    uint32_t lastSuccessMs;
    uint32_t sensorErrorCount;
    uint8_t digitalBits;
    uint8_t i2cFailureStage;
    bool sensorValid;
    bool trackingValid;
    bool lineLost;
} Req002TrackingInput;

typedef struct {
    Req002TrackingObservationState state;
    Req002TrackingInput snapshot;
    uint32_t sampleAgeMs;
    uint32_t nominalPeriodMs;
    uint32_t staleAfterMs;
    float steeringRequest;
    bool dataValid;
    bool i2cFailed;
    bool timedOut;
    bool signalInsufficient;
    bool lineLost;
    bool calibrated;
    bool motionAuthorized;
    bool actuatorLocked;
} Req002ControlDecision;

typedef struct {
    Req002State state;
    Req002BlockReason blockReason;
    Req002ControlDecision tracking;
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

Req002ControlDecision Req002_evaluateTracking(
    uint32_t nowMs, const Req002TrackingInput *input);
void Req002_init(uint32_t nowMs);
void Req002_service(uint32_t nowMs, bool buttonPress,
    const Req002TrackingInput *tracking);
const Req002Status *Req002_getStatus(void);
const char *Req002_stateName(Req002State state);
const char *Req002_blockReasonName(Req002BlockReason reason);

#endif /* APP_REQ002_H_ */

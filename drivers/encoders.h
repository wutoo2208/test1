#ifndef DRIVERS_ENCODERS_H_
#define DRIVERS_ENCODERS_H_

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    uint8_t rightState;
    uint32_t rightCount;
    bool rightDown;
    uint8_t leftState;
    int32_t leftCount;
    uint32_t leftInvalidTransitions;
} EncoderSnapshot;

#define ENCODER_SPEED_SHADOW_PERIOD_MS (10U)

typedef struct {
    uint32_t sampleCount;
    uint32_t lastSampleMs;
    uint32_t windowMs;
    uint32_t initialized;
    int32_t leftDelta;
    int32_t rightDelta;
    uint32_t leftAbsDelta;
    uint32_t rightAbsDelta;
    int32_t leftCount;
    uint32_t rightCount;
    uint32_t leftInvalidTransitions;
    uint32_t leftInvalidDelta;
    uint32_t leftState;
    uint32_t rightState;
    uint32_t rightDown;
    uint32_t motionSampleCount;
    uint32_t bothMotionSampleCount;
    uint32_t leftAbsSum;
    uint32_t rightAbsSum;
    uint32_t leftBothAbsSum;
    uint32_t rightBothAbsSum;
    uint32_t leftAbsPeak;
    uint32_t rightAbsPeak;
    uint32_t invalidDuringMotion;
} EncoderSpeedShadow;

extern volatile EncoderSpeedShadow gEncoderSpeedShadow;

void Encoders_init(void);
EncoderSnapshot Encoders_snapshot(void);
void Encoders_speedShadowInit(uint32_t nowMs);
void Encoders_speedShadowService(uint32_t nowMs);
EncoderSpeedShadow Encoders_speedShadowSnapshot(void);
void Encoders_onLeftEncoderAInterrupt(void);

#endif /* DRIVERS_ENCODERS_H_ */

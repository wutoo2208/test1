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

void Encoders_init(void);
EncoderSnapshot Encoders_snapshot(void);
void Encoders_onLeftCaptureInterrupt(void);

#endif /* DRIVERS_ENCODERS_H_ */

#include "drivers/encoders.h"

#include <stdbool.h>

#include "ti_msp_dl_config.h"

static volatile int32_t gLeftCount;
static volatile uint32_t gLeftInvalidTransitions;
static volatile uint8_t gLeftState;

static uint8_t readLeftState(void)
{
    uint8_t state = 0U;
    if (DL_GPIO_readPins(GPIO_LEFT_CAPTURE_C0_PORT,
            GPIO_LEFT_CAPTURE_C0_PIN) != 0U) state |= 2U;
    if (DL_GPIO_readPins(GPIO_LEFT_CAPTURE_C1_PORT,
            GPIO_LEFT_CAPTURE_C1_PIN) != 0U) state |= 1U;
    return state;
}

static uint8_t readRightState(void)
{
    uint8_t state = 0U;
    if (DL_GPIO_readPins(GPIO_RIGHT_QEI_PHA_PORT,
            GPIO_RIGHT_QEI_PHA_PIN) != 0U) state |= 2U;
    if (DL_GPIO_readPins(GPIO_RIGHT_QEI_PHB_PORT,
            GPIO_RIGHT_QEI_PHB_PIN) != 0U) state |= 1U;
    return state;
}

static void updateLeftQuadrature(void)
{
    static const int8_t transitions[16] = {
         0,  1, -1,  0, -1,  0,  0,  1,
         1,  0,  0, -1,  0, -1,  1,  0
    };
    uint8_t next = readLeftState();
    uint8_t index = (uint8_t) ((gLeftState << 2U) | next);
    int8_t step = transitions[index];

    if ((step == 0) && (next != gLeftState) &&
        ((next ^ gLeftState) == 3U)) {
        gLeftInvalidTransitions++;
    }
    gLeftCount += step;
    gLeftState = next;
}

void Encoders_init(void)
{
    gLeftCount = 0;
    gLeftInvalidTransitions = 0U;
    gLeftState = readLeftState();

    DL_TimerG_setTimerCount(RIGHT_QEI_INST, 0U);
    DL_TimerG_startCounter(RIGHT_QEI_INST);
    DL_TimerA_startCounter(LEFT_CAPTURE_INST);
    NVIC_ClearPendingIRQ(LEFT_CAPTURE_INST_INT_IRQN);
    NVIC_EnableIRQ(LEFT_CAPTURE_INST_INT_IRQN);
}

EncoderSnapshot Encoders_snapshot(void)
{
    EncoderSnapshot snapshot;

    __disable_irq();
    snapshot.leftState = readLeftState();
    snapshot.leftCount = gLeftCount;
    snapshot.leftInvalidTransitions = gLeftInvalidTransitions;
    __enable_irq();

    snapshot.rightState = readRightState();
    snapshot.rightCount = DL_TimerG_getTimerCount(RIGHT_QEI_INST);
    snapshot.rightDown = DL_TimerG_getQEIDirection(RIGHT_QEI_INST) ==
        DL_TIMER_QEI_DIR_DOWN;
    return snapshot;
}

void Encoders_onLeftCaptureInterrupt(void)
{
    DL_TIMER_IIDX pending;

    while ((pending = DL_TimerA_getPendingInterrupt(LEFT_CAPTURE_INST)) != 0) {
        switch (pending) {
            case DL_TIMER_IIDX_CC0_UP:
            case DL_TIMER_IIDX_CC1_UP:
                updateLeftQuadrature();
                break;
            case DL_TIMER_IIDX_LOAD:
            default:
                break;
        }
    }
}

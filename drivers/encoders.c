#include "drivers/encoders.h"

#include <stdbool.h>

#include "ti_msp_dl_config.h"

static volatile int32_t gLeftCount;
static volatile uint32_t gLeftInvalidTransitions;
static volatile uint8_t gLeftState;
static int32_t gShadowPreviousLeftCount;
static uint32_t gShadowPreviousRightCount;
static uint32_t gShadowPreviousInvalidTransitions;
static uint32_t gShadowLastSampleMs;
static uint32_t gShadowNextSampleMs;

volatile EncoderSpeedShadow gEncoderSpeedShadow;

static uint8_t readLeftState(void)
{
    uint8_t state = 0U;
    if (DL_GPIO_readPins(DIAG_GPIO_LEFT_ENCODER_A_PORT,
            DIAG_GPIO_LEFT_ENCODER_A_PIN) != 0U) state |= 2U;
    if (DL_GPIO_readPins(DIAG_GPIO_LEFT_ENCODER_B_PORT,
            DIAG_GPIO_LEFT_ENCODER_B_PIN) != 0U) state |= 1U;
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

void Encoders_init(void)
{
    gLeftCount = 0;
    gLeftInvalidTransitions = 0U;
    gLeftState = readLeftState();

    DL_GPIO_clearInterruptStatus(DIAG_GPIO_LEFT_ENCODER_A_PORT,
        DIAG_GPIO_LEFT_ENCODER_A_PIN);
    NVIC_ClearPendingIRQ(DIAG_GPIO_INT_IRQN);
    NVIC_EnableIRQ(DIAG_GPIO_INT_IRQN);

    DL_TimerG_setTimerCount(RIGHT_QEI_INST, 0U);
    DL_TimerG_startCounter(RIGHT_QEI_INST);
}

EncoderSnapshot Encoders_snapshot(void)
{
    EncoderSnapshot snapshot;

    /* Read both continuously running counters at one logical time point. */
    __disable_irq();
    snapshot.leftState = readLeftState();
    snapshot.leftCount = gLeftCount;
    snapshot.leftInvalidTransitions = gLeftInvalidTransitions;
    snapshot.rightState = readRightState();
    snapshot.rightCount = DL_TimerG_getTimerCount(RIGHT_QEI_INST);
    snapshot.rightDown = DL_TimerG_getQEIDirection(RIGHT_QEI_INST) ==
        DL_TIMER_QEI_DIR_DOWN;
    __enable_irq();
    return snapshot;
}

static uint32_t absoluteDelta(int32_t value)
{
    if (value >= 0) return (uint32_t) value;
    return (uint32_t) (-(value + 1)) + 1U;
}

void Encoders_speedShadowInit(uint32_t nowMs)
{
    EncoderSnapshot snapshot = Encoders_snapshot();

    gShadowPreviousLeftCount = snapshot.leftCount;
    gShadowPreviousRightCount = snapshot.rightCount;
    gShadowPreviousInvalidTransitions = snapshot.leftInvalidTransitions;
    gShadowLastSampleMs = nowMs;
    gShadowNextSampleMs = nowMs + ENCODER_SPEED_SHADOW_PERIOD_MS;

    gEncoderSpeedShadow.sampleCount = 0U;
    gEncoderSpeedShadow.lastSampleMs = nowMs;
    gEncoderSpeedShadow.windowMs = 0U;
    gEncoderSpeedShadow.initialized = 0U;
    gEncoderSpeedShadow.leftDelta = 0;
    gEncoderSpeedShadow.rightDelta = 0;
    gEncoderSpeedShadow.leftAbsDelta = 0U;
    gEncoderSpeedShadow.rightAbsDelta = 0U;
    gEncoderSpeedShadow.leftCount = snapshot.leftCount;
    gEncoderSpeedShadow.rightCount = snapshot.rightCount;
    gEncoderSpeedShadow.leftInvalidTransitions =
        snapshot.leftInvalidTransitions;
    gEncoderSpeedShadow.leftInvalidDelta = 0U;
    gEncoderSpeedShadow.leftState = snapshot.leftState;
    gEncoderSpeedShadow.rightState = snapshot.rightState;
    gEncoderSpeedShadow.rightDown = snapshot.rightDown ? 1U : 0U;
    gEncoderSpeedShadow.motionSampleCount = 0U;
    gEncoderSpeedShadow.bothMotionSampleCount = 0U;
    gEncoderSpeedShadow.leftAbsSum = 0U;
    gEncoderSpeedShadow.rightAbsSum = 0U;
    gEncoderSpeedShadow.leftBothAbsSum = 0U;
    gEncoderSpeedShadow.rightBothAbsSum = 0U;
    gEncoderSpeedShadow.leftAbsPeak = 0U;
    gEncoderSpeedShadow.rightAbsPeak = 0U;
    gEncoderSpeedShadow.invalidDuringMotion = 0U;
}

void Encoders_speedShadowService(uint32_t nowMs)
{
    EncoderSnapshot snapshot;
    int32_t leftDelta;
    int32_t rightDelta;
    uint32_t invalidDelta;

    if ((int32_t) (nowMs - gShadowNextSampleMs) < 0) return;

    snapshot = Encoders_snapshot();
    leftDelta = snapshot.leftCount - gShadowPreviousLeftCount;
    rightDelta = (int32_t) (int16_t) ((uint16_t) snapshot.rightCount -
        (uint16_t) gShadowPreviousRightCount);
    invalidDelta = snapshot.leftInvalidTransitions -
        gShadowPreviousInvalidTransitions;

    gEncoderSpeedShadow.sampleCount++;
    gEncoderSpeedShadow.lastSampleMs = nowMs;
    gEncoderSpeedShadow.windowMs = nowMs - gShadowLastSampleMs;
    gEncoderSpeedShadow.initialized = 1U;
    gEncoderSpeedShadow.leftDelta = leftDelta;
    gEncoderSpeedShadow.rightDelta = rightDelta;
    gEncoderSpeedShadow.leftAbsDelta = absoluteDelta(leftDelta);
    gEncoderSpeedShadow.rightAbsDelta = absoluteDelta(rightDelta);
    gEncoderSpeedShadow.leftCount = snapshot.leftCount;
    gEncoderSpeedShadow.rightCount = snapshot.rightCount;
    gEncoderSpeedShadow.leftInvalidTransitions =
        snapshot.leftInvalidTransitions;
    gEncoderSpeedShadow.leftInvalidDelta = invalidDelta;
    gEncoderSpeedShadow.leftState = snapshot.leftState;
    gEncoderSpeedShadow.rightState = snapshot.rightState;
    gEncoderSpeedShadow.rightDown = snapshot.rightDown ? 1U : 0U;
    if ((gEncoderSpeedShadow.leftAbsDelta != 0U) ||
        (gEncoderSpeedShadow.rightAbsDelta != 0U)) {
        gEncoderSpeedShadow.motionSampleCount++;
        gEncoderSpeedShadow.leftAbsSum +=
            gEncoderSpeedShadow.leftAbsDelta;
        gEncoderSpeedShadow.rightAbsSum +=
            gEncoderSpeedShadow.rightAbsDelta;
        gEncoderSpeedShadow.invalidDuringMotion += invalidDelta;
        if (gEncoderSpeedShadow.leftAbsDelta >
            gEncoderSpeedShadow.leftAbsPeak) {
            gEncoderSpeedShadow.leftAbsPeak =
                gEncoderSpeedShadow.leftAbsDelta;
        }
        if (gEncoderSpeedShadow.rightAbsDelta >
            gEncoderSpeedShadow.rightAbsPeak) {
            gEncoderSpeedShadow.rightAbsPeak =
                gEncoderSpeedShadow.rightAbsDelta;
        }
    }
    if ((gEncoderSpeedShadow.leftAbsDelta != 0U) &&
        (gEncoderSpeedShadow.rightAbsDelta != 0U)) {
        gEncoderSpeedShadow.bothMotionSampleCount++;
        gEncoderSpeedShadow.leftBothAbsSum +=
            gEncoderSpeedShadow.leftAbsDelta;
        gEncoderSpeedShadow.rightBothAbsSum +=
            gEncoderSpeedShadow.rightAbsDelta;
    }

    gShadowPreviousLeftCount = snapshot.leftCount;
    gShadowPreviousRightCount = snapshot.rightCount;
    gShadowPreviousInvalidTransitions = snapshot.leftInvalidTransitions;
    gShadowLastSampleMs = nowMs;
    gShadowNextSampleMs += ENCODER_SPEED_SHADOW_PERIOD_MS;
    if ((int32_t) (nowMs - gShadowNextSampleMs) >= 0) {
        gShadowNextSampleMs = nowMs + ENCODER_SPEED_SHADOW_PERIOD_MS;
    }
}

EncoderSpeedShadow Encoders_speedShadowSnapshot(void)
{
    EncoderSpeedShadow snapshot;

    __disable_irq();
    snapshot = gEncoderSpeedShadow;
    __enable_irq();
    return snapshot;
}

void Encoders_onLeftEncoderAInterrupt(void)
{
    /* One count per A rising edge. B selects direction. The sign can be
     * inverted later without affecting straight-line speed magnitude. */
    if (DL_GPIO_readPins(DIAG_GPIO_LEFT_ENCODER_B_PORT,
            DIAG_GPIO_LEFT_ENCODER_B_PIN) != 0U) {
        gLeftCount--;
    } else {
        gLeftCount++;
    }
    gLeftState = readLeftState();
}

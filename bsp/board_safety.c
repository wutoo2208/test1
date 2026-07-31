#include "bsp/board_safety.h"

#include "drivers/motor_driver.h"
#include "ti_msp_dl_config.h"

static BoardSafetyStatus gStatus;

static void disableD36a(void)
{
    DL_GPIO_clearPins(DIAG_GPIO_D36A_EN_SAFE_PORT,
        DIAG_GPIO_D36A_EN_SAFE_PIN);
    DL_GPIO_clearPins(DIAG_GPIO_D36A_DIR_SAFE_PORT,
        DIAG_GPIO_D36A_DIR_SAFE_PIN);
    DL_GPIO_clearPins(DIAG_GPIO_D36A_STEP_SAFE_PORT,
        DIAG_GPIO_D36A_STEP_SAFE_PIN);
}

static bool d36aDisabled(void)
{
    return (DL_GPIO_readPins(DIAG_GPIO_D36A_EN_SAFE_PORT,
                DIAG_GPIO_D36A_EN_SAFE_PIN) == 0U) &&
        (DL_GPIO_readPins(DIAG_GPIO_D36A_DIR_SAFE_PORT,
             DIAG_GPIO_D36A_DIR_SAFE_PIN) == 0U) &&
        (DL_GPIO_readPins(DIAG_GPIO_D36A_STEP_SAFE_PORT,
             DIAG_GPIO_D36A_STEP_SAFE_PIN) == 0U);
}

void BoardSafety_init(void)
{
    gStatus.faultLatched = false;
    BoardSafety_stop(BOARD_SAFETY_STOP_BOOT);
}

void BoardSafety_service(void)
{
    gStatus.motorStopped = MotorDriver_outputsStopped();
    gStatus.d36aDisabled = d36aDisabled();
    if (!gStatus.d36aDisabled) {
        gStatus.faultLatched = true;
        BoardSafety_stop(BOARD_SAFETY_STOP_FAULT);
    }
}

void BoardSafety_stop(BoardSafetyStopReason reason)
{
    MotorDriver_stopAll();
    disableD36a();
    gStatus.lastStopReason = reason;
    gStatus.motorStopped = MotorDriver_outputsStopped();
    gStatus.d36aDisabled = d36aDisabled();
    if (reason == BOARD_SAFETY_STOP_FAULT) {
        gStatus.faultLatched = true;
    }
}

bool BoardSafety_outputsSafe(void)
{
    return MotorDriver_outputsStopped() && d36aDisabled();
}

BoardSafetyStatus BoardSafety_snapshot(void)
{
    BoardSafetyStatus status = gStatus;
    status.motorStopped = MotorDriver_outputsStopped();
    status.d36aDisabled = d36aDisabled();
    return status;
}

bool BoardSafety_faultLatched(void)
{
    return gStatus.faultLatched;
}

const char *BoardSafety_buzzerPolicy(void)
{
    return "PB27_DNC_PHYSICALLY_DISCONNECTED_POLARITY_UNKNOWN";
}

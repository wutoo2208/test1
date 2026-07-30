#include "bsp/board_safety.h"

#include "ti_msp_dl_config.h"

static void forceLockedOutputs(void)
{
    DL_GPIO_clearPins(DIAG_GPIO_MOTOR_AIN1_SAFE_PORT,
        DIAG_GPIO_MOTOR_AIN1_SAFE_PIN);
    DL_GPIO_clearPins(DIAG_GPIO_MOTOR_AIN2_SAFE_PORT,
        DIAG_GPIO_MOTOR_AIN2_SAFE_PIN);
    DL_GPIO_clearPins(DIAG_GPIO_MOTOR_BIN1_SAFE_PORT,
        DIAG_GPIO_MOTOR_BIN1_SAFE_PIN);
    DL_GPIO_clearPins(DIAG_GPIO_MOTOR_BIN2_SAFE_PORT,
        DIAG_GPIO_MOTOR_BIN2_SAFE_PIN);
    DL_GPIO_clearPins(DIAG_GPIO_D36A_EN_SAFE_PORT,
        DIAG_GPIO_D36A_EN_SAFE_PIN);
    DL_GPIO_clearPins(DIAG_GPIO_D36A_DIR_SAFE_PORT,
        DIAG_GPIO_D36A_DIR_SAFE_PIN);
    DL_GPIO_clearPins(DIAG_GPIO_D36A_STEP_SAFE_PORT,
        DIAG_GPIO_D36A_STEP_SAFE_PIN);
}

void BoardSafety_init(void)
{
    forceLockedOutputs();
}

void BoardSafety_service(void)
{
    forceLockedOutputs();
}

bool BoardSafety_outputsLocked(void)
{
    bool safe = true;

    safe &= (DL_GPIO_readPins(DIAG_GPIO_MOTOR_AIN1_SAFE_PORT,
                 DIAG_GPIO_MOTOR_AIN1_SAFE_PIN) == 0U);
    safe &= (DL_GPIO_readPins(DIAG_GPIO_MOTOR_AIN2_SAFE_PORT,
                 DIAG_GPIO_MOTOR_AIN2_SAFE_PIN) == 0U);
    safe &= (DL_GPIO_readPins(DIAG_GPIO_MOTOR_BIN1_SAFE_PORT,
                 DIAG_GPIO_MOTOR_BIN1_SAFE_PIN) == 0U);
    safe &= (DL_GPIO_readPins(DIAG_GPIO_MOTOR_BIN2_SAFE_PORT,
                 DIAG_GPIO_MOTOR_BIN2_SAFE_PIN) == 0U);
    safe &= (DL_GPIO_readPins(DIAG_GPIO_D36A_EN_SAFE_PORT,
                 DIAG_GPIO_D36A_EN_SAFE_PIN) == 0U);
    safe &= (DL_GPIO_readPins(DIAG_GPIO_D36A_DIR_SAFE_PORT,
                 DIAG_GPIO_D36A_DIR_SAFE_PIN) == 0U);
    safe &= (DL_GPIO_readPins(DIAG_GPIO_D36A_STEP_SAFE_PORT,
                 DIAG_GPIO_D36A_STEP_SAFE_PIN) == 0U);
    return safe;
}

const char *BoardSafety_buzzerPolicy(void)
{
    return "PB27_DNC_PHYSICALLY_DISCONNECTED_POLARITY_UNKNOWN";
}

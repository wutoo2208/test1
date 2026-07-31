#include "drivers/motor_driver.h"

#include "ti_msp_dl_config.h"

#define MOTOR_PWM_FREQUENCY_HZ (20000U)
#define MOTOR_PWM_PERIOD_COUNTS \
    (MOTOR_PWM_INST_CLK_FREQ / MOTOR_PWM_FREQUENCY_HZ)

static MotorDriverStatus gStatus;

static void setDuty(uint32_t channel, uint16_t permille)
{
    uint32_t compare = MOTOR_PWM_PERIOD_COUNTS;

    if (permille != 0U) {
        compare = MOTOR_PWM_PERIOD_COUNTS -
            ((MOTOR_PWM_PERIOD_COUNTS * (uint32_t) permille) / 1000U);
    }
    DL_TimerA_setCaptureCompareValue(MOTOR_PWM_INST, compare, channel);
}

void MotorDriver_stopAll(void)
{
    setDuty(GPIO_MOTOR_PWM_C0_IDX, 0U);
    setDuty(GPIO_MOTOR_PWM_C2_IDX, 0U);
    DL_GPIO_clearPins(DIAG_GPIO_MOTOR_AIN2_SAFE_PORT,
        DIAG_GPIO_MOTOR_AIN2_SAFE_PIN);
    DL_GPIO_clearPins(DIAG_GPIO_MOTOR_BIN2_SAFE_PORT,
        DIAG_GPIO_MOTOR_BIN2_SAFE_PIN);
    gStatus.leftDutyPermille = 0U;
    gStatus.rightDutyPermille = 0U;
}

void MotorDriver_init(void)
{
    gStatus.pwmCounterRunning = false;
    MotorDriver_stopAll();
    DL_TimerA_startCounter(MOTOR_PWM_INST);
    gStatus.pwmCounterRunning = true;
}

MotorDriverResult MotorDriver_driveSinglePrimary(MotorWheel wheel,
    uint16_t dutyPermille)
{
    if ((dutyPermille == 0U) || (dutyPermille > 1000U)) {
        MotorDriver_stopAll();
        return MOTOR_DRIVER_INVALID_DUTY;
    }
    if (!MotorDriver_outputsStopped()) {
        return MOTOR_DRIVER_BUSY;
    }

    MotorDriver_stopAll();
    if (wheel == MOTOR_WHEEL_LEFT) {
        setDuty(GPIO_MOTOR_PWM_C2_IDX, dutyPermille);
        gStatus.leftDutyPermille = dutyPermille;
    } else {
        setDuty(GPIO_MOTOR_PWM_C0_IDX, dutyPermille);
        gStatus.rightDutyPermille = dutyPermille;
    }
    return MOTOR_DRIVER_OK;
}

bool MotorDriver_outputsStopped(void)
{
    return (gStatus.leftDutyPermille == 0U) &&
        (gStatus.rightDutyPermille == 0U) &&
        (DL_TimerA_getCaptureCompareValue(MOTOR_PWM_INST,
             GPIO_MOTOR_PWM_C0_IDX) == MOTOR_PWM_PERIOD_COUNTS) &&
        (DL_TimerA_getCaptureCompareValue(MOTOR_PWM_INST,
             GPIO_MOTOR_PWM_C2_IDX) == MOTOR_PWM_PERIOD_COUNTS) &&
        (DL_GPIO_readPins(DIAG_GPIO_MOTOR_AIN2_SAFE_PORT,
             DIAG_GPIO_MOTOR_AIN2_SAFE_PIN) == 0U) &&
        (DL_GPIO_readPins(DIAG_GPIO_MOTOR_BIN2_SAFE_PORT,
             DIAG_GPIO_MOTOR_BIN2_SAFE_PIN) == 0U);
}

MotorDriverStatus MotorDriver_snapshot(void)
{
    return gStatus;
}

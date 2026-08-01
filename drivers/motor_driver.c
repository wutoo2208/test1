#include "drivers/motor_driver.h"

#include "ti_msp_dl_config.h"

#define MOTOR_PWM_FREQUENCY_HZ (1000U)
#define MOTOR_PWM_PERIOD_COUNTS \
    (MOTOR_PWM_INST_CLK_FREQ / MOTOR_PWM_FREQUENCY_HZ)
#define MOTOR_PWM_ZERO_COMPARE (MOTOR_PWM_PERIOD_COUNTS - 1U)

static MotorDriverStatus gStatus;

static void setDuty(uint32_t channel, uint16_t permille)
{
    /* Timer LOAD is period - 1. Using period skips the compare event and
     * leaves the edge-aligned output high, so zero duty must use LOAD. */
    uint32_t compare = MOTOR_PWM_ZERO_COMPARE;

    if (permille != 0U) {
        compare = MOTOR_PWM_PERIOD_COUNTS -
            ((MOTOR_PWM_PERIOD_COUNTS * (uint32_t) permille) / 1000U);
    }
    DL_TimerA_setCaptureCompareValue(MOTOR_PWM_INST, compare, channel);
}

static void setLeftVehicleForwardDuty(uint16_t dutyPermille)
{
    if (dutyPermille == 0U) {
        setDuty(GPIO_MOTOR_PWM_C2_IDX, 0U);
        DL_GPIO_clearPins(DIAG_GPIO_MOTOR_BIN2_SAFE_PORT,
            DIAG_GPIO_MOTOR_BIN2_SAFE_PIN);
    } else {
        if (gStatus.leftDutyPermille == 0U) {
            setDuty(GPIO_MOTOR_PWM_C2_IDX, 0U);
            DL_GPIO_setPins(DIAG_GPIO_MOTOR_BIN2_SAFE_PORT,
                DIAG_GPIO_MOTOR_BIN2_SAFE_PIN);
        }
        setDuty(GPIO_MOTOR_PWM_C2_IDX,
            (uint16_t) (1000U - dutyPermille));
    }
    gStatus.leftDutyPermille = dutyPermille;
}

static void setRightVehicleForwardDuty(uint16_t dutyPermille)
{
    DL_GPIO_clearPins(DIAG_GPIO_MOTOR_AIN2_SAFE_PORT,
        DIAG_GPIO_MOTOR_AIN2_SAFE_PIN);
    setDuty(GPIO_MOTOR_PWM_C0_IDX, dutyPermille);
    gStatus.rightDutyPermille = dutyPermille;
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
    /* Override the generated 20 kHz period. The U2 isolation/bridge path
     * passed static levels but did not pass the 20 kHz motor PWM in test. */
    DL_TimerA_setLoadValue(MOTOR_PWM_INST, MOTOR_PWM_ZERO_COMPARE);
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
    } else if (wheel == MOTOR_WHEEL_RIGHT) {
        setDuty(GPIO_MOTOR_PWM_C0_IDX, dutyPermille);
        gStatus.rightDutyPermille = dutyPermille;
    } else if (wheel == MOTOR_WHEEL_BOTH) {
        setDuty(GPIO_MOTOR_PWM_C2_IDX, dutyPermille);
        setDuty(GPIO_MOTOR_PWM_C0_IDX, dutyPermille);
        gStatus.leftDutyPermille = dutyPermille;
        gStatus.rightDutyPermille = dutyPermille;
    } else {
        MotorDriver_stopAll();
        return MOTOR_DRIVER_INVALID_DUTY;
    }
    return MOTOR_DRIVER_OK;
}

MotorDriverResult MotorDriver_setVehicleForwardDuties(
    uint16_t leftDutyPermille, uint16_t rightDutyPermille)
{
    if ((leftDutyPermille > 1000U) || (rightDutyPermille > 1000U)) {
        MotorDriver_stopAll();
        return MOTOR_DRIVER_INVALID_DUTY;
    }

    /* Ground-verified vehicle-forward polarity:
     * left B channel reverses with BIN2 high and complemented BIN1 PWM;
     * right A channel uses normal AIN1 PWM with AIN2 low. */
    setLeftVehicleForwardDuty(leftDutyPermille);
    setRightVehicleForwardDuty(rightDutyPermille);
    return MOTOR_DRIVER_OK;
}

MotorDriverResult MotorDriver_driveVehicleForward(MotorWheel wheel,
    uint16_t dutyPermille)
{
    uint16_t leftDuty = 0U;
    uint16_t rightDuty = 0U;

    if ((dutyPermille == 0U) || (dutyPermille > 1000U)) {
        MotorDriver_stopAll();
        return MOTOR_DRIVER_INVALID_DUTY;
    }
    if (!MotorDriver_outputsStopped()) {
        return MOTOR_DRIVER_BUSY;
    }

    if (wheel == MOTOR_WHEEL_LEFT) {
        leftDuty = dutyPermille;
    } else if (wheel == MOTOR_WHEEL_RIGHT) {
        rightDuty = dutyPermille;
    } else if (wheel == MOTOR_WHEEL_BOTH) {
        leftDuty = dutyPermille;
        rightDuty = dutyPermille;
    } else {
        MotorDriver_stopAll();
        return MOTOR_DRIVER_INVALID_DUTY;
    }

    MotorDriver_stopAll();
    return MotorDriver_setVehicleForwardDuties(leftDuty, rightDuty);
}
bool MotorDriver_outputsStopped(void)
{
    return (gStatus.leftDutyPermille == 0U) &&
        (gStatus.rightDutyPermille == 0U) &&
        (DL_TimerA_getCaptureCompareValue(MOTOR_PWM_INST,
             GPIO_MOTOR_PWM_C0_IDX) == MOTOR_PWM_ZERO_COMPARE) &&
        (DL_TimerA_getCaptureCompareValue(MOTOR_PWM_INST,
             GPIO_MOTOR_PWM_C2_IDX) == MOTOR_PWM_ZERO_COMPARE) &&
        (DL_GPIO_readPins(DIAG_GPIO_MOTOR_AIN2_SAFE_PORT,
             DIAG_GPIO_MOTOR_AIN2_SAFE_PIN) == 0U) &&
        (DL_GPIO_readPins(DIAG_GPIO_MOTOR_BIN2_SAFE_PORT,
             DIAG_GPIO_MOTOR_BIN2_SAFE_PIN) == 0U);
}

MotorDriverStatus MotorDriver_snapshot(void)
{
    return gStatus;
}

#include "drivers/i2c_diag.h"

#include "config/firmware_config.h"
#include "ti_msp_dl_config.h"

static bool waitComplete(I2C_Regs *instance)
{
    uint32_t timeout = I2C_DIAG_TIMEOUT_LOOPS;

    while (timeout-- != 0U) {
        uint32_t status = DL_I2C_getControllerStatus(instance);
        if ((status & DL_I2C_CONTROLLER_STATUS_BUSY) == 0U) {
            return (status & DL_I2C_CONTROLLER_STATUS_ERROR) == 0U;
        }
    }
    return false;
}

static void prepare(I2C_Regs *instance)
{
    DL_I2C_resetControllerTransfer(instance);
    DL_I2C_flushControllerTXFIFO(instance);
    DL_I2C_flushControllerRXFIFO(instance);
}

static bool probe(I2C_Regs *instance, uint8_t address)
{
    prepare(instance);
    DL_I2C_startControllerTransfer(instance, address,
        DL_I2C_CONTROLLER_DIRECTION_TX, 0U);
    delay_cycles(12U);
    return waitComplete(instance);
}

static bool readRegister(
    I2C_Regs *instance, uint8_t address, uint8_t reg, uint8_t *value)
{
    prepare(instance);
    DL_I2C_fillControllerTXFIFO(instance, &reg, 1U);
    DL_I2C_startControllerTransfer(instance, address,
        DL_I2C_CONTROLLER_DIRECTION_TX, 1U);
    delay_cycles(12U);
    if (!waitComplete(instance)) return false;

    prepare(instance);
    DL_I2C_startControllerTransfer(instance, address,
        DL_I2C_CONTROLLER_DIRECTION_RX, 1U);
    delay_cycles(12U);
    if (!waitComplete(instance) ||
        DL_I2C_isControllerRXFIFOEmpty(instance)) return false;
    *value = DL_I2C_receiveControllerData(instance);
    return true;
}

I2cDiagScan I2cDiag_scan(void)
{
    I2cDiagScan scan;
    scan.oled3c = probe(OLED_I2C_INST, 0x3CU);
    scan.oled3d = probe(OLED_I2C_INST, 0x3DU);
    scan.mpu68 = probe(MPU_I2C_INST, 0x68U);
    scan.mpu69 = probe(MPU_I2C_INST, 0x69U);
    return scan;
}

I2cDiagMpu I2cDiag_readMpuWhoAmI(void)
{
    I2cDiagMpu result = {false, 0U, 0U};
    uint8_t address;

    for (address = 0x68U; address <= 0x69U; address++) {
        if (readRegister(MPU_I2C_INST, address, 0x75U,
                &result.whoAmI)) {
            result.found = true;
            result.address = address;
            break;
        }
    }
    return result;
}

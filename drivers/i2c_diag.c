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

I2cDiagScan I2cDiag_scan(void)
{
    I2cDiagScan scan;

    scan.line5c = probe(LINE_I2C_INST, 0x5CU);
    scan.oled3c = probe(OLED_I2C_INST, 0x3CU);
    scan.oled3d = probe(OLED_I2C_INST, 0x3DU);
    return scan;
}

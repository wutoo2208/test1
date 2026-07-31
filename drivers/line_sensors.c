#include "drivers/line_sensors.h"

#include <string.h>

#include "bsp/timebase.h"
#include "config/firmware_config.h"
#include "ti_msp_dl_config.h"

#define LINE_SENSOR_I2C_ADDRESS       (0x5CU)
#define LINE_SENSOR_DIGITAL_REGISTER  (0x05U)
#define LINE_SENSOR_ANALOG_REGISTER   (0x06U)
#define LINE_SENSOR_ANALOG_BYTES      (LINE_SENSOR_COUNT * 2U)

static LineSensorSample gSample;
static uint32_t gNextPollMs;

static bool recordFailure(
    LineSensorFailureStage stage, uint8_t reg, uint8_t received)
{
    gSample.lastFailureStage = (uint8_t) stage;
    gSample.lastRegister = reg;
    gSample.lastReceivedBytes = received;
    gSample.lastControllerStatus =
        DL_I2C_getControllerStatus(LINE_I2C_INST);
    return false;
}

static void resetTransfer(void)
{
    DL_I2C_resetControllerTransfer(LINE_I2C_INST);
    DL_I2C_flushControllerTXFIFO(LINE_I2C_INST);
    DL_I2C_flushControllerRXFIFO(LINE_I2C_INST);
}

static void recoverController(void)
{
    DL_I2C_reset(LINE_I2C_INST);
    DL_I2C_enablePower(LINE_I2C_INST);
    delay_cycles(POWER_STARTUP_DELAY);
    SYSCFG_DL_LINE_I2C_init();
}

static bool controllerHasError(uint32_t status)
{
    return (status & (DL_I2C_CONTROLLER_STATUS_ERROR |
        DL_I2C_CONTROLLER_STATUS_ARBITRATION_LOST)) != 0U;
}

static bool waitForBusIdle(void)
{
    uint32_t timeout = I2C_DIAG_TIMEOUT_LOOPS;

    while (timeout-- != 0U) {
        uint32_t status = DL_I2C_getControllerStatus(LINE_I2C_INST);
        if (controllerHasError(status)) return false;
        if ((status & (DL_I2C_CONTROLLER_STATUS_BUSY |
                       DL_I2C_CONTROLLER_STATUS_BUSY_BUS)) == 0U) return true;
    }
    return false;
}

static bool waitForTxDone(void)
{
    uint32_t timeout = I2C_DIAG_TIMEOUT_LOOPS;

    while (timeout-- != 0U) {
        uint32_t status = DL_I2C_getControllerStatus(LINE_I2C_INST);
        if (controllerHasError(status)) return false;
        if (DL_I2C_getRawInterruptStatus(LINE_I2C_INST,
                DL_I2C_INTERRUPT_CONTROLLER_TX_DONE) != 0U) return true;
    }
    return false;
}

static bool readRegisterBlock(uint8_t reg, uint8_t *data, uint8_t length)
{
    uint32_t timeout;
    uint8_t received = 0U;

    if (!waitForBusIdle()) {
        return recordFailure(LINE_SENSOR_FAILURE_BUS_IDLE, reg, 0U);
    }
    resetTransfer();
    DL_I2C_clearInterruptStatus(LINE_I2C_INST,
        DL_I2C_INTERRUPT_CONTROLLER_TX_DONE);
    if (DL_I2C_fillControllerTXFIFO(LINE_I2C_INST, &reg, 1U) != 1U) {
        resetTransfer();
        return recordFailure(LINE_SENSOR_FAILURE_TX_FIFO, reg, 0U);
    }

    DL_I2C_startControllerTransferAdvanced(LINE_I2C_INST,
        LINE_SENSOR_I2C_ADDRESS, DL_I2C_CONTROLLER_DIRECTION_TX, 1U,
        DL_I2C_CONTROLLER_START_ENABLE, DL_I2C_CONTROLLER_STOP_DISABLE,
        DL_I2C_CONTROLLER_ACK_DISABLE);
    delay_cycles(12U);
    if (!waitForTxDone()) {
        recordFailure(LINE_SENSOR_FAILURE_TX_DONE, reg, 0U);
        resetTransfer();
        return false;
    }
    DL_I2C_clearInterruptStatus(LINE_I2C_INST,
        DL_I2C_INTERRUPT_CONTROLLER_TX_DONE);

    DL_I2C_clearInterruptStatus(LINE_I2C_INST,
        DL_I2C_INTERRUPT_CONTROLLER_RX_DONE);
    DL_I2C_startControllerTransferAdvanced(LINE_I2C_INST,
        LINE_SENSOR_I2C_ADDRESS, DL_I2C_CONTROLLER_DIRECTION_RX, length,
        DL_I2C_CONTROLLER_START_ENABLE, DL_I2C_CONTROLLER_STOP_ENABLE,
        DL_I2C_CONTROLLER_ACK_DISABLE);
    delay_cycles(12U);

    timeout = I2C_DIAG_TIMEOUT_LOOPS;
    while (timeout-- != 0U) {
        uint32_t status = DL_I2C_getControllerStatus(LINE_I2C_INST);

        if (controllerHasError(status)) break;
        while (!DL_I2C_isControllerRXFIFOEmpty(LINE_I2C_INST)) {
            uint8_t value = DL_I2C_receiveControllerData(LINE_I2C_INST);
            if (received < length) data[received++] = value;
        }
        if (DL_I2C_getRawInterruptStatus(LINE_I2C_INST,
                DL_I2C_INTERRUPT_CONTROLLER_RX_DONE) != 0U) break;
    }

    while (!DL_I2C_isControllerRXFIFOEmpty(LINE_I2C_INST)) {
        uint8_t value = DL_I2C_receiveControllerData(LINE_I2C_INST);
        if (received < length) data[received++] = value;
    }

    DL_I2C_clearInterruptStatus(LINE_I2C_INST,
        DL_I2C_INTERRUPT_CONTROLLER_RX_DONE);
    if (received != length) {
        recordFailure(LINE_SENSOR_FAILURE_RX_DATA, reg, received);
        resetTransfer();
        return false;
    }
    if (!waitForBusIdle()) {
        recordFailure(LINE_SENSOR_FAILURE_BUS_RELEASE, reg, received);
        resetTransfer();
        return false;
    }
    return true;
}

void LineSensors_init(uint32_t nowMs)
{
    memset(&gSample, 0, sizeof(gSample));
    gNextPollMs = nowMs;
}

void LineSensors_service(uint32_t nowMs)
{
    uint32_t scheduledMs;
    uint32_t completedMs;
    uint8_t digital;
    uint8_t analogBytes[LINE_SENSOR_ANALOG_BYTES];
    uint8_t index;

    if (!Timebase_reached(nowMs, gNextPollMs)) return;
    scheduledMs = nowMs + LINE_SENSOR_POLL_PERIOD_MS;

    if (!readRegisterBlock(LINE_SENSOR_DIGITAL_REGISTER, &digital, 1U) ||
        !readRegisterBlock(LINE_SENSOR_ANALOG_REGISTER, analogBytes,
            sizeof(analogBytes))) {
        recoverController();
        gSample.valid = false;
        gSample.errorCount++;
        gNextPollMs = Timebase_nowMs() + LINE_SENSOR_RETRY_PERIOD_MS;
        return;
    }

    gSample.digitalBits = digital & 0x3FU;
    for (index = 0U; index < LINE_SENSOR_COUNT; index++) {
        uint8_t offset = index * 2U;
        gSample.analog[index] = (uint16_t) analogBytes[offset] |
            ((uint16_t) analogBytes[offset + 1U] << 8U);
    }
    completedMs = Timebase_nowMs();
    gSample.lastSuccessMs = completedMs;
    gSample.sequence++;
    gSample.valid = true;
    gNextPollMs = Timebase_reached(completedMs, scheduledMs) ?
        (completedMs + LINE_SENSOR_POLL_PERIOD_MS) : scheduledMs;
}

LineSensorSample LineSensors_snapshot(void)
{
    return gSample;
}

uint8_t LineSensors_readRawBits(void)
{
    return gSample.digitalBits;
}

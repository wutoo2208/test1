#include "drivers/oled_ssd1306.h"

#include <stdbool.h>
#include <stdint.h>

#include "config/firmware_config.h"
#include "ti_msp_dl_config.h"

#define OLED_ADDRESS           (0x3CU)
#define OLED_CONTROL_COMMAND   (0x00U)
#define OLED_CONTROL_DATA      (0x40U)
#define OLED_WIDTH             (128U)
#define OLED_PAGES             (8U)
#define OLED_CHUNK_DATA        (7U)

static OledSsd1306Status gStatus;

static bool waitComplete(void)
{
    uint32_t timeout = I2C_DIAG_TIMEOUT_LOOPS;

    while (timeout-- != 0U) {
        uint32_t status = DL_I2C_getControllerStatus(OLED_I2C_INST);
        if ((status & DL_I2C_CONTROLLER_STATUS_BUSY) == 0U) {
            return (status & DL_I2C_CONTROLLER_STATUS_ERROR) == 0U;
        }
    }
    return false;
}

static void prepare(void)
{
    DL_I2C_resetControllerTransfer(OLED_I2C_INST);
    DL_I2C_flushControllerTXFIFO(OLED_I2C_INST);
    DL_I2C_flushControllerRXFIFO(OLED_I2C_INST);
}

static bool writeSmall(const uint8_t *data, uint8_t length)
{
    prepare();
    if (DL_I2C_fillControllerTXFIFO(OLED_I2C_INST, data, length) != length) {
        return false;
    }
    DL_I2C_startControllerTransfer(OLED_I2C_INST, OLED_ADDRESS,
        DL_I2C_CONTROLLER_DIRECTION_TX, length);
    delay_cycles(12U);
    return waitComplete();
}

static bool writeCommand(uint8_t command)
{
    uint8_t packet[2] = {OLED_CONTROL_COMMAND, command};
    return writeSmall(packet, sizeof(packet));
}

static bool setPageAndColumn(uint8_t page, uint8_t column)
{
    return writeCommand((uint8_t) (0xB0U | page)) &&
        writeCommand((uint8_t) (column & 0x0FU)) &&
        writeCommand((uint8_t) (0x10U | (column >> 4U)));
}

static bool writePageData(uint8_t page, const uint8_t *data)
{
    uint8_t column;

    if (!setPageAndColumn(page, 0U)) return false;
    for (column = 0U; column < OLED_WIDTH; column += OLED_CHUNK_DATA) {
        uint8_t packet[OLED_CHUNK_DATA + 1U];
        uint8_t count = (uint8_t) (OLED_WIDTH - column);
        uint8_t index;

        if (count > OLED_CHUNK_DATA) count = OLED_CHUNK_DATA;
        packet[0] = OLED_CONTROL_DATA;
        for (index = 0U; index < count; index++) {
            packet[index + 1U] = data[column + index];
        }
        if (!writeSmall(packet, (uint8_t) (count + 1U))) return false;
    }
    return true;
}

static bool initialize(void)
{
    static const uint8_t commands[] = {
        0xAEU, 0xD5U, 0x80U, 0xA8U, 0x3FU, 0xD3U, 0x00U, 0x40U,
        0x8DU, 0x14U, 0x20U, 0x02U, 0xA1U, 0xC8U, 0xDAU, 0x12U,
        0x81U, 0x7FU, 0xD9U, 0xF1U, 0xDBU, 0x40U, 0xA4U, 0xA6U
    };
    uint8_t index;

    for (index = 0U; index < sizeof(commands); index++) {
        if (!writeCommand(commands[index])) return false;
    }
    gStatus.initialized = true;
    return true;
}

bool OledSsd1306_showTestPattern(void)
{
    uint8_t data[OLED_WIDTH];
    uint8_t page;
    uint8_t column;
    bool ok;

    gStatus.initialized = false;
    gStatus.lastTransferOk = false;
    if (!initialize()) return false;

    for (page = 0U; page < OLED_PAGES; page++) {
        for (column = 0U; column < OLED_WIDTH; column++) {
            bool border = (page == 0U) || (page == (OLED_PAGES - 1U)) ||
                (column == 0U) || (column == (OLED_WIDTH - 1U));
            bool checker = (((column >> 3U) + page) & 1U) != 0U;
            data[column] = border ? 0xFFU : (checker ? 0x18U : 0x24U);
        }
        if (!writePageData(page, data)) return false;
    }
    ok = writeCommand(0xAFU);
    gStatus.lastTransferOk = ok;
    return ok;
}

bool OledSsd1306_forceAllPixelsOn(void)
{
    bool ok;

    gStatus.initialized = false;
    gStatus.lastTransferOk = false;
    ok = writeCommand(0xAEU) &&
        writeCommand(0x8DU) && writeCommand(0x14U) &&
        writeCommand(0xAFU) && writeCommand(0xA5U);
    gStatus.initialized = ok;
    gStatus.lastTransferOk = ok;
    return ok;
}

bool OledSsd1306_forceAllPixelsOnSh1106(void)
{
    bool ok;

    gStatus.initialized = false;
    gStatus.lastTransferOk = false;
    ok = writeCommand(0xAEU) &&
        writeCommand(0xADU) && writeCommand(0x8BU);
    if (ok) {
        delay_cycles(CPUCLK_FREQ / 20U);
        ok = writeCommand(0xAFU) && writeCommand(0xA5U);
    }
    gStatus.initialized = ok;
    gStatus.lastTransferOk = ok;
    return ok;
}

void OledSsd1306_clear(void)
{
    uint8_t data[OLED_WIDTH] = {0U};
    uint8_t page;
    bool ok = gStatus.initialized;

    for (page = 0U; ok && (page < OLED_PAGES); page++) {
        ok = writePageData(page, data);
    }
    if (ok) ok = writeCommand(0xAEU);
    gStatus.lastTransferOk = ok;
}

OledSsd1306Status OledSsd1306_getStatus(void)
{
    return gStatus;
}

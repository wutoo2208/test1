#include "drivers/oled_ssd1306.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "config/firmware_config.h"
#include "ti_msp_dl_config.h"

#define OLED_ADDRESS_PRIMARY   (0x3CU)
#define OLED_ADDRESS_SECONDARY (0x3DU)
#define OLED_CONTROL_COMMAND   (0x00U)
#define OLED_CONTROL_DATA      (0x40U)
#define OLED_WIDTH             (128U)
#define OLED_PAGES             (8U)
#define OLED_CHUNK_DATA        (7U)

static OledSsd1306Status gStatus;
static uint8_t gAddress = OLED_ADDRESS_PRIMARY;

static bool waitComplete(void)
{
    uint32_t timeout = I2C_DIAG_TIMEOUT_LOOPS;
    bool txDone = false;

    while (timeout-- != 0U) {
        uint32_t status = DL_I2C_getControllerStatus(OLED_I2C_INST);
        if ((status & DL_I2C_CONTROLLER_STATUS_ERROR) != 0U) return false;
        if (DL_I2C_getRawInterruptStatus(OLED_I2C_INST,
                DL_I2C_INTERRUPT_CONTROLLER_TX_DONE) != 0U) {
            txDone = true;
            break;
        }
    }
    DL_I2C_clearInterruptStatus(OLED_I2C_INST,
        DL_I2C_INTERRUPT_CONTROLLER_TX_DONE);
    if (!txDone) return false;

    timeout = I2C_DIAG_TIMEOUT_LOOPS;
    while (timeout-- != 0U) {
        uint32_t status = DL_I2C_getControllerStatus(OLED_I2C_INST);
        if ((status & DL_I2C_CONTROLLER_STATUS_ERROR) != 0U) return false;
        if ((status & (DL_I2C_CONTROLLER_STATUS_BUSY |
                       DL_I2C_CONTROLLER_STATUS_BUSY_BUS)) == 0U) return true;
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
    DL_I2C_clearInterruptStatus(OLED_I2C_INST,
        DL_I2C_INTERRUPT_CONTROLLER_TX_DONE);
    if (DL_I2C_fillControllerTXFIFO(OLED_I2C_INST, data, length) != length) {
        return false;
    }
    DL_I2C_startControllerTransfer(OLED_I2C_INST, gAddress,
        DL_I2C_CONTROLLER_DIRECTION_TX, length);
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

static const uint8_t gDigits[10][5] = {
    {0x3EU, 0x51U, 0x49U, 0x45U, 0x3EU},
    {0x00U, 0x42U, 0x7FU, 0x40U, 0x00U},
    {0x42U, 0x61U, 0x51U, 0x49U, 0x46U},
    {0x21U, 0x41U, 0x45U, 0x4BU, 0x31U},
    {0x18U, 0x14U, 0x12U, 0x7FU, 0x10U},
    {0x27U, 0x45U, 0x45U, 0x45U, 0x39U},
    {0x3CU, 0x4AU, 0x49U, 0x49U, 0x30U},
    {0x01U, 0x71U, 0x09U, 0x05U, 0x03U},
    {0x36U, 0x49U, 0x49U, 0x49U, 0x36U},
    {0x06U, 0x49U, 0x49U, 0x29U, 0x1EU}
};

static const uint8_t gLetters[26][5] = {
    {0x7EU, 0x11U, 0x11U, 0x11U, 0x7EU},
    {0x7FU, 0x49U, 0x49U, 0x49U, 0x36U},
    {0x3EU, 0x41U, 0x41U, 0x41U, 0x22U},
    {0x7FU, 0x41U, 0x41U, 0x22U, 0x1CU},
    {0x7FU, 0x49U, 0x49U, 0x49U, 0x41U},
    {0x7FU, 0x09U, 0x09U, 0x09U, 0x01U},
    {0x3EU, 0x41U, 0x49U, 0x49U, 0x7AU},
    {0x7FU, 0x08U, 0x08U, 0x08U, 0x7FU},
    {0x00U, 0x41U, 0x7FU, 0x41U, 0x00U},
    {0x20U, 0x40U, 0x41U, 0x3FU, 0x01U},
    {0x7FU, 0x08U, 0x14U, 0x22U, 0x41U},
    {0x7FU, 0x40U, 0x40U, 0x40U, 0x40U},
    {0x7FU, 0x02U, 0x0CU, 0x02U, 0x7FU},
    {0x7FU, 0x04U, 0x08U, 0x10U, 0x7FU},
    {0x3EU, 0x41U, 0x41U, 0x41U, 0x3EU},
    {0x7FU, 0x09U, 0x09U, 0x09U, 0x06U},
    {0x3EU, 0x41U, 0x51U, 0x21U, 0x5EU},
    {0x7FU, 0x09U, 0x19U, 0x29U, 0x46U},
    {0x46U, 0x49U, 0x49U, 0x49U, 0x31U},
    {0x01U, 0x01U, 0x7FU, 0x01U, 0x01U},
    {0x3FU, 0x40U, 0x40U, 0x40U, 0x3FU},
    {0x1FU, 0x20U, 0x40U, 0x20U, 0x1FU},
    {0x3FU, 0x40U, 0x38U, 0x40U, 0x3FU},
    {0x63U, 0x14U, 0x08U, 0x14U, 0x63U},
    {0x03U, 0x04U, 0x78U, 0x04U, 0x03U},
    {0x61U, 0x51U, 0x49U, 0x45U, 0x43U}
};

static void glyphFor(char character, uint8_t glyph[5])
{
    const uint8_t *source = NULL;
    uint8_t index;

    if ((character >= '0') && (character <= '9')) {
        source = gDigits[(uint8_t) (character - '0')];
    } else if ((character >= 'A') && (character <= 'Z')) {
        source = gLetters[(uint8_t) (character - 'A')];
    }

    if (source != NULL) {
        memcpy(glyph, source, 5U);
    } else {
        memset(glyph, 0, 5U);
        if (character == '.') {
            glyph[1] = 0x60U;
            glyph[2] = 0x60U;
        } else if (character == ':') {
            glyph[1] = 0x36U;
            glyph[2] = 0x36U;
        } else if (character == '-') {
            for (index = 0U; index < 5U; index++) glyph[index] = 0x08U;
        }
    }
}

static void renderText(uint8_t data[OLED_WIDTH], const char *text)
{
    uint8_t column = 0U;

    memset(data, 0, OLED_WIDTH);
    while ((*text != '\0') && (column <= (OLED_WIDTH - 6U))) {
        uint8_t glyph[5];
        uint8_t index;

        glyphFor(*text++, glyph);
        for (index = 0U; index < 5U; index++) data[column++] = glyph[index];
        data[column++] = 0U;
    }
}

static void formatElapsed(char text[13], uint32_t elapsedMs)
{
    uint32_t seconds;
    uint32_t milliseconds;

    if (elapsedMs > 99999U) elapsedMs = 99999U;
    seconds = elapsedMs / 1000U;
    milliseconds = elapsedMs % 1000U;
    text[0] = 'T'; text[1] = 'I'; text[2] = 'M'; text[3] = 'E'; text[4] = ' ';
    text[5] = (char) ('0' + ((seconds / 10U) % 10U));
    text[6] = (char) ('0' + (seconds % 10U));
    text[7] = '.';
    text[8] = (char) ('0' + (milliseconds / 100U));
    text[9] = (char) ('0' + ((milliseconds / 10U) % 10U));
    text[10] = (char) ('0' + (milliseconds % 10U));
    text[11] = 'S';
    text[12] = '\0';
}
static bool initializeAt(uint8_t address)
{
    static const uint8_t commands[] = {
        0xAEU, 0xD5U, 0x80U, 0xA8U, 0x3FU, 0xD3U, 0x00U, 0x40U,
        0x8DU, 0x14U, 0x20U, 0x02U, 0xA1U, 0xC8U, 0xDAU, 0x12U,
        0x81U, 0x7FU, 0xD9U, 0xF1U, 0xDBU, 0x40U, 0xA4U, 0xA6U
    };
    uint8_t index;

    gAddress = address;
    for (index = 0U; index < sizeof(commands); index++) {
        if (!writeCommand(commands[index])) return false;
    }
    gStatus.address = address;
    return true;
}

static bool initialize(void)
{
    gStatus.initialized = false;
    gStatus.lastTransferOk = false;
    gStatus.address = 0U;

    if (!initializeAt(OLED_ADDRESS_PRIMARY) &&
        !initializeAt(OLED_ADDRESS_SECONDARY)) {
        return false;
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
    if (!gStatus.initialized && !initialize()) return false;
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
    if (!gStatus.initialized && !initialize()) return false;
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

bool OledSsd1306_showReq002Status(OledReq002State state,
    uint32_t elapsedMs, bool lineValid)
{
    static const char *const stateText[] = {
        "READY", "RUN", "DONE", "FAULT"
    };
    uint8_t data[OLED_WIDTH];
    char elapsedText[13];
    uint8_t page;
    bool ok;

    if ((uint32_t) state > (uint32_t) OLED_REQ002_FAULT) {
        state = OLED_REQ002_FAULT;
    }
    if (!gStatus.initialized && !initialize()) {
        gStatus.lastTransferOk = false;
        return false;
    }

    formatElapsed(elapsedText, elapsedMs);
    ok = true;
    for (page = 0U; ok && (page < OLED_PAGES); page++) {
        if (page == 0U) {
            renderText(data, "REQ2");
        } else if (page == 2U) {
            renderText(data, stateText[state]);
        } else if (page == 4U) {
            renderText(data, elapsedText);
        } else if (page == 6U) {
            renderText(data, lineValid ? "LINE OK" : "LINE ERR");
        } else {
            memset(data, 0, sizeof(data));
        }
        ok = writePageData(page, data);
    }
    if (ok) ok = writeCommand(0xA4U) && writeCommand(0xAFU);
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

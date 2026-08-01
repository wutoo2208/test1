#include "drivers/nrf24_ptx.h"

#include <stddef.h>
#include <string.h>

#include "bsp/timebase.h"
#include "config/firmware_config.h"
#include "ti_msp_dl_config.h"

#define NRF_CMD_R_REGISTER          (0x00U)
#define NRF_CMD_W_REGISTER          (0x20U)
#define NRF_CMD_W_TX_PAYLOAD        (0xA0U)
#define NRF_CMD_FLUSH_TX            (0xE1U)
#define NRF_CMD_FLUSH_RX            (0xE2U)
#define NRF_CMD_NOP                 (0xFFU)
#define NRF_REGISTER_MASK           (0x1FU)

#define NRF_REG_CONFIG              (0x00U)
#define NRF_REG_EN_AA               (0x01U)
#define NRF_REG_EN_RXADDR           (0x02U)
#define NRF_REG_SETUP_AW            (0x03U)
#define NRF_REG_SETUP_RETR          (0x04U)
#define NRF_REG_RF_CH               (0x05U)
#define NRF_REG_RF_SETUP            (0x06U)
#define NRF_REG_STATUS              (0x07U)
#define NRF_REG_RX_ADDR_P0          (0x0AU)
#define NRF_REG_TX_ADDR             (0x10U)
#define NRF_REG_RX_PW_P0            (0x11U)
#define NRF_REG_DYNPD               (0x1CU)
#define NRF_REG_FEATURE             (0x1DU)

#define NRF_CONFIG_EN_CRC           (1U << 3U)
#define NRF_CONFIG_CRCO             (1U << 2U)
#define NRF_CONFIG_PWR_UP           (1U << 1U)
#define NRF_STATUS_RX_DR            (1U << 6U)
#define NRF_STATUS_TX_DS            (1U << 5U)
#define NRF_STATUS_MAX_RT           (1U << 4U)
#define NRF_STATUS_IRQ_MASK         (NRF_STATUS_RX_DR | NRF_STATUS_TX_DS | \
                                     NRF_STATUS_MAX_RT)
#define NRF_FEATURE_EN_DPL          (1U << 2U)
#define NRF_DYNPD_P0                (1U << 0U)
#define NRF_RF_DR_LOW               (1U << 5U)
#define NRF_RF_DR_HIGH              (1U << 3U)
#define NRF_RF_PWR_0_DBM             (3U << 1U)
#define RADIO_CE_PULSE_CYCLES       (CPUCLK_FREQ / 66667U)

typedef enum {
    RADIO_RATE_250_KBPS = 0,
    RADIO_RATE_1_MBPS,
    RADIO_RATE_2_MBPS
} RadioRate;

typedef enum {
    RADIO_STATE_DISABLED = 0,
    RADIO_STATE_PROFILE_INVALID,
    RADIO_STATE_POR_WAIT,
    RADIO_STATE_POWERUP_WAIT,
    RADIO_STATE_IDLE,
    RADIO_STATE_WAIT_RESULT,
    RADIO_STATE_FAULT
} RadioState;

typedef struct {
    bool valid;
    bool dynamicPayload;
    bool autoAck;
    uint8_t channel;
    RadioRate rate;
    uint8_t addressWidth;
    uint8_t address[5];
    uint8_t staticPayloadWidth;
    uint8_t retryDelay;
    uint8_t retryCount;
} RadioProfile;

typedef struct {
    uint8_t data[RADIO_MAX_PAYLOAD];
    uint8_t length;
    uint16_t messageId;
} RadioFrame;

typedef struct {
    uint32_t spiErrors;
    uint32_t initErrors;
    uint32_t txSuccess;
    uint32_t maxRetry;
    uint32_t txTimeout;
    uint32_t droppedLines;
    uint32_t droppedFrames;
} RadioStats;

/* Exact approved working-copy Baoqian USB V2.0 / NF-02-PA profile. */
static const RadioProfile gProfile = {
    .valid = (RADIO_PROFILE_VALID != 0U),
    .dynamicPayload = false,
    .autoAck = true,
    .channel = 0U,
    .rate = RADIO_RATE_2_MBPS,
    .addressWidth = 5U,
    .address = {0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU},
    .staticPayloadWidth = 32U,
    .retryDelay = 1U,
    .retryCount = 10U
};

static RadioFrame gQueue[RADIO_TX_QUEUE_SLOTS];
static RadioStats gStats;
static RadioState gState;
static uint8_t gQueueHead;
static uint8_t gQueueTail;
static uint8_t gQueueCount;
static uint16_t gNextMessageId;
static uint32_t gDeadline;
static bool gArmed;
static char gDiagLine[DIAG_LINE_MIRROR_SIZE];
static uint16_t gDiagLineLength;
static bool gDiagLineOverflow;
static bool gOneShotQueued;

static void ceLow(void)
{
    DL_GPIO_clearPins(DIAG_GPIO_RADIO_CE_PORT, DIAG_GPIO_RADIO_CE_PIN);
}

static void csnHigh(void)
{
    DL_GPIO_setPins(DIAG_GPIO_RADIO_CSN_PORT, DIAG_GPIO_RADIO_CSN_PIN);
}

static void forceSafe(void)
{
    ceLow();
    csnHigh();
}

static bool spiByte(uint8_t output, uint8_t *input)
{
    uint32_t timeout = RADIO_SPI_TIMEOUT_LOOPS;

    while (!DL_SPI_transmitDataCheck8(RADIO_SPI_INST, output)) {
        if (timeout-- == 0U) return false;
    }
    timeout = RADIO_SPI_TIMEOUT_LOOPS;
    while (!DL_SPI_receiveDataCheck8(RADIO_SPI_INST, input)) {
        if (timeout-- == 0U) return false;
    }
    return true;
}

static bool transaction(uint8_t command, const uint8_t *txData,
    uint8_t *rxData, uint8_t length, uint8_t *status)
{
    uint8_t discard[RADIO_MAX_PAYLOAD + 1U];
    uint8_t input;
    uint8_t index;
    uint32_t timeout;
    bool ok = true;

    if (length > RADIO_MAX_PAYLOAD) return false;
    forceSafe();
    (void) DL_SPI_drainRXFIFO8(RADIO_SPI_INST, discard, sizeof(discard));
    DL_GPIO_clearPins(DIAG_GPIO_RADIO_CSN_PORT, DIAG_GPIO_RADIO_CSN_PIN);

    if (!spiByte(command, &input)) {
        ok = false;
    } else {
        *status = input;
    }
    for (index = 0U; ok && (index < length); index++) {
        uint8_t output = (txData != NULL) ? txData[index] : 0xFFU;
        if (!spiByte(output, &input)) {
            ok = false;
        } else if (rxData != NULL) {
            rxData[index] = input;
        }
    }

    timeout = RADIO_SPI_TIMEOUT_LOOPS;
    while (DL_SPI_isBusy(RADIO_SPI_INST) && (timeout-- != 0U)) {
    }
    if (timeout == 0U) ok = false;
    csnHigh();
    if (!ok) gStats.spiErrors++;
    return ok;
}

static bool command(uint8_t value, uint8_t *status)
{
    return transaction(value, NULL, NULL, 0U, status);
}

static bool readRegisters(
    uint8_t reg, uint8_t *data, uint8_t length, uint8_t *status)
{
    return transaction((uint8_t) (NRF_CMD_R_REGISTER |
        (reg & NRF_REGISTER_MASK)), NULL, data, length, status);
}

static bool writeRegisters(
    uint8_t reg, const uint8_t *data, uint8_t length, uint8_t *status)
{
    return transaction((uint8_t) (NRF_CMD_W_REGISTER |
        (reg & NRF_REGISTER_MASK)), data, NULL, length, status);
}

static bool readRegister(uint8_t reg, uint8_t *value)
{
    uint8_t status;
    return readRegisters(reg, value, 1U, &status);
}

static bool writeRegister(uint8_t reg, uint8_t value)
{
    uint8_t status;
    return writeRegisters(reg, &value, 1U, &status);
}

static bool writeAndVerify(uint8_t reg, uint8_t value)
{
    uint8_t readback;
    return writeRegister(reg, value) && readRegister(reg, &readback) &&
        (readback == value);
}

static bool writeRegistersAndVerify(
    uint8_t reg, const uint8_t *data, uint8_t length)
{
    uint8_t status;
    uint8_t readback[5];

    if ((length == 0U) || (length > sizeof(readback))) return false;
    return writeRegisters(reg, data, length, &status) &&
        readRegisters(reg, readback, length, &status) &&
        (memcmp(data, readback, length) == 0);
}

static bool profileIsValid(void)
{
    uint8_t index;
    bool nonzeroAddress = false;

    if ((RADIO_ALLOW_TX == 0U) || !gProfile.valid ||
        (gProfile.channel > 125U) || (gProfile.addressWidth < 3U) ||
        (gProfile.addressWidth > 5U) ||
        (gProfile.rate > RADIO_RATE_2_MBPS)) return false;
    if (!gProfile.dynamicPayload &&
        ((gProfile.staticPayloadWidth == 0U) ||
         (gProfile.staticPayloadWidth > RADIO_MAX_PAYLOAD))) return false;
    for (index = 0U; index < gProfile.addressWidth; index++) {
        nonzeroAddress |= gProfile.address[index] != 0U;
    }
    return nonzeroAddress;
}

static uint8_t rfSetupValue(void)
{
    switch (gProfile.rate) {
        case RADIO_RATE_250_KBPS:
            return NRF_RF_DR_LOW | NRF_RF_PWR_0_DBM;
        case RADIO_RATE_2_MBPS:
            return NRF_RF_DR_HIGH | NRF_RF_PWR_0_DBM;
        case RADIO_RATE_1_MBPS:
        default:
            return NRF_RF_PWR_0_DBM;
    }
}

static bool configureProfile(void)
{
    uint8_t status;
    uint8_t setupAddress = (uint8_t) (gProfile.addressWidth - 2U);
    uint8_t setupRetransmit = (uint8_t) (
        ((gProfile.retryDelay & 0x0FU) << 4U) |
        (gProfile.retryCount & 0x0FU));
    uint8_t config = (uint8_t) (NRF_CONFIG_EN_CRC | NRF_CONFIG_CRCO);
    uint8_t feature = gProfile.dynamicPayload ? NRF_FEATURE_EN_DPL : 0U;
    uint8_t dynpd = gProfile.dynamicPayload ? NRF_DYNPD_P0 : 0U;
    uint8_t payloadWidth = gProfile.dynamicPayload ?
        0U : gProfile.staticPayloadWidth;

    forceSafe();
    if (!writeRegister(NRF_REG_CONFIG, config) ||
        !writeRegister(NRF_REG_STATUS, NRF_STATUS_IRQ_MASK) ||
        !command(NRF_CMD_FLUSH_TX, &status) ||
        !command(NRF_CMD_FLUSH_RX, &status) ||
        !writeAndVerify(NRF_REG_SETUP_AW, setupAddress) ||
        !writeRegistersAndVerify(NRF_REG_TX_ADDR,
            gProfile.address, gProfile.addressWidth) ||
        !writeRegistersAndVerify(NRF_REG_RX_ADDR_P0,
            gProfile.address, gProfile.addressWidth) ||
        !writeAndVerify(NRF_REG_EN_AA, gProfile.autoAck ? 1U : 0U) ||
        !writeAndVerify(NRF_REG_EN_RXADDR, gProfile.autoAck ? 1U : 0U) ||
        !writeAndVerify(NRF_REG_SETUP_RETR, setupRetransmit) ||
        !writeAndVerify(NRF_REG_RF_CH, gProfile.channel) ||
        !writeAndVerify(NRF_REG_RF_SETUP, rfSetupValue()) ||
        !writeAndVerify(NRF_REG_RX_PW_P0, payloadWidth) ||
        !writeAndVerify(NRF_REG_FEATURE, feature) ||
        !writeAndVerify(NRF_REG_DYNPD, dynpd) ||
        !writeAndVerify(NRF_REG_CONFIG,
            (uint8_t) (config | NRF_CONFIG_PWR_UP))) {
        gStats.initErrors++;
        forceSafe();
        return false;
    }
    return true;
}

static void queueReset(void)
{
    gQueueHead = 0U;
    gQueueTail = 0U;
    gQueueCount = 0U;
}

static void enterFault(void)
{
    gArmed = false;
    queueReset();
    forceSafe();
    gState = RADIO_STATE_FAULT;
}

static void popFrame(void)
{
    if (gQueueCount != 0U) {
        gQueueHead = (uint8_t) ((gQueueHead + 1U) %
            RADIO_TX_QUEUE_SLOTS);
        gQueueCount--;
    }
}

static void dropCurrentMessage(void)
{
    uint16_t messageId;

    if (gQueueCount == 0U) return;
    messageId = gQueue[gQueueHead].messageId;
    do {
        gStats.droppedFrames++;
        popFrame();
    } while ((gQueueCount != 0U) &&
             (gQueue[gQueueHead].messageId == messageId));
}

static bool queueLine(const uint8_t *data, uint16_t length)
{
    const uint8_t userLimit = 31U;
    uint8_t required;
    uint8_t available;
    uint16_t offset = 0U;
    uint16_t messageId;

    if (!gArmed || (length == 0U)) return false;
    required = (uint8_t) ((length + userLimit - 1U) / userLimit);
    available = (uint8_t) (RADIO_TX_QUEUE_SLOTS - gQueueCount);
    if ((required == 0U) || (required > available)) {
        gStats.droppedLines++;
        return false;
    }

    messageId = gNextMessageId++;
    while (offset < length) {
        RadioFrame *frame = &gQueue[gQueueTail];
        uint8_t copyLength = (uint8_t) (length - offset);
        if (copyLength > userLimit) copyLength = userLimit;
        memset(frame->data, 0, sizeof(frame->data));
        frame->data[0] = copyLength;
        memcpy(&frame->data[1], &data[offset], copyLength);
        frame->length = RADIO_MAX_PAYLOAD;
        frame->messageId = messageId;
        offset += copyLength;
        gQueueTail = (uint8_t) ((gQueueTail + 1U) %
            RADIO_TX_QUEUE_SLOTS);
        gQueueCount++;
    }
    return true;
}

static bool startFrame(const RadioFrame *frame)
{
    uint8_t status;

    if (!writeRegister(NRF_REG_STATUS, NRF_STATUS_IRQ_MASK) ||
        !command(NRF_CMD_FLUSH_TX, &status) ||
        !transaction(NRF_CMD_W_TX_PAYLOAD, frame->data, NULL,
            frame->length, &status)) return false;
    DL_GPIO_setPins(DIAG_GPIO_RADIO_CE_PORT, DIAG_GPIO_RADIO_CE_PIN);
    delay_cycles(RADIO_CE_PULSE_CYCLES);
    ceLow();
    gDeadline = Timebase_nowMs() + RADIO_TX_TIMEOUT_MS;
    gState = RADIO_STATE_WAIT_RESULT;
    return true;
}

static const char *stateName(void)
{
    switch (gState) {
        case RADIO_STATE_PROFILE_INVALID: return "PROFILE_INVALID";
        case RADIO_STATE_POR_WAIT: return "POR_WAIT";
        case RADIO_STATE_POWERUP_WAIT: return "POWERUP_WAIT";
        case RADIO_STATE_IDLE: return "IDLE";
        case RADIO_STATE_WAIT_RESULT: return "WAIT_RESULT";
        case RADIO_STATE_FAULT: return "FAULT";
        case RADIO_STATE_DISABLED:
        default: return "DISABLED";
    }
}

void Nrf24Ptx_init(void)
{
    memset(&gStats, 0, sizeof(gStats));
    gState = RADIO_STATE_DISABLED;
    gNextMessageId = 1U;
    gArmed = false;
    gDiagLineLength = 0U;
    gDiagLineOverflow = false;
    gOneShotQueued = false;
    queueReset();
    forceSafe();
    if (RADIO_AUTO_ARM != 0U) {
        (void) Nrf24Ptx_arm();
    }
}

bool Nrf24Ptx_arm(void)
{
    if (!profileIsValid()) {
        gState = RADIO_STATE_PROFILE_INVALID;
        forceSafe();
        return false;
    }
    queueReset();
    gArmed = true;
    gState = RADIO_STATE_POR_WAIT;
    gDeadline = Timebase_nowMs() + RADIO_POR_DELAY_MS;
    forceSafe();
    return true;
}

void Nrf24Ptx_disarm(void)
{
    gArmed = false;
    gState = RADIO_STATE_DISABLED;
    queueReset();
    forceSafe();
}

void Nrf24Ptx_captureDiagnosticChar(char value)
{
    if (gDiagLineOverflow) {
        if (value == '\n') {
            gDiagLineOverflow = false;
            gDiagLineLength = 0U;
            gStats.droppedLines++;
        }
        return;
    }
    if (gDiagLineLength >= DIAG_LINE_MIRROR_SIZE) {
        gDiagLineOverflow = true;
        if (value == '\n') {
            gDiagLineOverflow = false;
            gDiagLineLength = 0U;
            gStats.droppedLines++;
        }
        return;
    }
    gDiagLine[gDiagLineLength++] = value;
    if (value == '\n') {
        (void) queueLine((const uint8_t *) gDiagLine, gDiagLineLength);
        gDiagLineLength = 0U;
    }
}

void Nrf24Ptx_service(void)
{
    uint8_t status;
    uint32_t nowMs = Timebase_nowMs();

    switch (gState) {
        case RADIO_STATE_POR_WAIT:
            if (Timebase_reached(nowMs, gDeadline)) {
                if (configureProfile()) {
                    gState = RADIO_STATE_POWERUP_WAIT;
                    gDeadline = nowMs + RADIO_POWERUP_DELAY_MS;
                } else {
                    enterFault();
                }
            }
            break;
        case RADIO_STATE_POWERUP_WAIT:
            if (Timebase_reached(nowMs, gDeadline)) {
                gState = RADIO_STATE_IDLE;
            }
            break;
        case RADIO_STATE_IDLE:
            if ((RADIO_ONE_SHOT_TEST != 0U) && !gOneShotQueued) {
                static const uint8_t message[] =
                    "@RFTEST NF02PA LINK OK\r\n";
                gOneShotQueued = queueLine(message,
                    (uint16_t) (sizeof(message) - 1U));
            }
            if (gQueueCount != 0U) {
                if (!startFrame(&gQueue[gQueueHead])) {
                    enterFault();
                }
            }
            break;
        case RADIO_STATE_WAIT_RESULT:
            if (!command(NRF_CMD_NOP, &status)) {
                enterFault();
            } else if ((status & NRF_STATUS_TX_DS) != 0U) {
                (void) writeRegister(NRF_REG_STATUS, NRF_STATUS_TX_DS);
                gStats.txSuccess++;
                popFrame();
                if ((RADIO_ONE_SHOT_TEST != 0U) && gOneShotQueued &&
                    (gQueueCount == 0U)) {
                    Nrf24Ptx_disarm();
                } else {
                    gState = RADIO_STATE_IDLE;
                }
            } else if ((status & NRF_STATUS_MAX_RT) != 0U) {
                (void) writeRegister(NRF_REG_STATUS, NRF_STATUS_MAX_RT);
                (void) command(NRF_CMD_FLUSH_TX, &status);
                gStats.maxRetry++;
                dropCurrentMessage();
                if ((RADIO_ONE_SHOT_TEST != 0U) && gOneShotQueued &&
                    (gQueueCount == 0U)) {
                    Nrf24Ptx_disarm();
                } else {
                    gState = RADIO_STATE_IDLE;
                }
            } else if (Timebase_reached(nowMs, gDeadline)) {
                (void) command(NRF_CMD_FLUSH_TX, &status);
                gStats.txTimeout++;
                enterFault();
            }
            break;
        case RADIO_STATE_DISABLED:
        case RADIO_STATE_PROFILE_INVALID:
        case RADIO_STATE_FAULT:
        default:
            forceSafe();
            break;
    }
}

Nrf24PtxStatus Nrf24Ptx_getStatus(void)
{
    Nrf24PtxStatus status;
    status.stateName = stateName();
    status.armed = gArmed;
    status.queued = gQueueCount;
    status.txSuccess = gStats.txSuccess;
    status.maxRetry = gStats.maxRetry;
    status.txTimeout = gStats.txTimeout;
    status.spiErrors = gStats.spiErrors;
    status.initErrors = gStats.initErrors;
    status.droppedLines = gStats.droppedLines;
    return status;
}

Nrf24PtxRegisters Nrf24Ptx_readCoreRegisters(void)
{
    Nrf24PtxRegisters registers = {false, 0U, 0U, 0U};
    registers.ok = readRegister(NRF_REG_CONFIG, &registers.config) &&
        readRegister(NRF_REG_RF_CH, &registers.channel) &&
        readRegister(NRF_REG_RF_SETUP, &registers.rfSetup);
    return registers;
}

bool Nrf24Ptx_safeWhenDisarmed(void)
{
    if (gArmed || (gQueueCount != 0U)) return false;
    return DL_GPIO_readPins(DIAG_GPIO_RADIO_CE_PORT,
               DIAG_GPIO_RADIO_CE_PIN) == 0U;
}

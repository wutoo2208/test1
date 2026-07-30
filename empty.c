/*
 * Copyright (c) 2021, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 * * Neither the name of Texas Instruments Incorporated nor the names of
 *   its contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "ti_msp_dl_config.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define FW_VERSION                  "diag-radio-0.2"
#define PIN_PLAN_VERSION            "1.2"
#define RX_BUFFER_SIZE              (128U)
#define COMMAND_BUFFER_SIZE         (80U)
#define DIAG_LINE_SIZE              (192U)
#define I2C_TIMEOUT_LOOPS           (200000U)
#define RADIO_SPI_TIMEOUT_LOOPS     (200000U)
#define RADIO_TX_QUEUE_SLOTS        (16U)
#define RADIO_MAX_PAYLOAD           (32U)
#define RADIO_TX_TIMEOUT_MS         (100U)
#define RADIO_POR_DELAY_MS          (100U)
#define RADIO_POWERUP_DELAY_MS      (2U)
#define RADIO_CE_PULSE_CYCLES       (CPUCLK_FREQ / 66667U)

/*
 * RF remains fail-closed until the actual COM7 adapter profile is recovered.
 * Set all three gates only after channel/address/rate/CRC/payload evidence.
 */
#define RADIO_ALLOW_TX              (0U)
#define RADIO_PROFILE_VALID         (0U)
#define RADIO_AUTO_ARM              (0U)

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
#define NRF_REG_FIFO_STATUS         (0x17U)
#define NRF_REG_DYNPD               (0x1CU)
#define NRF_REG_FEATURE             (0x1DU)

#define NRF_CONFIG_EN_CRC           (1U << 3U)
#define NRF_CONFIG_CRCO             (1U << 2U)
#define NRF_CONFIG_PWR_UP           (1U << 1U)
#define NRF_STATUS_RX_DR            (1U << 6U)
#define NRF_STATUS_TX_DS            (1U << 5U)
#define NRF_STATUS_MAX_RT           (1U << 4U)
#define NRF_STATUS_IRQ_MASK         (NRF_STATUS_RX_DR | NRF_STATUS_TX_DS | NRF_STATUS_MAX_RT)
#define NRF_FEATURE_EN_DPL          (1U << 2U)
#define NRF_DYNPD_P0                (1U << 0U)
#define NRF_FIFO_TX_EMPTY           (1U << 4U)
#define NRF_RF_DR_LOW               (1U << 5U)
#define NRF_RF_DR_HIGH              (1U << 3U)

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

/* Placeholder values are intentionally invalid until the receiver is known. */
static const RadioProfile gRadioProfile = {
    .valid = (RADIO_PROFILE_VALID != 0U),
    .dynamicPayload = false,
    .autoAck = false,
    .channel = 0U,
    .rate = RADIO_RATE_1_MBPS,
    .addressWidth = 0U,
    .address = {0U, 0U, 0U, 0U, 0U},
    .staticPayloadWidth = 0U,
    .retryDelay = 0U,
    .retryCount = 0U
};

static volatile uint8_t gRxBuffer[RX_BUFFER_SIZE];
static volatile uint16_t gRxHead;
static volatile uint16_t gRxTail;
static volatile uint32_t gRxOverflow;
static volatile int32_t gLeftEncoderCount;
static volatile uint32_t gLeftInvalidTransitions;
static volatile uint8_t gLeftState;
static volatile uint32_t gMilliseconds;
static char gCommand[COMMAND_BUFFER_SIZE];
static uint8_t gCommandLength;
static bool gDiscardLine;

static RadioFrame gRadioQueue[RADIO_TX_QUEUE_SLOTS];
static RadioStats gRadioStats;
static RadioState gRadioState = RADIO_STATE_DISABLED;
static uint8_t gRadioQueueHead;
static uint8_t gRadioQueueTail;
static uint8_t gRadioQueueCount;
static uint16_t gRadioNextMessageId = 1U;
static uint32_t gRadioDeadline;
static bool gRadioArmed;
static char gDiagLine[DIAG_LINE_SIZE];
static uint16_t gDiagLineLength;
static bool gDiagLineOverflow;
static bool gCaptureEnabled;

static void radioCaptureCharacter(char value);
static void radioService(void);

static bool timeReached(uint32_t now, uint32_t deadline)
{
    return ((int32_t) (now - deadline)) >= 0;
}

static void uartPutChar(char value)
{
    DL_UART_transmitDataBlocking(DIAG_UART_INST, (uint8_t) value);
    if (gCaptureEnabled) {
        radioCaptureCharacter(value);
    }
}

static void uartWrite(const char *text)
{
    while (*text != '\0') {
        uartPutChar(*text++);
    }
}

static void uartWriteU32(uint32_t value)
{
    char digits[10];
    uint8_t count = 0U;

    if (value == 0U) {
        uartPutChar('0');
        return;
    }
    while ((value != 0U) && (count < sizeof(digits))) {
        digits[count++] = (char) ('0' + (value % 10U));
        value /= 10U;
    }
    while (count != 0U) {
        uartPutChar(digits[--count]);
    }
}

static void uartWriteI32(int32_t value)
{
    uint32_t magnitude;

    if (value < 0) {
        uartPutChar('-');
        magnitude = (uint32_t) (-(value + 1));
        magnitude++;
    } else {
        magnitude = (uint32_t) value;
    }
    uartWriteU32(magnitude);
}

static void uartWriteHex8(uint8_t value)
{
    static const char hex[] = "0123456789ABCDEF";
    uartPutChar(hex[(value >> 4U) & 0x0FU]);
    uartPutChar(hex[value & 0x0FU]);
}

static void uartNewLine(void)
{
    uartWrite("\r\n");
}

static void forceActuatorSafeOutputs(void)
{
    DL_GPIO_clearPins(DIAG_GPIO_MOTOR_AIN1_SAFE_PORT,
        DIAG_GPIO_MOTOR_AIN1_SAFE_PIN);
    DL_GPIO_clearPins(DIAG_GPIO_MOTOR_AIN2_SAFE_PORT,
        DIAG_GPIO_MOTOR_AIN2_SAFE_PIN);
    DL_GPIO_clearPins(DIAG_GPIO_MOTOR_BIN1_SAFE_PORT,
        DIAG_GPIO_MOTOR_BIN1_SAFE_PIN);
    DL_GPIO_clearPins(DIAG_GPIO_MOTOR_BIN2_SAFE_PORT,
        DIAG_GPIO_MOTOR_BIN2_SAFE_PIN);
    DL_GPIO_clearPins(DIAG_GPIO_BUZZER_PORT, DIAG_GPIO_BUZZER_PIN);
    DL_GPIO_clearPins(DIAG_GPIO_D36A_EN_SAFE_PORT,
        DIAG_GPIO_D36A_EN_SAFE_PIN);
    DL_GPIO_clearPins(DIAG_GPIO_D36A_DIR_SAFE_PORT,
        DIAG_GPIO_D36A_DIR_SAFE_PIN);
    DL_GPIO_clearPins(DIAG_GPIO_D36A_STEP_SAFE_PORT,
        DIAG_GPIO_D36A_STEP_SAFE_PIN);
}

static void radioCeLow(void)
{
    DL_GPIO_clearPins(DIAG_GPIO_RADIO_CE_PORT, DIAG_GPIO_RADIO_CE_PIN);
}

static void radioCsnHigh(void)
{
    DL_GPIO_setPins(DIAG_GPIO_RADIO_CSN_PORT, DIAG_GPIO_RADIO_CSN_PIN);
}

static void radioForceSafe(void)
{
    radioCeLow();
    radioCsnHigh();
}

static bool radioSpiByte(uint8_t output, uint8_t *input)
{
    uint32_t timeout = RADIO_SPI_TIMEOUT_LOOPS;

    while (!DL_SPI_transmitDataCheck8(RADIO_SPI_INST, output)) {
        if (timeout-- == 0U) {
            return false;
        }
    }
    timeout = RADIO_SPI_TIMEOUT_LOOPS;
    while (!DL_SPI_receiveDataCheck8(RADIO_SPI_INST, input)) {
        if (timeout-- == 0U) {
            return false;
        }
    }
    return true;
}

static bool radioTransaction(uint8_t command, const uint8_t *txData,
    uint8_t *rxData, uint8_t length, uint8_t *status)
{
    uint8_t discard[RADIO_MAX_PAYLOAD + 1U];
    uint8_t input;
    uint8_t index;
    uint32_t timeout;
    bool ok = true;

    if (length > RADIO_MAX_PAYLOAD) {
        return false;
    }
    radioCeLow();
    radioCsnHigh();
    (void) DL_SPI_drainRXFIFO8(RADIO_SPI_INST, discard, sizeof(discard));
    DL_GPIO_clearPins(DIAG_GPIO_RADIO_CSN_PORT, DIAG_GPIO_RADIO_CSN_PIN);

    if (!radioSpiByte(command, &input)) {
        ok = false;
    } else {
        *status = input;
    }
    for (index = 0U; ok && (index < length); index++) {
        uint8_t output = (txData != NULL) ? txData[index] : 0xFFU;
        if (!radioSpiByte(output, &input)) {
            ok = false;
        } else if (rxData != NULL) {
            rxData[index] = input;
        }
    }

    timeout = RADIO_SPI_TIMEOUT_LOOPS;
    while (DL_SPI_isBusy(RADIO_SPI_INST) && (timeout-- != 0U)) {
    }
    if (timeout == 0U) {
        ok = false;
    }
    radioCsnHigh();
    if (!ok) {
        gRadioStats.spiErrors++;
    }
    return ok;
}

static bool radioCommand(uint8_t command, uint8_t *status)
{
    return radioTransaction(command, NULL, NULL, 0U, status);
}

static bool radioReadRegisters(
    uint8_t reg, uint8_t *data, uint8_t length, uint8_t *status)
{
    return radioTransaction((uint8_t) (NRF_CMD_R_REGISTER |
        (reg & NRF_REGISTER_MASK)), NULL, data, length, status);
}

static bool radioWriteRegisters(
    uint8_t reg, const uint8_t *data, uint8_t length, uint8_t *status)
{
    return radioTransaction((uint8_t) (NRF_CMD_W_REGISTER |
        (reg & NRF_REGISTER_MASK)), data, NULL, length, status);
}

static bool radioReadRegister(uint8_t reg, uint8_t *value)
{
    uint8_t status;
    return radioReadRegisters(reg, value, 1U, &status);
}

static bool radioWriteRegister(uint8_t reg, uint8_t value)
{
    uint8_t status;
    return radioWriteRegisters(reg, &value, 1U, &status);
}

static bool radioWriteAndVerify(uint8_t reg, uint8_t value)
{
    uint8_t readback;
    return radioWriteRegister(reg, value) &&
        radioReadRegister(reg, &readback) && (readback == value);
}

static bool radioWriteRegistersAndVerify(
    uint8_t reg, const uint8_t *data, uint8_t length)
{
    uint8_t status;
    uint8_t readback[5];

    if ((length == 0U) || (length > sizeof(readback))) {
        return false;
    }
    return radioWriteRegisters(reg, data, length, &status) &&
        radioReadRegisters(reg, readback, length, &status) &&
        (memcmp(data, readback, length) == 0);
}

static bool radioProfileIsValid(void)
{
    uint8_t i;
    bool nonzeroAddress = false;

    if ((RADIO_ALLOW_TX == 0U) || !gRadioProfile.valid ||
        (gRadioProfile.channel > 125U) ||
        (gRadioProfile.addressWidth < 3U) ||
        (gRadioProfile.addressWidth > 5U) ||
        (gRadioProfile.rate > RADIO_RATE_2_MBPS)) {
        return false;
    }
    if (!gRadioProfile.dynamicPayload &&
        ((gRadioProfile.staticPayloadWidth == 0U) ||
         (gRadioProfile.staticPayloadWidth > RADIO_MAX_PAYLOAD))) {
        return false;
    }
    for (i = 0U; i < gRadioProfile.addressWidth; i++) {
        nonzeroAddress |= gRadioProfile.address[i] != 0U;
    }
    return nonzeroAddress;
}

static uint8_t radioRfSetupValue(void)
{
    switch (gRadioProfile.rate) {
        case RADIO_RATE_250_KBPS:
            return NRF_RF_DR_LOW;
        case RADIO_RATE_2_MBPS:
            return NRF_RF_DR_HIGH;
        case RADIO_RATE_1_MBPS:
        default:
            return 0U;
    }
}

static bool radioConfigureProfile(void)
{
    uint8_t status;
    uint8_t setupAddress = (uint8_t) (gRadioProfile.addressWidth - 2U);
    uint8_t setupRetransmit = (uint8_t) (
        ((gRadioProfile.retryDelay & 0x0FU) << 4U) |
        (gRadioProfile.retryCount & 0x0FU));
    uint8_t config = (uint8_t) (NRF_CONFIG_EN_CRC | NRF_CONFIG_CRCO);
    uint8_t feature = gRadioProfile.dynamicPayload ? NRF_FEATURE_EN_DPL : 0U;
    uint8_t dynpd = gRadioProfile.dynamicPayload ? NRF_DYNPD_P0 : 0U;
    uint8_t payloadWidth = gRadioProfile.dynamicPayload ?
        0U : gRadioProfile.staticPayloadWidth;

    radioForceSafe();
    if (!radioWriteRegister(NRF_REG_CONFIG, config) ||
        !radioWriteRegister(NRF_REG_STATUS, NRF_STATUS_IRQ_MASK) ||
        !radioCommand(NRF_CMD_FLUSH_TX, &status) ||
        !radioCommand(NRF_CMD_FLUSH_RX, &status) ||
        !radioWriteAndVerify(NRF_REG_SETUP_AW, setupAddress) ||
        !radioWriteRegistersAndVerify(NRF_REG_TX_ADDR,
            gRadioProfile.address, gRadioProfile.addressWidth) ||
        !radioWriteRegistersAndVerify(NRF_REG_RX_ADDR_P0,
            gRadioProfile.address, gRadioProfile.addressWidth) ||
        !radioWriteAndVerify(NRF_REG_EN_AA,
            gRadioProfile.autoAck ? 1U : 0U) ||
        !radioWriteAndVerify(NRF_REG_EN_RXADDR,
            gRadioProfile.autoAck ? 1U : 0U) ||
        !radioWriteAndVerify(NRF_REG_SETUP_RETR, setupRetransmit) ||
        !radioWriteAndVerify(NRF_REG_RF_CH, gRadioProfile.channel) ||
        !radioWriteAndVerify(NRF_REG_RF_SETUP, radioRfSetupValue()) ||
        !radioWriteAndVerify(NRF_REG_RX_PW_P0, payloadWidth) ||
        !radioWriteAndVerify(NRF_REG_FEATURE, feature) ||
        !radioWriteAndVerify(NRF_REG_DYNPD, dynpd) ||
        !radioWriteAndVerify(NRF_REG_CONFIG,
            (uint8_t) (config | NRF_CONFIG_PWR_UP))) {
        gRadioStats.initErrors++;
        radioForceSafe();
        return false;
    }
    return true;
}

static uint8_t radioPayloadLimit(void)
{
    return gRadioProfile.dynamicPayload ?
        RADIO_MAX_PAYLOAD : gRadioProfile.staticPayloadWidth;
}

static void radioQueueReset(void)
{
    gRadioQueueHead = 0U;
    gRadioQueueTail = 0U;
    gRadioQueueCount = 0U;
}

static void radioPopFrame(void)
{
    if (gRadioQueueCount != 0U) {
        gRadioQueueHead = (uint8_t) ((gRadioQueueHead + 1U) %
            RADIO_TX_QUEUE_SLOTS);
        gRadioQueueCount--;
    }
}

static void radioDropCurrentMessage(void)
{
    uint16_t messageId;

    if (gRadioQueueCount == 0U) {
        return;
    }
    messageId = gRadioQueue[gRadioQueueHead].messageId;
    do {
        gRadioStats.droppedFrames++;
        radioPopFrame();
    } while ((gRadioQueueCount != 0U) &&
             (gRadioQueue[gRadioQueueHead].messageId == messageId));
}

static bool radioQueueLine(const uint8_t *data, uint16_t length)
{
    uint8_t limit;
    uint8_t required;
    uint8_t available;
    uint16_t offset = 0U;
    uint16_t messageId;

    if (!gRadioArmed || (length == 0U)) {
        return false;
    }
    limit = radioPayloadLimit();
    if (limit == 0U) {
        return false;
    }
    required = (uint8_t) ((length + limit - 1U) / limit);
    available = (uint8_t) (RADIO_TX_QUEUE_SLOTS - gRadioQueueCount);
    if ((required == 0U) || (required > available)) {
        gRadioStats.droppedLines++;
        return false;
    }

    messageId = gRadioNextMessageId++;
    while (offset < length) {
        RadioFrame *frame = &gRadioQueue[gRadioQueueTail];
        uint8_t copyLength = (uint8_t) (length - offset);
        if (copyLength > limit) {
            copyLength = limit;
        }
        memset(frame->data, 0, sizeof(frame->data));
        memcpy(frame->data, &data[offset], copyLength);
        frame->length = gRadioProfile.dynamicPayload ? copyLength : limit;
        frame->messageId = messageId;
        offset += copyLength;
        gRadioQueueTail = (uint8_t) ((gRadioQueueTail + 1U) %
            RADIO_TX_QUEUE_SLOTS);
        gRadioQueueCount++;
    }
    return true;
}

static void radioCaptureCharacter(char value)
{
    if (gDiagLineOverflow) {
        if (value == '\n') {
            gDiagLineOverflow = false;
            gDiagLineLength = 0U;
            gRadioStats.droppedLines++;
        }
        return;
    }
    if (gDiagLineLength >= DIAG_LINE_SIZE) {
        gDiagLineOverflow = true;
        if (value == '\n') {
            gDiagLineOverflow = false;
            gDiagLineLength = 0U;
            gRadioStats.droppedLines++;
        }
        return;
    }
    gDiagLine[gDiagLineLength++] = value;
    if (value == '\n') {
        (void) radioQueueLine((const uint8_t *) gDiagLine, gDiagLineLength);
        gDiagLineLength = 0U;
    }
}

static void radioDisarm(void)
{
    gRadioArmed = false;
    gRadioState = RADIO_STATE_DISABLED;
    radioQueueReset();
    radioForceSafe();
}

static bool radioArm(void)
{
    if (!radioProfileIsValid()) {
        gRadioState = RADIO_STATE_PROFILE_INVALID;
        radioForceSafe();
        return false;
    }
    radioQueueReset();
    gRadioArmed = true;
    gRadioState = RADIO_STATE_POR_WAIT;
    gRadioDeadline = gMilliseconds + RADIO_POR_DELAY_MS;
    radioForceSafe();
    return true;
}

static bool radioStartFrame(const RadioFrame *frame)
{
    uint8_t status;

    if (!radioWriteRegister(NRF_REG_STATUS, NRF_STATUS_IRQ_MASK) ||
        !radioCommand(NRF_CMD_FLUSH_TX, &status) ||
        !radioTransaction(NRF_CMD_W_TX_PAYLOAD, frame->data, NULL,
            frame->length, &status)) {
        return false;
    }
    DL_GPIO_setPins(DIAG_GPIO_RADIO_CE_PORT, DIAG_GPIO_RADIO_CE_PIN);
    delay_cycles(RADIO_CE_PULSE_CYCLES);
    radioCeLow();
    gRadioDeadline = gMilliseconds + RADIO_TX_TIMEOUT_MS;
    gRadioState = RADIO_STATE_WAIT_RESULT;
    return true;
}

static void radioService(void)
{
    uint8_t status;

    switch (gRadioState) {
        case RADIO_STATE_POR_WAIT:
            if (timeReached(gMilliseconds, gRadioDeadline)) {
                if (radioConfigureProfile()) {
                    gRadioState = RADIO_STATE_POWERUP_WAIT;
                    gRadioDeadline = gMilliseconds + RADIO_POWERUP_DELAY_MS;
                } else {
                    gRadioState = RADIO_STATE_FAULT;
                }
            }
            break;
        case RADIO_STATE_POWERUP_WAIT:
            if (timeReached(gMilliseconds, gRadioDeadline)) {
                gRadioState = RADIO_STATE_IDLE;
            }
            break;
        case RADIO_STATE_IDLE:
            if (gRadioQueueCount != 0U) {
                if (!radioStartFrame(&gRadioQueue[gRadioQueueHead])) {
                    gRadioState = RADIO_STATE_FAULT;
                    radioForceSafe();
                }
            }
            break;
        case RADIO_STATE_WAIT_RESULT:
            if (!radioCommand(NRF_CMD_NOP, &status)) {
                gRadioState = RADIO_STATE_FAULT;
                radioForceSafe();
            } else if ((status & NRF_STATUS_TX_DS) != 0U) {
                (void) radioWriteRegister(NRF_REG_STATUS, NRF_STATUS_TX_DS);
                gRadioStats.txSuccess++;
                radioPopFrame();
                gRadioState = RADIO_STATE_IDLE;
            } else if ((status & NRF_STATUS_MAX_RT) != 0U) {
                (void) radioWriteRegister(NRF_REG_STATUS, NRF_STATUS_MAX_RT);
                (void) radioCommand(NRF_CMD_FLUSH_TX, &status);
                gRadioStats.maxRetry++;
                radioDropCurrentMessage();
                gRadioState = RADIO_STATE_IDLE;
            } else if (timeReached(gMilliseconds, gRadioDeadline)) {
                (void) radioCommand(NRF_CMD_FLUSH_TX, &status);
                gRadioStats.txTimeout++;
                gRadioState = RADIO_STATE_FAULT;
                radioForceSafe();
            }
            break;
        case RADIO_STATE_DISABLED:
        case RADIO_STATE_PROFILE_INVALID:
        case RADIO_STATE_FAULT:
        default:
            radioForceSafe();
            break;
    }
}

static const char *radioStateName(void)
{
    switch (gRadioState) {
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

static uint8_t readLineBits(void)
{
    uint8_t bits = 0U;
    if (DL_GPIO_readPins(DIAG_GPIO_TCRT_OUT1_PORT,
            DIAG_GPIO_TCRT_OUT1_PIN) != 0U) bits |= (1U << 0U);
    if (DL_GPIO_readPins(DIAG_GPIO_TCRT_OUT2_PORT,
            DIAG_GPIO_TCRT_OUT2_PIN) != 0U) bits |= (1U << 1U);
    if (DL_GPIO_readPins(DIAG_GPIO_TCRT_OUT3_PORT,
            DIAG_GPIO_TCRT_OUT3_PIN) != 0U) bits |= (1U << 2U);
    if (DL_GPIO_readPins(DIAG_GPIO_TCRT_OUT4_PORT,
            DIAG_GPIO_TCRT_OUT4_PIN) != 0U) bits |= (1U << 3U);
    if (DL_GPIO_readPins(DIAG_GPIO_TCRT_OUT5_PORT,
            DIAG_GPIO_TCRT_OUT5_PIN) != 0U) bits |= (1U << 4U);
    return bits;
}

static uint8_t readLeftEncoderState(void)
{
    uint8_t state = 0U;
    if (DL_GPIO_readPins(GPIO_LEFT_CAPTURE_C0_PORT,
            GPIO_LEFT_CAPTURE_C0_PIN) != 0U) state |= 2U;
    if (DL_GPIO_readPins(GPIO_LEFT_CAPTURE_C1_PORT,
            GPIO_LEFT_CAPTURE_C1_PIN) != 0U) state |= 1U;
    return state;
}

static uint8_t readRightEncoderState(void)
{
    uint8_t state = 0U;
    if (DL_GPIO_readPins(GPIO_RIGHT_QEI_PHA_PORT,
            GPIO_RIGHT_QEI_PHA_PIN) != 0U) state |= 2U;
    if (DL_GPIO_readPins(GPIO_RIGHT_QEI_PHB_PORT,
            GPIO_RIGHT_QEI_PHB_PIN) != 0U) state |= 1U;
    return state;
}

static void updateLeftQuadrature(void)
{
    static const int8_t transitions[16] = {
         0,  1, -1,  0, -1,  0,  0,  1,
         1,  0,  0, -1,  0, -1,  1,  0
    };
    uint8_t next = readLeftEncoderState();
    uint8_t index = (uint8_t) ((gLeftState << 2U) | next);
    int8_t step = transitions[index];
    if ((step == 0) && (next != gLeftState) && ((next ^ gLeftState) == 3U)) {
        gLeftInvalidTransitions++;
    }
    gLeftEncoderCount += step;
    gLeftState = next;
}

static void reportLine(void)
{
    uint8_t bits = readLineBits();
    uint8_t i;
    uartWrite("@LINE raw=");
    for (i = 0U; i < 5U; i++) {
        uartPutChar(((bits & (1U << i)) != 0U) ? '1' : '0');
    }
    uartWrite(" polarity=UNKNOWN order=OUT1..OUT5");
    uartNewLine();
}

static void reportEncoders(void)
{
    uint8_t right = readRightEncoderState();
    uint8_t left = readLeftEncoderState();
    uartWrite("@ENC right_ab=");
    uartPutChar(((right & 2U) != 0U) ? '1' : '0');
    uartPutChar(((right & 1U) != 0U) ? '1' : '0');
    uartWrite(" right_count=");
    uartWriteU32(DL_TimerG_getTimerCount(RIGHT_QEI_INST));
    uartWrite(" right_dir=");
    uartWrite((DL_TimerG_getQEIDirection(RIGHT_QEI_INST) ==
                  DL_TIMER_QEI_DIR_DOWN) ? "DOWN" : "UP");
    uartWrite(" left_ab=");
    uartPutChar(((left & 2U) != 0U) ? '1' : '0');
    uartPutChar(((left & 1U) != 0U) ? '1' : '0');
    uartWrite(" left_count=");
    uartWriteI32(gLeftEncoderCount);
    uartWrite(" left_invalid=");
    uartWriteU32(gLeftInvalidTransitions);
    uartNewLine();
}

static bool i2cWaitComplete(I2C_Regs *instance)
{
    uint32_t timeout = I2C_TIMEOUT_LOOPS;
    while (timeout-- != 0U) {
        uint32_t status = DL_I2C_getControllerStatus(instance);
        if ((status & DL_I2C_CONTROLLER_STATUS_BUSY) == 0U) {
            return (status & DL_I2C_CONTROLLER_STATUS_ERROR) == 0U;
        }
    }
    return false;
}

static void i2cPrepare(I2C_Regs *instance)
{
    DL_I2C_resetControllerTransfer(instance);
    DL_I2C_flushControllerTXFIFO(instance);
    DL_I2C_flushControllerRXFIFO(instance);
}

static bool i2cProbe(I2C_Regs *instance, uint8_t address)
{
    i2cPrepare(instance);
    DL_I2C_startControllerTransfer(instance, address,
        DL_I2C_CONTROLLER_DIRECTION_TX, 0U);
    delay_cycles(12U);
    return i2cWaitComplete(instance);
}

static bool i2cReadRegister(
    I2C_Regs *instance, uint8_t address, uint8_t reg, uint8_t *value)
{
    i2cPrepare(instance);
    DL_I2C_fillControllerTXFIFO(instance, &reg, 1U);
    DL_I2C_startControllerTransfer(instance, address,
        DL_I2C_CONTROLLER_DIRECTION_TX, 1U);
    delay_cycles(12U);
    if (!i2cWaitComplete(instance)) return false;
    i2cPrepare(instance);
    DL_I2C_startControllerTransfer(instance, address,
        DL_I2C_CONTROLLER_DIRECTION_RX, 1U);
    delay_cycles(12U);
    if (!i2cWaitComplete(instance) ||
        DL_I2C_isControllerRXFIFOEmpty(instance)) return false;
    *value = DL_I2C_receiveControllerData(instance);
    return true;
}

static void reportI2C(void)
{
    bool oled3c = i2cProbe(OLED_I2C_INST, 0x3CU);
    bool oled3d = i2cProbe(OLED_I2C_INST, 0x3DU);
    bool mpu68 = i2cProbe(MPU_I2C_INST, 0x68U);
    bool mpu69 = i2cProbe(MPU_I2C_INST, 0x69U);
    uartWrite("@I2C oled_3c="); uartWrite(oled3c ? "ACK" : "NACK");
    uartWrite(" oled_3d="); uartWrite(oled3d ? "ACK" : "NACK");
    uartWrite(" mpu_68="); uartWrite(mpu68 ? "ACK" : "NACK");
    uartWrite(" mpu_69="); uartWrite(mpu69 ? "ACK" : "NACK");
    uartNewLine();
}

static void reportMpu(void)
{
    uint8_t address;
    uint8_t whoAmI = 0U;
    bool found = false;
    for (address = 0x68U; address <= 0x69U; address++) {
        if (i2cReadRegister(MPU_I2C_INST, address, 0x75U, &whoAmI)) {
            found = true;
            break;
        }
    }
    if (!found) {
        uartWrite("@MPU status=NO_RESPONSE addresses=68,69");
    } else {
        uartWrite("@MPU address=0x"); uartWriteHex8(address);
        uartWrite(" who_am_i=0x"); uartWriteHex8(whoAmI);
        uartWrite(" configured=NO");
    }
    uartNewLine();
}

static void reportRadioRegisters(void)
{
    uint8_t config;
    uint8_t channel;
    uint8_t setup;
    if (!radioReadRegister(NRF_REG_CONFIG, &config) ||
        !radioReadRegister(NRF_REG_RF_CH, &channel) ||
        !radioReadRegister(NRF_REG_RF_SETUP, &setup)) {
        uartWrite("@RADIO_REGS status=TIMEOUT_OR_SHORT_FRAME ce=0");
    } else {
        uartWrite("@RADIO_REGS config=0x"); uartWriteHex8(config);
        uartWrite(" rf_ch=0x"); uartWriteHex8(channel);
        uartWrite(" rf_setup=0x"); uartWriteHex8(setup);
        uartWrite(" ce=0");
    }
    uartNewLine();
}

static void reportRadioStatus(void)
{
    uartWrite("@RADIO state="); uartWrite(radioStateName());
    uartWrite(" allow_tx="); uartWriteU32(RADIO_ALLOW_TX);
    uartWrite(" profile_valid="); uartWriteU32(RADIO_PROFILE_VALID);
    uartWrite(" armed="); uartWriteU32(gRadioArmed ? 1U : 0U);
    uartWrite(" queued="); uartWriteU32(gRadioQueueCount);
    uartWrite(" tx_ok="); uartWriteU32(gRadioStats.txSuccess);
    uartWrite(" max_rt="); uartWriteU32(gRadioStats.maxRetry);
    uartWrite(" timeout="); uartWriteU32(gRadioStats.txTimeout);
    uartWrite(" spi_err="); uartWriteU32(gRadioStats.spiErrors);
    uartWrite(" init_err="); uartWriteU32(gRadioStats.initErrors);
    uartWrite(" drop_lines="); uartWriteU32(gRadioStats.droppedLines);
    uartNewLine();
}

static void reportStatus(void)
{
    uartWrite("@STATUS fw=" FW_VERSION " pinmap=" PIN_PLAN_VERSION);
    uartWrite(" safe=SOFTWARE_LOCKED_RESET_BIAS_UNVERIFIED motor=00/00");
    uartWrite(" d36a=DISABLED radio="); uartWrite(radioStateName());
    uartWrite(" rx_overflow="); uartWriteU32(gRxOverflow);
    uartNewLine();
}

static void reportHelp(void)
{
    uartWrite("@HELP commands=help,status,pins,line,enc,i2c,mpu,radio_regs,radio_status,radio_arm,radio_disarm,radio_test,stop,selftest");
    uartNewLine();
}

static void runSelfTest(void)
{
    bool safe = true;
    forceActuatorSafeOutputs();
    if (!gRadioArmed) radioForceSafe();
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
    safe &= (DL_GPIO_readPins(DIAG_GPIO_D36A_STEP_SAFE_PORT,
                 DIAG_GPIO_D36A_STEP_SAFE_PIN) == 0U);
    if (!gRadioArmed) {
        safe &= (DL_GPIO_readPins(DIAG_GPIO_RADIO_CE_PORT,
                     DIAG_GPIO_RADIO_CE_PIN) == 0U);
    }
    uartWrite("@SELFTEST safe_outputs="); uartWrite(safe ? "PASS" : "FAIL");
    uartWrite(" reset_bias=UNVERIFIED result=");
    uartWrite(safe ? "PASS_SOFTWARE_ONLY" : "FAIL");
    uartNewLine();
}

static void processCommand(const char *command)
{
    if (strcmp(command, "help") == 0) {
        reportHelp();
    } else if (strcmp(command, "status") == 0) {
        reportStatus();
    } else if ((strcmp(command, "pins") == 0) ||
               (strcmp(command, "enc") == 0) ||
               (strcmp(command, "enc raw") == 0)) {
        reportLine(); reportEncoders();
    } else if ((strcmp(command, "line") == 0) ||
               (strcmp(command, "line raw") == 0)) {
        reportLine();
    } else if ((strcmp(command, "i2c") == 0) ||
               (strcmp(command, "i2c scan") == 0)) {
        reportI2C();
    } else if ((strcmp(command, "mpu") == 0) ||
               (strcmp(command, "mpu whoami") == 0)) {
        reportMpu();
    } else if ((strcmp(command, "radio") == 0) ||
               (strcmp(command, "radio regs") == 0) ||
               (strcmp(command, "radio_regs") == 0)) {
        reportRadioRegisters();
    } else if ((strcmp(command, "radio status") == 0) ||
               (strcmp(command, "radio_status") == 0)) {
        reportRadioStatus();
    } else if ((strcmp(command, "radio arm") == 0) ||
               (strcmp(command, "radio_arm") == 0)) {
        if (radioArm()) uartWrite("@OK cmd=radio_arm state=POR_WAIT\r\n");
        else uartWrite("@BLOCKED cmd=radio_arm reason=PROFILE_UNKNOWN_OR_TX_GATE\r\n");
    } else if ((strcmp(command, "radio disarm") == 0) ||
               (strcmp(command, "radio_disarm") == 0)) {
        radioDisarm(); uartWrite("@OK cmd=radio_disarm state=DISABLED\r\n");
    } else if ((strcmp(command, "radio test") == 0) ||
               (strcmp(command, "radio_test") == 0)) {
        uartWrite("@RFTEST fw=" FW_VERSION " ms="); uartWriteU32(gMilliseconds);
        uartNewLine();
    } else if ((strcmp(command, "stop") == 0) ||
               (strcmp(command, "motor stop") == 0)) {
        forceActuatorSafeOutputs(); radioDisarm();
        uartWrite("@OK cmd=stop safe=LOCKED motor=00/00 radio=DISABLED\r\n");
    } else if (strcmp(command, "selftest") == 0) {
        runSelfTest();
    } else if (command[0] != '\0') {
        uartWrite("@ERR code=BAD_CMD\r\n");
    }
}

static bool rxPop(uint8_t *value)
{
    bool available = false;
    __disable_irq();
    if (gRxTail != gRxHead) {
        *value = gRxBuffer[gRxTail];
        gRxTail = (uint16_t) ((gRxTail + 1U) % RX_BUFFER_SIZE);
        available = true;
    }
    __enable_irq();
    return available;
}

static void pollConsole(void)
{
    uint8_t value;
    while (rxPop(&value)) {
        if ((value == '\r') || (value == '\n')) {
            if (gDiscardLine) {
                gDiscardLine = false; gCommandLength = 0U;
                uartWrite("@ERR code=LINE_TOO_LONG\r\n");
            } else if (gCommandLength != 0U) {
                gCommand[gCommandLength] = '\0';
                processCommand(gCommand);
                gCommandLength = 0U;
            }
        } else if (!gDiscardLine) {
            if (gCommandLength < (COMMAND_BUFFER_SIZE - 1U)) {
                gCommand[gCommandLength++] = (char) value;
            } else {
                gDiscardLine = true;
            }
        }
    }
}

int main(void)
{
    SYSCFG_DL_init();
    forceActuatorSafeOutputs();
    radioForceSafe();
    DL_SYSTICK_init(CPUCLK_FREQ / 1000U);
    DL_SYSTICK_enableInterrupt();
    DL_SYSTICK_enable();

    gLeftState = readLeftEncoderState();
    DL_TimerG_setTimerCount(RIGHT_QEI_INST, 0U);
    DL_TimerG_startCounter(RIGHT_QEI_INST);
    DL_TimerA_startCounter(LEFT_CAPTURE_INST);
    NVIC_ClearPendingIRQ(DIAG_UART_INST_INT_IRQN);
    NVIC_EnableIRQ(DIAG_UART_INST_INT_IRQN);
    NVIC_ClearPendingIRQ(LEFT_CAPTURE_INST_INT_IRQN);
    NVIC_EnableIRQ(LEFT_CAPTURE_INST_INT_IRQN);

    if (RADIO_AUTO_ARM != 0U) {
        (void) radioArm();
    }
    gCaptureEnabled = true;
    delay_cycles(CPUCLK_FREQ / 100U);
    uartWrite("@BOOT proto=2 fw=" FW_VERSION " pinmap=" PIN_PLAN_VERSION
              " safe=SOFTWARE_LOCKED_RESET_BIAS_UNVERIFIED baud=115200");
    uartNewLine();
    reportStatus();
    reportRadioStatus();

    while (1) {
        forceActuatorSafeOutputs();
        pollConsole();
        radioService();
    }
}

void SysTick_Handler(void)
{
    gMilliseconds++;
}

void DIAG_UART_INST_IRQHandler(void)
{
    if (DL_UART_Main_getPendingInterrupt(DIAG_UART_INST) ==
        DL_UART_MAIN_IIDX_RX) {
        while (!DL_UART_Main_isRXFIFOEmpty(DIAG_UART_INST)) {
            uint8_t value = DL_UART_Main_receiveData(DIAG_UART_INST);
            uint16_t next = (uint16_t) ((gRxHead + 1U) % RX_BUFFER_SIZE);
            if (next == gRxTail) gRxOverflow++;
            else { gRxBuffer[gRxHead] = value; gRxHead = next; }
        }
    }
}

void LEFT_CAPTURE_INST_IRQHandler(void)
{
    DL_TIMER_IIDX pending;
    while ((pending = DL_TimerA_getPendingInterrupt(LEFT_CAPTURE_INST)) != 0) {
        switch (pending) {
            case DL_TIMER_IIDX_CC0_UP:
            case DL_TIMER_IIDX_CC1_UP:
                updateLeftQuadrature();
                break;
            case DL_TIMER_IIDX_LOAD:
            default:
                break;
        }
    }
}

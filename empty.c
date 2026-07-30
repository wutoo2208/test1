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

#define FW_VERSION             "diag-0.1"
#define PIN_PLAN_VERSION       "1.2"
#define RX_BUFFER_SIZE         (128U)
#define COMMAND_BUFFER_SIZE    (80U)
#define I2C_TIMEOUT_LOOPS      (200000U)

static volatile uint8_t gRxBuffer[RX_BUFFER_SIZE];
static volatile uint16_t gRxHead;
static volatile uint16_t gRxTail;
static volatile uint32_t gRxOverflow;
static volatile int32_t gLeftEncoderCount;
static volatile uint32_t gLeftInvalidTransitions;
static volatile uint8_t gLeftState;
static char gCommand[COMMAND_BUFFER_SIZE];
static uint8_t gCommandLength;
static bool gDiscardLine;

static void uartPutChar(char value)
{
    DL_UART_transmitDataBlocking(DIAG_UART_INST, (uint8_t) value);
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

static void forceSafeOutputs(void)
{
    DL_GPIO_clearPins(DIAG_GPIO_MOTOR_AIN1_SAFE_PORT,
        DIAG_GPIO_MOTOR_AIN1_SAFE_PIN);
    DL_GPIO_clearPins(DIAG_GPIO_MOTOR_AIN2_SAFE_PORT,
        DIAG_GPIO_MOTOR_AIN2_SAFE_PIN);
    DL_GPIO_clearPins(DIAG_GPIO_MOTOR_BIN1_SAFE_PORT,
        DIAG_GPIO_MOTOR_BIN1_SAFE_PIN);
    DL_GPIO_clearPins(DIAG_GPIO_MOTOR_BIN2_SAFE_PORT,
        DIAG_GPIO_MOTOR_BIN2_SAFE_PIN);
    DL_GPIO_clearPins(DIAG_GPIO_RADIO_CE_PORT, DIAG_GPIO_RADIO_CE_PIN);
    DL_GPIO_clearPins(DIAG_GPIO_BUZZER_PORT, DIAG_GPIO_BUZZER_PIN);
    DL_GPIO_clearPins(DIAG_GPIO_D36A_EN_SAFE_PORT,
        DIAG_GPIO_D36A_EN_SAFE_PIN);
    DL_GPIO_clearPins(DIAG_GPIO_D36A_DIR_SAFE_PORT,
        DIAG_GPIO_D36A_DIR_SAFE_PIN);
    DL_GPIO_clearPins(DIAG_GPIO_D36A_STEP_SAFE_PORT,
        DIAG_GPIO_D36A_STEP_SAFE_PIN);
}

static uint8_t readLineBits(void)
{
    uint8_t bits = 0U;

    if (DL_GPIO_readPins(DIAG_GPIO_TCRT_OUT1_PORT,
            DIAG_GPIO_TCRT_OUT1_PIN) != 0U) {
        bits |= (1U << 0U);
    }
    if (DL_GPIO_readPins(DIAG_GPIO_TCRT_OUT2_PORT,
            DIAG_GPIO_TCRT_OUT2_PIN) != 0U) {
        bits |= (1U << 1U);
    }
    if (DL_GPIO_readPins(DIAG_GPIO_TCRT_OUT3_PORT,
            DIAG_GPIO_TCRT_OUT3_PIN) != 0U) {
        bits |= (1U << 2U);
    }
    if (DL_GPIO_readPins(DIAG_GPIO_TCRT_OUT4_PORT,
            DIAG_GPIO_TCRT_OUT4_PIN) != 0U) {
        bits |= (1U << 3U);
    }
    if (DL_GPIO_readPins(DIAG_GPIO_TCRT_OUT5_PORT,
            DIAG_GPIO_TCRT_OUT5_PIN) != 0U) {
        bits |= (1U << 4U);
    }
    return bits;
}

static uint8_t readLeftEncoderState(void)
{
    uint8_t state = 0U;

    if (DL_GPIO_readPins(GPIO_LEFT_CAPTURE_C0_PORT,
            GPIO_LEFT_CAPTURE_C0_PIN) != 0U) {
        state |= 2U;
    }
    if (DL_GPIO_readPins(GPIO_LEFT_CAPTURE_C1_PORT,
            GPIO_LEFT_CAPTURE_C1_PIN) != 0U) {
        state |= 1U;
    }
    return state;
}

static uint8_t readRightEncoderState(void)
{
    uint8_t state = 0U;

    if (DL_GPIO_readPins(GPIO_RIGHT_QEI_PHA_PORT,
            GPIO_RIGHT_QEI_PHA_PIN) != 0U) {
        state |= 2U;
    }
    if (DL_GPIO_readPins(GPIO_RIGHT_QEI_PHB_PORT,
            GPIO_RIGHT_QEI_PHB_PIN) != 0U) {
        state |= 1U;
    }
    return state;
}

static void updateLeftQuadrature(void)
{
    static const int8_t transitions[16] = {
         0,  1, -1,  0,
        -1,  0,  0,  1,
         1,  0,  0, -1,
         0, -1,  1,  0
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
    if (!i2cWaitComplete(instance)) {
        return false;
    }

    i2cPrepare(instance);
    DL_I2C_startControllerTransfer(instance, address,
        DL_I2C_CONTROLLER_DIRECTION_RX, 1U);
    delay_cycles(12U);
    if (!i2cWaitComplete(instance) ||
        DL_I2C_isControllerRXFIFOEmpty(instance)) {
        return false;
    }
    *value = DL_I2C_receiveControllerData(instance);
    return true;
}

static void reportI2C(void)
{
    bool oled3c = i2cProbe(OLED_I2C_INST, 0x3CU);
    bool oled3d = i2cProbe(OLED_I2C_INST, 0x3DU);
    bool mpu68 = i2cProbe(MPU_I2C_INST, 0x68U);
    bool mpu69 = i2cProbe(MPU_I2C_INST, 0x69U);

    uartWrite("@I2C oled_3c=");
    uartWrite(oled3c ? "ACK" : "NACK");
    uartWrite(" oled_3d=");
    uartWrite(oled3d ? "ACK" : "NACK");
    uartWrite(" mpu_68=");
    uartWrite(mpu68 ? "ACK" : "NACK");
    uartWrite(" mpu_69=");
    uartWrite(mpu69 ? "ACK" : "NACK");
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
        uartWrite("@MPU address=0x");
        uartWriteHex8(address);
        uartWrite(" who_am_i=0x");
        uartWriteHex8(whoAmI);
        uartWrite(" configured=NO");
    }
    uartNewLine();
}

static bool radioReadRegister(
    uint8_t reg, uint8_t *status, uint8_t *value)
{
    uint8_t tx[2] = {(uint8_t) (reg & 0x1FU), 0xFFU};
    uint8_t rx[4];
    uint32_t timeout = I2C_TIMEOUT_LOOPS;
    uint32_t count;

    (void) DL_SPI_drainRXFIFO8(RADIO_SPI_INST, rx, sizeof(rx));
    if (DL_SPI_fillTXFIFO8(RADIO_SPI_INST, tx, sizeof(tx)) != sizeof(tx)) {
        return false;
    }
    while (DL_SPI_isBusy(RADIO_SPI_INST) && (timeout-- != 0U)) {
    }
    if (timeout == 0U) {
        return false;
    }
    count = DL_SPI_drainRXFIFO8(RADIO_SPI_INST, rx, sizeof(rx));
    if (count < 2U) {
        return false;
    }
    *status = rx[0];
    *value = rx[1];
    return true;
}

static void reportRadio(void)
{
    uint8_t statusConfig;
    uint8_t statusChannel;
    uint8_t config;
    uint8_t channel;

    forceSafeOutputs();
    if (!radioReadRegister(0x00U, &statusConfig, &config) ||
        !radioReadRegister(0x05U, &statusChannel, &channel)) {
        uartWrite("@RADIO status=TIMEOUT_OR_SHORT_FRAME ce=0 txrx=DISABLED");
        uartNewLine();
        return;
    }

    uartWrite("@RADIO status_config=0x");
    uartWriteHex8(statusConfig);
    uartWrite(" status_channel=0x");
    uartWriteHex8(statusChannel);
    uartWrite(" config=0x");
    uartWriteHex8(config);
    uartWrite(" rf_ch=0x");
    uartWriteHex8(channel);
    uartWrite(" ce=0 txrx=DISABLED");
    uartNewLine();
}

static void reportStatus(void)
{
    uartWrite("@STATUS fw=" FW_VERSION " pinmap=" PIN_PLAN_VERSION);
    uartWrite(" safe=SOFTWARE_LOCKED_RESET_BIAS_UNVERIFIED motor=00/00");
    uartWrite(" d36a=DISABLED radio_ce=0");
    uartWrite(" rx_overflow=");
    uartWriteU32(gRxOverflow);
    uartNewLine();
}

static void reportHelp(void)
{
    uartWrite("@HELP commands=help,status,pins,line,enc,i2c,mpu,radio,stop,selftest");
    uartNewLine();
}

static void runSelfTest(void)
{
    bool safe = true;

    forceSafeOutputs();
    safe &= (DL_GPIO_readPins(DIAG_GPIO_MOTOR_AIN1_SAFE_PORT,
                 DIAG_GPIO_MOTOR_AIN1_SAFE_PIN) == 0U);
    safe &= (DL_GPIO_readPins(DIAG_GPIO_MOTOR_AIN2_SAFE_PORT,
                 DIAG_GPIO_MOTOR_AIN2_SAFE_PIN) == 0U);
    safe &= (DL_GPIO_readPins(DIAG_GPIO_MOTOR_BIN1_SAFE_PORT,
                 DIAG_GPIO_MOTOR_BIN1_SAFE_PIN) == 0U);
    safe &= (DL_GPIO_readPins(DIAG_GPIO_MOTOR_BIN2_SAFE_PORT,
                 DIAG_GPIO_MOTOR_BIN2_SAFE_PIN) == 0U);
    safe &= (DL_GPIO_readPins(DIAG_GPIO_RADIO_CE_PORT,
                 DIAG_GPIO_RADIO_CE_PIN) == 0U);
    safe &= (DL_GPIO_readPins(DIAG_GPIO_D36A_EN_SAFE_PORT,
                 DIAG_GPIO_D36A_EN_SAFE_PIN) == 0U);
    safe &= (DL_GPIO_readPins(DIAG_GPIO_D36A_STEP_SAFE_PORT,
                 DIAG_GPIO_D36A_STEP_SAFE_PIN) == 0U);

    uartWrite("@SELFTEST safe_outputs=");
    uartWrite(safe ? "PASS" : "FAIL");
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
        reportLine();
        reportEncoders();
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
               (strcmp(command, "radio regs") == 0)) {
        reportRadio();
    } else if ((strcmp(command, "stop") == 0) ||
               (strcmp(command, "motor stop") == 0)) {
        forceSafeOutputs();
        uartWrite("@OK cmd=stop safe=LOCKED motor=00/00");
        uartNewLine();
    } else if (strcmp(command, "selftest") == 0) {
        runSelfTest();
    } else if (command[0] != '\0') {
        uartWrite("@ERR code=BAD_CMD");
        uartNewLine();
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
                gDiscardLine = false;
                gCommandLength = 0U;
                uartWrite("@ERR code=LINE_TOO_LONG");
                uartNewLine();
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
    forceSafeOutputs();

    gLeftState = readLeftEncoderState();
    DL_TimerG_setTimerCount(RIGHT_QEI_INST, 0U);
    DL_TimerG_startCounter(RIGHT_QEI_INST);
    DL_TimerA_startCounter(LEFT_CAPTURE_INST);

    NVIC_ClearPendingIRQ(DIAG_UART_INST_INT_IRQN);
    NVIC_EnableIRQ(DIAG_UART_INST_INT_IRQN);
    NVIC_ClearPendingIRQ(LEFT_CAPTURE_INST_INT_IRQN);
    NVIC_EnableIRQ(LEFT_CAPTURE_INST_INT_IRQN);

    delay_cycles(CPUCLK_FREQ / 100U);
    uartWrite("@BOOT proto=1 fw=" FW_VERSION " pinmap=" PIN_PLAN_VERSION
              " safe=SOFTWARE_LOCKED_RESET_BIAS_UNVERIFIED baud=115200");
    uartNewLine();
    reportStatus();

    while (1) {
        forceSafeOutputs();
        pollConsole();
    }
}

void DIAG_UART_INST_IRQHandler(void)
{
    if (DL_UART_Main_getPendingInterrupt(DIAG_UART_INST) ==
        DL_UART_MAIN_IIDX_RX) {
        while (!DL_UART_Main_isRXFIFOEmpty(DIAG_UART_INST)) {
            uint8_t value = DL_UART_Main_receiveData(DIAG_UART_INST);
            uint16_t next = (uint16_t) ((gRxHead + 1U) % RX_BUFFER_SIZE);

            if (next == gRxTail) {
                gRxOverflow++;
            } else {
                gRxBuffer[gRxHead] = value;
                gRxHead = next;
            }
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

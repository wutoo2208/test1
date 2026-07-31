#include "drivers/diag_console.h"

#include <stdbool.h>
#include <string.h>

#include "app/req002.h"
#include "app/motor_test.h"
#include "algorithm/line_tracking.h"
#include "bsp/board_safety.h"
#include "bsp/timebase.h"
#include "config/firmware_config.h"
#include "drivers/encoders.h"
#include "drivers/i2c_diag.h"
#include "drivers/line_sensors.h"
#include "drivers/nrf24_ptx.h"
#include "drivers/oled_ssd1306.h"
#include "drivers/start_button.h"
#include "ti_msp_dl_config.h"

#if MOTOR_SELFTEST_BUILD
#define MOTOR_TEST_HELP_COMMANDS ",motor_status,motor_test_left,motor_test_right"
#else
#define MOTOR_TEST_HELP_COMMANDS ""
#endif

static volatile uint8_t gRxBuffer[DIAG_UART_RX_BUFFER_SIZE];
static volatile uint16_t gRxHead;
static volatile uint16_t gRxTail;
static volatile uint32_t gRxOverflow;
static char gCommand[DIAG_COMMAND_BUFFER_SIZE];
static uint8_t gCommandLength;
static bool gDiscardLine;
static bool gCaptureEnabled;

static void putChar(char value)
{
    DL_UART_transmitDataBlocking(DIAG_UART_INST, (uint8_t) value);
    if (gCaptureEnabled) {
        Nrf24Ptx_captureDiagnosticChar(value);
    }
}

static void writeText(const char *text)
{
    while (*text != '\0') putChar(*text++);
}

static void writeU32(uint32_t value)
{
    char digits[10];
    uint8_t count = 0U;

    if (value == 0U) {
        putChar('0');
        return;
    }
    while ((value != 0U) && (count < sizeof(digits))) {
        digits[count++] = (char) ('0' + (value % 10U));
        value /= 10U;
    }
    while (count != 0U) putChar(digits[--count]);
}

static void writeI32(int32_t value)
{
    uint32_t magnitude;

    if (value < 0) {
        putChar('-');
        magnitude = (uint32_t) (-(value + 1));
        magnitude++;
    } else {
        magnitude = (uint32_t) value;
    }
    writeU32(magnitude);
}

static void writeMilliFloat(float value)
{
    int32_t scaled = (int32_t) (value * 1000.0f);

    writeI32(scaled);
}

static void writeHex8(uint8_t value)
{
    static const char hex[] = "0123456789ABCDEF";
    putChar(hex[(value >> 4U) & 0x0FU]);
    putChar(hex[value & 0x0FU]);
}

static void newLine(void)
{
    writeText("\r\n");
}

static void reportLine(void)
{
    LineSensorSample sample = LineSensors_snapshot();
    uint8_t index;

    writeText("@LINE valid="); writeU32(sample.valid ? 1U : 0U);
    writeText(" digital=");
    for (index = 0U; index < LINE_SENSOR_COUNT; index++) {
        putChar(((sample.digitalBits & (1U << index)) != 0U) ? '1' : '0');
    }
    writeText(" analog=");
    for (index = 0U; index < LINE_SENSOR_COUNT; index++) {
        if (index != 0U) putChar(',');
        writeU32(sample.analog[index]);
    }
    writeText(" sequence="); writeU32(sample.sequence);
    writeText(" last_success_ms="); writeU32(sample.lastSuccessMs);
    writeText(" errors="); writeU32(sample.errorCount);
    writeText(" fail_stage="); writeU32(sample.lastFailureStage);
    writeText(" fail_reg=0x"); writeHex8(sample.lastRegister);
    writeText(" received="); writeU32(sample.lastReceivedBytes);
    writeText(" i2c_status="); writeU32(sample.lastControllerStatus);
    writeText(" black_digital=1 analog_black_higher=1 order=CH1..CH6_LEFT_TO_RIGHT");
    newLine();
}

static void reportLineTracking(void)
{
    const LineTrackingStatus *status = LineTracking_getStatus();
    uint8_t index;

    writeText("@TRACK valid="); writeU32(status->valid ? 1U : 0U);
    writeText(" lost="); writeU32(status->lineLost ? 1U : 0U);
    writeText(" norm_milli=");
    for (index = 0U; index < LINE_SENSOR_COUNT; index++) {
        if (index != 0U) putChar(',');
        writeMilliFloat(status->normalized[index]);
    }
    writeText(" position_milli="); writeMilliFloat(status->position);
    writeText(" error_milli="); writeMilliFloat(status->centeredError);
    writeText(" confidence_milli="); writeMilliFloat(status->confidence);
    writeText(" shadow_milli="); writeMilliFloat(status->shadowCorrection);
    writeText(" actuator_lock=1");
    newLine();
}

static void reportEncoders(void)
{
    EncoderSnapshot snapshot = Encoders_snapshot();

    writeText("@ENC right_ab=");
    putChar(((snapshot.rightState & 2U) != 0U) ? '1' : '0');
    putChar(((snapshot.rightState & 1U) != 0U) ? '1' : '0');
    writeText(" right_count="); writeU32(snapshot.rightCount);
    writeText(" right_dir="); writeText(snapshot.rightDown ? "DOWN" : "UP");
    writeText(" left_ab=");
    putChar(((snapshot.leftState & 2U) != 0U) ? '1' : '0');
    putChar(((snapshot.leftState & 1U) != 0U) ? '1' : '0');
    writeText(" left_count="); writeI32(snapshot.leftCount);
    writeText(" left_invalid="); writeU32(snapshot.leftInvalidTransitions);
    newLine();
}

#if MOTOR_SELFTEST_BUILD
static void reportMotorTest(void)
{
    MotorTestStatus status = MotorTest_snapshot();

    writeText("@MOTOR_TEST state="); writeText(MotorTest_stateName(status.state));
    writeText(" wheel=");
    writeText(status.wheel == MOTOR_WHEEL_LEFT ? "LEFT" : "RIGHT");
    writeText(" active="); writeU32(status.outputsActive ? 1U : 0U);
    writeText(" duty_permille="); writeU32(status.dutyPermille);
    writeText(" started_ms="); writeU32(status.startedMs);
    writeText(" deadline_ms="); writeU32(status.deadlineMs);
    newLine();
}
#endif

static void reportI2c(void)
{
    I2cDiagScan scan = I2cDiag_scan();

    writeText("@I2C line_5c="); writeText(scan.line5c ? "ACK" : "NACK");
    writeText(" oled_3c="); writeText(scan.oled3c ? "ACK" : "NACK");
    writeText(" oled_3d="); writeText(scan.oled3d ? "ACK" : "NACK");
    newLine();
}

static void runOledTest(void)
{
    bool ok = OledSsd1306_showTestPattern();
    writeText("@OLED address=0x3C controller=SSD1306_CANDIDATE test=");
    writeText(ok ? "I2C_PASS_CHECK_SCREEN" : "I2C_FAIL");
    newLine();
}

static void forceOledAllOn(void)
{
    bool ok = OledSsd1306_forceAllPixelsOn();
    writeText("@OLED address=0x3C command=A5_ALL_ON test=");
    writeText(ok ? "I2C_PASS_CHECK_SCREEN" : "I2C_FAIL");
    newLine();
}

static void forceOledAllOnSh1106(void)
{
    bool ok = OledSsd1306_forceAllPixelsOnSh1106();
    writeText("@OLED address=0x3C controller=SH1106_CANDIDATE command=AD8B_AF_A5 test=");
    writeText(ok ? "I2C_PASS_CHECK_SCREEN" : "I2C_FAIL");
    newLine();
}

static void clearOled(void)
{
    OledSsd1306_clear();
    writeText("@OLED clear=REQUESTED display=OFF");
    newLine();
}

static void reportRadioRegisters(void)
{
    Nrf24PtxRegisters registers = Nrf24Ptx_readCoreRegisters();

    if (!registers.ok) {
        writeText("@RADIO_REGS status=TIMEOUT_OR_SHORT_FRAME ce=0");
    } else {
        writeText("@RADIO_REGS config=0x"); writeHex8(registers.config);
        writeText(" rf_ch=0x"); writeHex8(registers.channel);
        writeText(" rf_setup=0x"); writeHex8(registers.rfSetup);
        writeText(" ce=0");
    }
    newLine();
}

static void reportRadioStatus(void)
{
    Nrf24PtxStatus status = Nrf24Ptx_getStatus();
    bool capture = gCaptureEnabled;

    gCaptureEnabled = false;
    writeText("@RADIO state="); writeText(status.stateName);
    writeText(" protocol=BAOQIAN_V2_STATIC32_LEN0");
    writeText(" allow_tx="); writeU32(RADIO_ALLOW_TX);
    writeText(" profile_valid="); writeU32(RADIO_PROFILE_VALID);
    writeText(" armed="); writeU32(status.armed ? 1U : 0U);
    writeText(" rf_ch=0 rate=2Mbps crc=16 aw=5 ack=P0 retr=0x1A");
    writeText(" payload=32 user_max=31 addr=FFFFFFFFFF");
    writeText(" queued="); writeU32(status.queued);
    writeText(" tx_ok="); writeU32(status.txSuccess);
    writeText(" max_rt="); writeU32(status.maxRetry);
    writeText(" timeout="); writeU32(status.txTimeout);
    writeText(" spi_err="); writeU32(status.spiErrors);
    writeText(" init_err="); writeU32(status.initErrors);
    writeText(" drop_lines="); writeU32(status.droppedLines);
    newLine();
    gCaptureEnabled = capture;
}

static void reportButton(void)
{
    writeText("@BUTTON pin=PA21 key=KEY2 active_low=1 raw_pressed=");
    writeU32(StartButton_readHardwarePressed() ? 1U : 0U);
    newLine();
}

static void reportReq002(void)
{
    const Req002Status *status = Req002_getStatus();

    writeText("@REQ002 state="); writeText(Req002_stateName(status->state));
    writeText(" blocked=");
    writeText(Req002_blockReasonName(status->blockReason));
    writeText(" actuator_lock="); writeU32(status->actuatorLocked ? 1U : 0U);
    writeText(" calibration_gate="); writeU32(REQ002_CALIBRATION_VALID);
    writeText(" actuation_gate="); writeU32(REQ002_ACTUATION_GATE_VALID);
    writeText(" physical_params=");
    writeU32(REQ002_PHYSICAL_PARAMETERS_VALID);
    writeText(" adapter_enabled=");
    writeU32(REQ002_ACTUATOR_ADAPTER_ENABLED);
    writeText(" pid_enabled="); writeU32(REQ002_PID_ENABLED);
    writeText(" elapsed_ms="); writeU32(status->elapsedMs);
    writeText(" frozen_ms="); writeU32(status->frozenElapsedMs);
    writeText(" timeout_ms="); writeU32(status->timeoutMs);
    writeText(" timer_running="); writeU32(status->timerRunning ? 1U : 0U);
    writeText(" marker_start="); writeU32(status->startMarkerSeen ? 1U : 0U);
    writeText(" departed="); writeU32(status->departedStartMarker ? 1U : 0U);
    writeText(" marker_return="); writeU32(status->returnMarkerSeen ? 1U : 0U);
    writeText(" attempts="); writeU32(status->buttonAttempts);
    newLine();
}

static void reportStatus(void)
{
    Nrf24PtxStatus radio = Nrf24Ptx_getStatus();
    const Req002Status *req002 = Req002_getStatus();
    bool motorSafe = BoardSafety_outputsSafe();

    writeText("@STATUS fw=" FW_VERSION " pinmap=" PIN_PLAN_VERSION);
    writeText(motorSafe ? " safe=SOFTWARE_LOCKED_RESET_BIAS_UNVERIFIED motor=00/00" :
        " safe=MOTION_ACTIVE_SOFTWARE_ONLY motor=ACTIVE");
    writeText(" d36a=DISABLED buzzer=");
    writeText(BoardSafety_buzzerPolicy());
    writeText(" radio="); writeText(radio.stateName);
    writeText(" req002="); writeText(Req002_stateName(req002->state));
    writeText(" rx_overflow="); writeU32(gRxOverflow);
    newLine();
}

static void reportHelp(void)
{
    writeText("@HELP commands=help,status,pins,line,track,enc,button,i2c"
        MOTOR_TEST_HELP_COMMANDS
        ",oled_test,oled_all_on,oled_sh1106_on,oled_clear,radio_regs,radio_status,radio_arm,radio_disarm,radio_test,req002,req002_status,stop,selftest");
    newLine();
}

static void runSelfTest(void)
{
    bool safe;

    MotorTest_abort();
    BoardSafety_stop(BOARD_SAFETY_STOP_SELFTEST);
    Nrf24Ptx_disarm();
    safe = BoardSafety_outputsSafe() && Nrf24Ptx_safeWhenDisarmed();
    writeText("@SELFTEST safe_outputs="); writeText(safe ? "PASS" : "FAIL");
    writeText(" reset_bias=UNVERIFIED buzzer=NOT_TESTED_DNC result=");
    writeText(safe ? "PASS_SOFTWARE_ONLY" : "FAIL");
    newLine();
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
    } else if ((strcmp(command, "track") == 0) ||
               (strcmp(command, "line track") == 0) ||
               (strcmp(command, "line_track") == 0)) {
        reportLineTracking();
    } else if ((strcmp(command, "button") == 0) ||
               (strcmp(command, "button raw") == 0)) {
        reportButton();
    } else if ((strcmp(command, "i2c") == 0) ||
               (strcmp(command, "i2c scan") == 0)) {
        reportI2c();
#if MOTOR_SELFTEST_BUILD
    } else if ((strcmp(command, "motor status") == 0) ||
               (strcmp(command, "motor_status") == 0)) {
        reportMotorTest();
    } else if ((strcmp(command, "motor test left") == 0) ||
               (strcmp(command, "motor_test_left") == 0)) {
        if (MotorTest_start(MOTOR_WHEEL_LEFT, Timebase_nowMs()) ==
            MOTOR_TEST_START_OK) {
            writeText("@OK cmd=motor_test_left "); reportMotorTest();
        } else {
            writeText("@BLOCKED cmd=motor_test_left reason=DISABLED_BUSY_OR_FAULT\r\n");
        }
    } else if ((strcmp(command, "motor test right") == 0) ||
               (strcmp(command, "motor_test_right") == 0)) {
        if (MotorTest_start(MOTOR_WHEEL_RIGHT, Timebase_nowMs()) ==
            MOTOR_TEST_START_OK) {
            writeText("@OK cmd=motor_test_right "); reportMotorTest();
        } else {
            writeText("@BLOCKED cmd=motor_test_right reason=DISABLED_BUSY_OR_FAULT\r\n");
        }
#endif
    } else if ((strcmp(command, "oled test") == 0) ||
               (strcmp(command, "oled_test") == 0)) {
        runOledTest();
    } else if ((strcmp(command, "oled all on") == 0) ||
               (strcmp(command, "oled_all_on") == 0)) {
        forceOledAllOn();
    } else if ((strcmp(command, "oled sh1106 on") == 0) ||
               (strcmp(command, "oled_sh1106_on") == 0)) {
        forceOledAllOnSh1106();
    } else if ((strcmp(command, "oled clear") == 0) ||
               (strcmp(command, "oled_clear") == 0)) {
        clearOled();
    } else if ((strcmp(command, "radio") == 0) ||
               (strcmp(command, "radio regs") == 0) ||
               (strcmp(command, "radio_regs") == 0)) {
        reportRadioRegisters();
    } else if ((strcmp(command, "radio status") == 0) ||
               (strcmp(command, "radio_status") == 0)) {
        reportRadioStatus();
    } else if ((strcmp(command, "radio arm") == 0) ||
               (strcmp(command, "radio_arm") == 0)) {
        if (Nrf24Ptx_arm())
            writeText("@OK cmd=radio_arm state=POR_WAIT\r\n");
        else
            writeText("@BLOCKED cmd=radio_arm reason=PROFILE_UNKNOWN_OR_TX_GATE\r\n");
    } else if ((strcmp(command, "radio disarm") == 0) ||
               (strcmp(command, "radio_disarm") == 0)) {
        Nrf24Ptx_disarm();
        writeText("@OK cmd=radio_disarm state=DISABLED\r\n");
    } else if ((strcmp(command, "radio test") == 0) ||
               (strcmp(command, "radio_test") == 0)) {
        writeText("@RFTEST fw=" FW_VERSION " ms=");
        writeU32(Timebase_nowMs()); newLine();
    } else if ((strcmp(command, "req002") == 0) ||
               (strcmp(command, "req002 status") == 0) ||
               (strcmp(command, "req002_status") == 0)) {
        reportReq002();
    } else if ((strcmp(command, "stop") == 0) ||
               (strcmp(command, "motor stop") == 0)) {
        bool motorSafe;
        bool radioSafe;
        MotorTest_abort();
        BoardSafety_stop(BOARD_SAFETY_STOP_OPERATOR);
        Nrf24Ptx_disarm();
        motorSafe = BoardSafety_outputsSafe();
        radioSafe = Nrf24Ptx_safeWhenDisarmed();
        writeText((motorSafe && radioSafe) ? "@OK" : "@FAIL");
        writeText(" cmd=stop motor_safe="); writeU32(motorSafe ? 1U : 0U);
        writeText(" radio_safe="); writeU32(radioSafe ? 1U : 0U);
        newLine();
    } else if (strcmp(command, "selftest") == 0) {
        runSelfTest();
    } else if (command[0] != '\0') {
        writeText("@ERR code=BAD_CMD\r\n");
    }
}

static bool rxPop(uint8_t *value)
{
    bool available = false;

    __disable_irq();
    if (gRxTail != gRxHead) {
        *value = gRxBuffer[gRxTail];
        gRxTail = (uint16_t) ((gRxTail + 1U) %
            DIAG_UART_RX_BUFFER_SIZE);
        available = true;
    }
    __enable_irq();
    return available;
}

void DiagConsole_init(void)
{
    gRxHead = 0U;
    gRxTail = 0U;
    gRxOverflow = 0U;
    gCommandLength = 0U;
    gDiscardLine = false;
    gCaptureEnabled = true;
    NVIC_ClearPendingIRQ(DIAG_UART_INST_INT_IRQN);
    NVIC_EnableIRQ(DIAG_UART_INST_INT_IRQN);
}

void DiagConsole_service(void)
{
    uint8_t value;

    while (rxPop(&value)) {
        if ((value == '\r') || (value == '\n')) {
            if (gDiscardLine) {
                gDiscardLine = false;
                gCommandLength = 0U;
                writeText("@ERR code=LINE_TOO_LONG\r\n");
            } else if (gCommandLength != 0U) {
                gCommand[gCommandLength] = '\0';
                processCommand(gCommand);
                gCommandLength = 0U;
            }
        } else if (!gDiscardLine) {
            if (gCommandLength < (DIAG_COMMAND_BUFFER_SIZE - 1U)) {
                gCommand[gCommandLength++] = (char) value;
            } else {
                gDiscardLine = true;
            }
        }
    }
}

void DiagConsole_reportBoot(void)
{
    delay_cycles(CPUCLK_FREQ / 100U);
    writeText("@BOOT proto=2 fw=" FW_VERSION " pinmap=" PIN_PLAN_VERSION
              " safe=SOFTWARE_LOCKED_RESET_BIAS_UNVERIFIED baud=115200");
    newLine();
    reportStatus();
    reportRadioStatus();
}

void DiagConsole_onUartInterrupt(void)
{
    if (DL_UART_Main_getPendingInterrupt(DIAG_UART_INST) ==
        DL_UART_MAIN_IIDX_RX) {
        while (!DL_UART_Main_isRXFIFOEmpty(DIAG_UART_INST)) {
            uint8_t value = DL_UART_Main_receiveData(DIAG_UART_INST);
            uint16_t next = (uint16_t) ((gRxHead + 1U) %
                DIAG_UART_RX_BUFFER_SIZE);
            if (next == gRxTail) {
                gRxOverflow++;
            } else {
                gRxBuffer[gRxHead] = value;
                gRxHead = next;
            }
        }
    }
}

uint32_t DiagConsole_rxOverflow(void)
{
    return gRxOverflow;
}

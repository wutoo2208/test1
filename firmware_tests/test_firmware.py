#!/usr/bin/env python3
"""Pure-stdlib source checks for safety-critical firmware invariants."""

from __future__ import annotations

import pathlib
import re
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[1]


def text(relative: str) -> str:
    return (ROOT / relative).read_text(encoding="utf-8")


class FirmwareSafetyTests(unittest.TestCase):
    def test_working_copy_one_shot_profile_and_truthful_selftest(self) -> None:
        config = text("config/firmware_config.h")
        radio = text("drivers/nrf24_ptx.c")
        for gate in (
            "RADIO_ALLOW_TX",
            "RADIO_PROFILE_VALID",
            "RADIO_AUTO_ARM",
            "RADIO_ONE_SHOT_TEST",
        ):
            self.assertRegex(config, rf"#define\s+{gate}\s+\(1U\)")
        self.assertIn('"@RFTEST NF02PA LINK OK\\r\\n"', radio)
        self.assertIn("frame->data[0] = copyLength;", radio)
        self.assertIn("frame->length = RADIO_MAX_PAYLOAD;", radio)
        self.assertRegex(
            radio,
            r"RADIO_ONE_SHOT_TEST[^}]+gOneShotQueued[^}]+"
            r"gQueueCount == 0U[^}]+Nrf24Ptx_disarm\(\);",
        )
        app = text("app/app.c")
        self.assertLess(app.index("DiagConsole_reportBoot();"),
                        app.index("Nrf24Ptx_init();"))
        max_rt = radio[radio.index("NRF_STATUS_MAX_RT", radio.index("void Nrf24Ptx_service")):]
        self.assertIn("Nrf24Ptx_disarm();", max_rt)
        self.assertIn("static void enterFault(void)", radio)
        fault_body = radio[radio.index("static void enterFault(void)"):
                           radio.index("static void popFrame(void)")]
        self.assertIn("gArmed = false;", fault_body)
        self.assertIn("queueReset();", fault_body)
        self.assertIn("forceSafe();", fault_body)

        safety_body = radio[radio.index("bool Nrf24Ptx_safeWhenDisarmed(void)"):]
        self.assertRegex(safety_body, r"gArmed\s*\|\|\s*\(gQueueCount != 0U\)")
        self.assertIn("return false;", safety_body)
        self.assertIn("DIAG_GPIO_RADIO_CE_PIN", safety_body)

    def test_motor_output_ownership_and_test_isolation(self) -> None:
        config = text("config/firmware_config.h")
        syscfg = text("empty.syscfg")
        safety = text("bsp/board_safety.c") + text("bsp/board_safety.h")
        driver = text("drivers/motor_driver.c")
        motor_test = text("app/motor_test.c") + text("app/motor_test.h")
        console = text("drivers/diag_console.c")

        self.assertRegex(config, r"#define\s+MOTOR_SELFTEST_BUILD\s+\(0U\)")
        for token in (
            'PWM1.$name                         = "MOTOR_PWM"',
            "PWM1.timerCount                    = 1600",
            'PWM1.peripheral.$assign            = "TIMA0"',
            'PWM1.peripheral.ccp0Pin.$assign    = "PB14"',
            'PWM1.peripheral.ccp2Pin.$assign    = "PA7"',
            "PWM1.PWM_CHANNEL_0.dutyCycle       = 0",
            "PWM1.PWM_CHANNEL_2.dutyCycle       = 0",
        ):
            self.assertIn(token, syscfg)

        for forbidden in (
            "MotorTestState",
            "MOTOR_TEST_DUTY_PERMILLE",
            "MOTOR_TEST_DURATION_MS",
            "MOTOR_PWM_INST",
            "DL_TimerA_setCaptureCompareValue",
            "DIAG_GPIO_MOTOR_AIN2_SAFE",
            "DIAG_GPIO_MOTOR_BIN2_SAFE",
        ):
            self.assertNotIn(forbidden, safety)
        self.assertIn("MotorDriver_stopAll", safety)
        for token in (
            "MotorDriver_prepareBrakeAll",
            "MotorDriver_engageBrakeAll",
            "MotorDriver_releaseBrakeAll",
            "gStatus.leftBraking = true",
            "gStatus.rightBraking = true",
            "setDuty(GPIO_MOTOR_PWM_C0_IDX, 1000U)",
            "setDuty(GPIO_MOTOR_PWM_C2_IDX, 1000U)",
        ):
            self.assertIn(token, driver + text("drivers/motor_driver.h"))
        self.assertNotIn("ti_msp_dl_config.h", motor_test)
        self.assertNotIn("DL_TimerA_", motor_test + console)
        self.assertIn("MotorTest_start", console)
        self.assertIn("#if MOTOR_SELFTEST_BUILD", console)
        self.assertIn("MotorTest_onTimebaseTick", text("empty.c"))

        output_tokens = (
            "MOTOR_PWM_INST",
            "DIAG_GPIO_MOTOR_AIN2_SAFE",
            "DIAG_GPIO_MOTOR_BIN2_SAFE",
        )
        for path in ROOT.rglob("*.c"):
            if ("Debug" in path.parts or "MotorSelfTest" in path.parts or
                    "firmware_tests" in path.parts):
                continue
            source = path.read_text(encoding="utf-8")
            if any(token in source for token in output_tokens):
                self.assertEqual(path.relative_to(ROOT).as_posix(),
                                 "drivers/motor_driver.c")
        self.assertIn("DL_TimerA_setCaptureCompareValue", driver)

    def test_req002_default_build_is_locked_and_status_only(self) -> None:
        config = text("config/firmware_config.h")
        req002 = text("app/req002.c")
        console = text("drivers/diag_console.c")

        self.assertRegex(
            config, r"#define\s+MOTOR_SELFTEST_BUILD\s+\(0U\)")
        self.assertIn(
            "#define REQ002_ACTUATION_BUILD              MOTOR_SELFTEST_BUILD",
            config)
        for gate in (
            "REQ002_CALIBRATION_VALID",
            "REQ002_ACTUATION_GATE_VALID",
            "REQ002_PHYSICAL_PARAMETERS_VALID",
            "REQ002_ACTUATOR_ADAPTER_ENABLED",
            "REQ002_PID_ENABLED",
        ):
            self.assertRegex(
                config, rf"#define\s+{gate}\s+REQ002_ACTUATION_BUILD")
        self.assertIn("#if REQ002_ACTUATION_BUILD", req002)
        self.assertIn("REQ002_TIMEOUT_MS", req002)
        self.assertNotIn("req002 start", console.lower())
        self.assertNotIn("Req002_service", console)
        self.assertIn("req002_status", console)

    def test_req002_tracking_adapter_is_isolated_and_safe(self) -> None:
        config = text("config/firmware_config.h")
        header = text("app/req002.h")
        req002 = text("app/req002.c")
        app = text("app/app.c")
        console = text("drivers/diag_console.c")
        motor = text("drivers/motor_driver.c")

        for token in (
            "Req002TrackingInput",
            "Req002ControlDecision",
            "REQ002_TRACKING_VALID_OBSERVATION",
            "REQ002_TRACKING_I2C_FAILURE",
            "REQ002_TRACKING_TIMEOUT",
            "REQ002_TRACKING_SIGNAL_INSUFFICIENT",
            "REQ002_TRACKING_LINE_LOST",
            "sampleAgeMs",
            "centeredError",
            "steeringCorrection",
            "analog[REQ002_LINE_SENSOR_COUNT]",
            "i2cFailureStage",
            "motionAuthorized",
            "actuatorLocked",
            "leftDemandPermille",
            "rightDemandPermille",
            "controlSequence",
            "Req002_abort",
        ):
            self.assertIn(token, header)

        self.assertIn("LineSensors_snapshot()", app)
        self.assertIn("LineTracking_getStatus()", app)
        self.assertIn("lineTracking->shadowCorrection", app)
        self.assertIn("StartButton_takePress(&gStartButton)", app)
        self.assertIn("&req002Tracking", app)
        self.assertNotIn("LineSensors_readRawBits()", app)
        self.assertNotIn("gNextOledUpdateMs", app)
        self.assertLess(app.index("LineTracking_service(nowMs);"),
                        app.index("Req002_service(nowMs"))

        for token in (
            "Req002_evaluateTracking",
            "input->sensorErrorCount",
            "input->lastSuccessMs",
            "input->steeringCorrection",
            "decision.snapshot = *input;",
            "MotorDriver_setVehicleForwardDuties",
            "BoardSafety_stop",
            "REQ002_MARKER_MIN_BLACK",
            "markerDetected",
            "blackCount",
            "REQ002_DEPART_CONFIRM_MS",
            "REQ002_MARKER_CONFIRM_MS",
            "REQ002_SOFT_START_MS",
            "REQ002_TIMEOUT_MS",
            "if (buttonPress && isActiveState",
            "gTrackingFaultPending",
            "REQ002_TRACKING_FAULT_CONFIRM_MS",
            "applyTrackingRecovery",
            "REQ002_TRACKING_RECOVERY_LEFT_PERMILLE",
            "REQ002_TRACKING_RECOVERY_RIGHT_PERMILLE",
            "reason == REQ002_BLOCK_SIGNAL_INSUFFICIENT",
            "reason == REQ002_BLOCK_LINE_LOST",
        ):
            self.assertIn(token, req002)

        self.assertIn(
            "#define REQ002_MARKER_MIN_BLACK              (4U)", config)
        self.assertIn("#define REQ002_SOFT_START_MS                 (300U)", config)
        self.assertIn("#define REQ002_TRACKING_FAULT_CONFIRM_MS     (100U)", config)
        self.assertIn(
            "#define REQ002_TRACKING_RECOVERY_LEFT_PERMILLE  (600U)",
            config)
        self.assertIn(
            "#define REQ002_TRACKING_RECOVERY_RIGHT_PERMILLE (290U)",
            config)
        self.assertIn("#define REQ002_BASE_PULSE_PERMILLE           (850U)", config)
        self.assertIn("#define REQ002_RIGHT_TRIM_PERMILLE            (420U)", config)
        self.assertIn(
            "#define REQ002_LEFT_SPEED_TARGET_RATIO          (1.20f)",
            config)
        self.assertIn("#define REQ002_RIGHT_CURVE_SLOWDOWN_PERMILLE  (220U)", config)
        self.assertIn("#define REQ002_LEFT_CURVE_SLOWDOWN_PERMILLE   (100U)", config)
        self.assertIn("#define REQ002_MAX_PULSE_PERMILLE             (900U)", config)
        self.assertIn(
            "#define REQ002_TURN_MAX_PULSE_PERMILLE        (1000U)",
            config)
        self.assertIn(
            "#define REQ002_TURN_RIGHT_MIN_PULSE_PERMILLE   (430U)",
            config)
        self.assertIn(
            "#define REQ002_TURN_BASE_PULSE_PERMILLE        (650U)",
            config)
        self.assertIn("#define REQ002_RIGHT_TURN_PULSE_PERMILLE      (1340U)", config)
        self.assertIn("#define REQ002_LEFT_TURN_PULSE_PERMILLE       (800U)", config)
        self.assertIn("#define REQ002_TURN_MIN_CORRECTION             (0.25f)", config)
        for token in (
            "REQ002_SHARP_RIGHT_ENTER_ERROR          (0.15f)",
            "REQ002_SHARP_RIGHT_EXIT_ERROR           (0.05f)",
            "REQ002_SHARP_RIGHT_CONFIRM_MS           (10U)",
            "REQ002_SHARP_RIGHT_MAX_MS               (100U)",
            "REQ002_SHARP_RIGHT_LEFT_PULSE_PERMILLE  (800U)",
            "REQ002_SHARP_RIGHT_RIGHT_PULSE_PERMILLE (0U)",
        ):
            self.assertIn(token, config)
        for token in (
            "gSharpRightTurnActive",
            "sharpRightTurnRequested",
            "sharpRightTurnUpdate",
            "commandSharpRightTurn",
            "applySharpRightTurn",
            "REQ002_SHARP_RIGHT_LEFT_PULSE_PERMILLE",
            "REQ002_SHARP_RIGHT_RIGHT_PULSE_PERMILLE",
            "speedBalanceReset();",
        ):
            self.assertIn(token, req002)
        self.assertIn("correctionMagnitude", req002)
        self.assertIn("steeringCommand", req002)
        self.assertIn("steeringMagnitude", req002)
        self.assertIn("steeringMagnitude < REQ002_TURN_MIN_CORRECTION", req002)
        self.assertIn("demandLimitPermille", req002)
        self.assertIn("REQ002_TURN_MAX_PULSE_PERMILLE", req002)
        self.assertIn("REQ002_TURN_RIGHT_MIN_PULSE_PERMILLE", req002)
        self.assertIn("REQ002_TURN_BASE_PULSE_PERMILLE", req002)
        self.assertIn("rightBase = leftBase;", req002)
        self.assertIn("rightTurnMinimum", req002)
        self.assertIn("clampDemand(leftDemand, demandLimitPermille)", req002)
        self.assertIn("if (correction < 0.0f)", req002)
        self.assertIn("rightBase = leftBase -", req002)
        self.assertIn("REQ002_RIGHT_TRIM_PERMILLE", req002)
        self.assertIn("REQ002_RIGHT_CURVE_SLOWDOWN_PERMILLE", req002)
        self.assertIn("REQ002_LEFT_CURVE_SLOWDOWN_PERMILLE", req002)
        self.assertIn("REQ002_RIGHT_TURN_PULSE_PERMILLE", req002)
        self.assertIn("REQ002_LEFT_TURN_PULSE_PERMILLE", req002)
        self.assertIn("rampScale", req002)
        self.assertIn(
            "gStatus.leftDemandPermille, gStatus.rightDemandPermille", req002)
        self.assertNotIn("REQ002_START_KICK_MS", config + req002)
        self.assertNotIn("PulseAccumulator", req002)
        self.assertNotIn("leftOutput = 1000U", req002)
        self.assertNotIn("rightOutput = 1000U", req002)
        self.assertIn("Req002_abort(Timebase_nowMs());", console)
        self.assertIn("REQ002_FINISH_BRAKE_PREPARE_MS         (1U)", config)
        self.assertIn("serviceReturnMarker(nowMs);", req002)
        self.assertLess(
            req002.index("serviceReturnMarker(nowMs);"),
            req002.index("if (!gStatus.tracking.dataValid) {",
                         req002.index("void Req002_service")))
        for token in (
            "MotorDriver_prepareBrakeAll();",
            "MotorDriver_engageBrakeAll();",
            "MotorDriver_releaseBrakeAll();",
            "gFinishBrakeEngaged",
            "gMarkerSinceMs + REQ002_MARKER_CONFIRM_MS",
        ):
            self.assertIn(token, req002)
        for token in (
            "last_left=",
            "last_right=",
            "speed_trim_milli=",
            "speed_peak_milli=",
            "encoder_missing_max_ms=",
            "encoder_missing_events=",
        ):
            self.assertIn(token, console)
        self.assertIn("Current physical vehicle-forward polarity", motor)
        self.assertIn("DIAG_GPIO_MOTOR_BIN2_SAFE", motor)
        self.assertIn("DIAG_GPIO_MOTOR_AIN2_SAFE", motor)
        self.assertNotIn("1000U - dutyPermille", motor)
        self.assertIn("setDuty(GPIO_MOTOR_PWM_C2_IDX, dutyPermille);", motor)

        self.assertNotIn("MotorDriver_", text("algorithm/line_tracking.c"))
        self.assertNotIn("motor_driver.h", text("algorithm/line_tracking.c"))
        self.assertNotIn("MotorDriver_", text("algorithm/pid.c"))

        evaluator_start = req002.index(
            "Req002ControlDecision Req002_evaluateTracking")
        evaluator_end = req002.index("void Req002_init", evaluator_start)
        evaluator = req002[evaluator_start:evaluator_end]
        for forbidden in (
            "gStatus",
            "Timebase_",
            "LineSensors_",
            "LineTracking_",
            "MotorDriver_",
        ):
            self.assertNotIn(forbidden, evaluator)
    def test_k230_uart1_read_only_diagnostic_contract(self) -> None:
        syscfg = text("empty.syscfg")
        entrypoint = text("empty.c")
        link = text("drivers/k230_link.c") + text("drivers/k230_link.h")

        for token in (
            'const UART2   = UART.addInstance();',
            'UART2.$name                    = "K230_UART"',
            'UART2.targetBaudRate           = 115200',
            'UART2.enabledInterrupts        = ["RX"]',
            'UART2.peripheral.$assign       = "UART1"',
            'UART2.peripheral.rxPin.$assign = "PA9"',
            'UART2.peripheral.txPin.$assign = "PA8"',
        ):
            self.assertIn(token, syscfg)

        for token in (
            '#include "drivers/k230_link.h"',
            'volatile K230RuntimeDiag gK230RuntimeDiag',
            'NVIC_DisableIRQ(K230_UART_INST_INT_IRQN);',
            'K230Link_init(Timebase_nowMs());',
            'K230Diag_service();',
            'void K230_UART_INST_IRQHandler(void)',
            'DL_UART_Main_isRXFIFOEmpty(K230_UART_INST)',
            'K230Link_pushRxByteFromIsr',
            'K230Link_service(nowMs);',
            'K230Link_snapshot(nowMs, &sample)',
            'K230Link_getStats(&stats);',
        ):
            self.assertIn(token, entrypoint)

        self.assertIn('K230_LINK_FRAME_SIZE               (14U)', link)
        self.assertIn('K230_LINK_TIMEOUT_MS               (100U)', link)
        self.assertIn('K230Protocol_crc8Atm', link)
        self.assertNotIn('MotorDriver_', link)
        self.assertNotIn('Req002_', link)
    def test_encoder_gpio_1x_speed_shadow_and_pi_contract(self) -> None:
        encoder = text("drivers/encoders.c") + text("drivers/encoders.h")
        entrypoint = text("empty.c")

        for token in (
            "ENCODER_SPEED_SHADOW_PERIOD_MS (10U)",
            "volatile EncoderSpeedShadow gEncoderSpeedShadow",
            "Encoders_speedShadowInit",
            "Encoders_speedShadowService",
            "gEncoderSpeedShadow.leftDelta",
            "gEncoderSpeedShadow.rightDelta",
            "gEncoderSpeedShadow.leftAbsDelta",
            "gEncoderSpeedShadow.rightAbsDelta",
            "gEncoderSpeedShadow.leftInvalidDelta",
            "gEncoderSpeedShadow.motionSampleCount",
            "gEncoderSpeedShadow.bothMotionSampleCount",
            "gEncoderSpeedShadow.leftAbsSum",
            "gEncoderSpeedShadow.rightAbsSum",
            "gEncoderSpeedShadow.leftBothAbsSum",
            "gEncoderSpeedShadow.rightBothAbsSum",
            "gEncoderSpeedShadow.leftAbsPeak",
            "gEncoderSpeedShadow.rightAbsPeak",
            "gEncoderSpeedShadow.invalidDuringMotion",
            "Encoders_speedShadowInit(Timebase_nowMs());",
            "Encoders_speedShadowService(Timebase_nowMs());",
        ):
            self.assertIn(token, encoder + entrypoint)

        snapshot_start = encoder.index("EncoderSnapshot Encoders_snapshot")
        snapshot_end = encoder.index("static uint32_t absoluteDelta", snapshot_start)
        snapshot = encoder[snapshot_start:snapshot_end]
        self.assertLess(snapshot.index("__disable_irq();"),
                        snapshot.index("snapshot.leftCount"))
        self.assertLess(snapshot.index("snapshot.leftCount"),
                        snapshot.index("snapshot.rightCount"))
        self.assertLess(snapshot.index("snapshot.rightCount"),
                        snapshot.index("__enable_irq();"))

        self.assertNotIn("MotorDriver_", encoder)
        self.assertNotIn("MOTOR_PWM", encoder)
        self.assertNotIn("Req002_", encoder)

        syscfg = text("empty.syscfg")
        req002 = text("app/req002.c")
        header = text("app/req002.h")
        config = text("config/firmware_config.h")
        for token in (
            'GPIO1.associatedPins[9].$name            = "LEFT_ENCODER_B"',
            'GPIO1.associatedPins[10].interruptEn      = true',
            'GPIO1.associatedPins[10].polarity         = "RISE"',
            'GPIO1.associatedPins[9].pin.$assign      = "PB4"',
            'GPIO1.associatedPins[10].$name            = "LEFT_ENCODER_A"',
            'GPIO1.associatedPins[10].pin.$assign      = "PB5"',
            "Encoders_onLeftEncoderAInterrupt",
            "DIAG_GPIO_LEFT_ENCODER_B_PIN",
            "void GROUP1_IRQHandler(void)",
            "DIAG_GPIO_INT_IIDX",
            "Encoders_speedShadowSnapshot",
        ):
            self.assertIn(token, syscfg + encoder + entrypoint)
        self.assertNotIn("LEFT_CAPTURE", syscfg + encoder + entrypoint)

        for token in (
            "REQ002_LEFT_ENCODER_TO_QEI_SCALE",
            "REQ002_LEFT_SPEED_TARGET_RATIO",
            "REQ002_SPEED_PI_STRAIGHT_THRESHOLD",
            "REQ002_SPEED_PI_OUTPUT_LIMIT",
            "REQ002_ENCODER_FEEDBACK_FAULT_MS",
            "REQ002_BLOCK_ENCODER_FEEDBACK_INVALID",
            "encoderFeedbackMissingMaxMs",
            "peakSpeedTrimPermille",
            "Pid_init(&gSpeedBalancePi",
            "Pid_step(controller",
            "Pid_init(&gTurnLeftPi",
            "SPEED_BALANCE_MODE_RIGHT_TURN",
            "speedTargetRatio = REQ002_LEFT_SPEED_TARGET_RATIO *",
            "leftDemand / rightDemand",
            "speedBalanceStep",
            "feedbackMonitorEnabled",
            "speedBalanceStep(speedBalanceMode,",
            "leftDemand += speedTrim",
            "rightDemand -= speedTrim",
            "speed.leftAbsDelta == 0U",
            "speed.rightAbsDelta == 0U",
        ):
            self.assertIn(token, config + req002 + header)
        self.assertIn("leftMissing && rightMissing", req002)
        self.assertIn("speedPiConfig.outputMin = 0.0f", req002)
        self.assertIn("speedPiConfig.integralMin = 0.0f", req002)
        self.assertIn("&gTurnLeftPi : &gSpeedBalancePi", req002)
        self.assertIn(
            "speedBalanceMode == SPEED_BALANCE_MODE_RIGHT_TURN", req002)
        self.assertIn("speedBalanceWatchdog(nowMs, leftMissing || rightMissing)", req002)
        self.assertNotIn(
            "if ((speed.leftAbsDelta == 0U) || (speed.rightAbsDelta == 0U)",
            req002)
    def test_pid_contains_limits_freeze_filter_and_anti_windup(self) -> None:
        header = text("algorithm/pid.h")
        source = text("algorithm/pid.c")
        for token in (
            "outputMin",
            "outputMax",
            "integralMin",
            "integralMax",
            "derivativeAlpha",
            "freezeIntegral",
        ):
            self.assertIn(token, header + source)
        self.assertIn("candidateIntegral", source)
        self.assertIn("unsaturated == output", source)
        self.assertIn("dtSeconds <= 0.0f", source)
        self.assertRegex(source, r"dtSeconds <= 0\.0f[^}]+return clampValue")

    def test_buzzer_is_absent_from_sysconfig_and_target_code(self) -> None:
        syscfg = text("empty.syscfg")
        target_sources = "\n".join(
            path.read_text(encoding="utf-8")
            for path in ROOT.rglob("*.c")
            if "firmware_tests" not in path.parts and "Debug" not in path.parts
        )
        self.assertNotIn('"BUZZER"', syscfg)
        self.assertNotIn("DIAG_GPIO_BUZZER", target_sources)
        self.assertIn('pin.$assign      = "PA21"', syscfg)
        self.assertNotIn('pin.$assign      = "PB21"', syscfg)
        self.assertIn('internalResistor = "PULL_UP"', syscfg)

    def test_entrypoint_is_thin_and_tests_are_not_target_sources(self) -> None:
        entrypoint = text("empty.c")
        self.assertLess(len(entrypoint.splitlines()), 220)
        self.assertIn("SYSCFG_DL_init();", entrypoint)
        self.assertIn("App_init();", entrypoint)
        self.assertIn("App_service();", entrypoint)
        build_sources = text("Debug/sources.mk")
        self.assertNotIn("firmware_tests", build_sources)

    def test_clangd_receives_device_and_arm_target_flags(self) -> None:
        clangd = text(".clangd")
        self.assertNotIn("Suppress: '*'", clangd)
        for flag in (
            "-D__MSPM0G3507__",
            "-D__USE_SYSCONFIG__",
            "--target=arm-none-eabi",
            "-mcpu=cortex-m0plus",
            "-mthumb",
        ):
            self.assertIn(flag, clangd)

    def test_pin_plan_v15_uses_six_line_i2c_and_removes_mpu(self) -> None:
        syscfg = text("empty.syscfg")
        header = text("drivers/line_sensors.h")
        source = text("drivers/line_sensors.c")
        app = text("app/app.c")
        console = text("drivers/diag_console.c")
        i2c_diag = text("drivers/i2c_diag.c")

        for token in (
            'I2C1.$name                     = "LINE_I2C"',
            'I2C1.peripheral.$assign        = "I2C0"',
            'I2C1.peripheral.sdaPin.$assign = "PA28"',
            'I2C1.peripheral.sclPin.$assign = "PA31"',
            'I2C2.$name                     = "OLED_I2C"',
            'I2C2.peripheral.$assign        = "I2C1"',
            'I2C2.peripheral.sdaPin.$assign = "PB3"',
            'I2C2.peripheral.sclPin.$assign = "PB2"',
        ):
            self.assertIn(token, syscfg)
        self.assertNotIn("MPU_I2C", syscfg + i2c_diag + console)
        self.assertNotRegex(syscfg + source, r"TCRT_OUT[1-5]")

        self.assertRegex(header, r"LINE_SENSOR_COUNT\s+\(6U\)")
        for token in (
            "LineSensors_init",
            "LineSensors_service",
            "LineSensors_snapshot",
            "uint16_t analog[LINE_SENSOR_COUNT]",
        ):
            self.assertIn(token, header)
        self.assertIn("LINE_I2C_INST", source)
        self.assertIn("LINE_SENSOR_I2C_ADDRESS       (0x5CU)", source)
        self.assertIn("LINE_SENSOR_DIGITAL_REGISTER  (0x05U)", source)
        self.assertIn("LINE_SENSOR_ANALOG_REGISTER   (0x06U)", source)
        self.assertIn("DL_I2C_INTERRUPT_CONTROLLER_TX_DONE", source)
        self.assertIn("DL_I2C_INTERRUPT_CONTROLLER_RX_DONE", source)
        self.assertIn("waitForTxDone", source)
        self.assertNotIn("waitForTransferComplete", source)
        self.assertIn("LineSensors_init(nowMs);", app)
        self.assertIn("LineSensors_service(nowMs);", app)
        for token in (
            "LINE_SENSOR_STARTUP_DELAY_MS",
            "LINE_SENSOR_BUS_RECOVERY_PULSES",
            "recoverBus",
            "DL_GPIO_readPins",
            "DL_GPIO_disableOutput",
            "DL_GPIO_enableOutput",
            "SYSCFG_DL_LINE_I2C_init",
            "lastRecoverySucceeded",
        ):
            self.assertIn(token, source + header + text("config/firmware_config.h"))
        self.assertIn("LINE_SENSOR_BUS_RECOVERY_PULSES    (9U)",
            text("config/firmware_config.h"))
        self.assertIn(
            "gNextPollMs = nowMs + LINE_SENSOR_STARTUP_DELAY_MS;", source)
        self.assertNotIn("DL_GPIO_setPins", source)
        self.assertIn("line_5c", console)
        self.assertIn("CH1..CH6_LEFT_TO_RIGHT", console)
        self.assertNotIn('strcmp(command, "mpu")', console)

        tracking = text("algorithm/line_tracking.c")
        tracking_header = text("algorithm/line_tracking.h")
        self.assertIn("1921U, 1514U, 1830U, 1604U, 1850U, 1607U", tracking)
        self.assertIn("3038U, 2797U, 3242U, 2400U, 2899U, 2336U", tracking)
        self.assertIn("LINE_TRACKING_CENTER_OFFSET", tracking)
        self.assertIn("Pid_step", tracking)
        self.assertIn("shadowCorrection", tracking_header)
        self.assertIn("LineTracking_service(nowMs);", app)
        self.assertIn("actuator_lock=1", console)


if __name__ == "__main__":
    unittest.main(verbosity=2)

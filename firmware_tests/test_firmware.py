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
            "REQ002_START_KICK_MS",
            "REQ002_TIMEOUT_MS",
            "if (buttonPress && isActiveState",
        ):
            self.assertIn(token, req002)

        self.assertIn(
            "#define REQ002_MARKER_MIN_BLACK              (4U)", config)
        self.assertIn("REQ002_BASE_PULSE_PERMILLE", config)
        self.assertIn("REQ002_TURN_PULSE_PERMILLE", config)
        self.assertIn("Req002_abort(Timebase_nowMs());", console)
        self.assertIn("Ground-verified vehicle-forward polarity", motor)
        self.assertIn("DIAG_GPIO_MOTOR_BIN2_SAFE", motor)
        self.assertIn("DIAG_GPIO_MOTOR_AIN2_SAFE", motor)

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
        self.assertLess(len(entrypoint.splitlines()), 100)
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

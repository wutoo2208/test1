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
    def test_working_copy_one_shot_profile_is_preserved(self) -> None:
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

    def test_req002_gates_are_false_and_uart_has_status_only(self) -> None:
        config = text("config/firmware_config.h")
        req002 = text("app/req002.c")
        console = text("drivers/diag_console.c")
        for gate in (
            "REQ002_CALIBRATION_VALID",
            "REQ002_ACTUATION_GATE_VALID",
            "REQ002_PHYSICAL_PARAMETERS_VALID",
            "REQ002_ACTUATOR_ADAPTER_ENABLED",
        ):
            self.assertRegex(config, rf"#define\s+{gate}\s+\(0U\)")
        self.assertIn("REQ002_TIMEOUT_MS", req002)
        self.assertNotIn("req002 start", console.lower())
        self.assertNotIn("Req002_service", console)
        self.assertIn("req002_status", console)

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
        self.assertIn('pin.$assign      = "PB21"', syscfg)
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


if __name__ == "__main__":
    unittest.main(verbosity=2)

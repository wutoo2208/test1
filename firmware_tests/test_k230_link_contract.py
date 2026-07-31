from __future__ import annotations

import re
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
HEADER = ROOT / "drivers" / "k230_link.h"
SOURCE = ROOT / "drivers" / "k230_link.c"


def crc8_atm(data: bytes) -> int:
    crc = 0
    for value in data:
        crc ^= value
        for _ in range(8):
            crc = ((crc << 1) ^ 0x07) & 0xFF if crc & 0x80 else (crc << 1) & 0xFF
    return crc


class K230LinkContractTests(unittest.TestCase):
    def test_crc8_atm_reference_vector(self) -> None:
        self.assertEqual(crc8_atm(b"123456789"), 0xF4)

    def test_frame_contract_is_14_bytes_with_crc_over_13(self) -> None:
        header = HEADER.read_text(encoding="utf-8")
        self.assertRegex(header, r"K230_LINK_FRAME_SIZE\s+\(14U\)")
        self.assertRegex(header, r"K230_LINK_CRC_INPUT_SIZE\s+\(13U\)")

        lost_without_crc = bytes.fromhex(
            "55 AA 00 00 00 00 00 00 00 FF FF FF FF"
        )
        frame = lost_without_crc + bytes([crc8_atm(lost_without_crc)])
        self.assertEqual(len(frame), 14)
        self.assertEqual(frame[-1], crc8_atm(frame[:-1]))

    def test_module_is_hardware_independent_and_documents_integration(self) -> None:
        header = HEADER.read_text(encoding="utf-8")
        source = SOURCE.read_text(encoding="utf-8")
        combined = header + source
        self.assertNotIn("ti_msp_dl_config.h", combined)
        self.assertNotIn("DL_UART_", combined)
        for token in (
            "CLAUDE CODE INTEGRATION NOTE",
            "K230Link_pushRxByteFromIsr",
            "K230Link_service",
            "K230Link_snapshot",
            "measurementUsable",
            "sequenceAgeMs",
            "lostPayloadCanonical",
        ):
            self.assertIn(token, combined)

    def test_protocol_offsets_and_safety_checks_are_present(self) -> None:
        source = SOURCE.read_text(encoding="utf-8")
        for token in (
            "gFrame[2]",
            "gFrame[3]",
            "&gFrame[4]",
            "&gFrame[6]",
            "gFrame[8]",
            "&gFrame[9]",
            "&gFrame[11]",
            "K230_LINK_TIMEOUT_MS",
            "K230_STATUS_LOST",
            "K230_STATUS_PREDICTED",
            "lostPayloadIsCanonical",
        ):
            self.assertIn(token, source)
        self.assertRegex(source, r"K230_CRC8_POLYNOMIAL\s+\(0x07U\)")
        self.assertIn("expectedCrc != gFrame[K230_LINK_FRAME_SIZE - 1U]", source)
        self.assertIn("sample->linkFresh &&", source)

    def test_queue_capacity_is_power_of_two(self) -> None:
        header = HEADER.read_text(encoding="utf-8")
        match = re.search(r"K230_LINK_RX_QUEUE_CAPACITY\s+\((\d+)U\)", header)
        self.assertIsNotNone(match)
        capacity = int(match.group(1))
        self.assertGreater(capacity, 14)
        self.assertEqual(capacity & (capacity - 1), 0)


if __name__ == "__main__":
    unittest.main(verbosity=2)

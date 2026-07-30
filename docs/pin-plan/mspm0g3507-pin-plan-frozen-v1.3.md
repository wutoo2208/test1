# MSPM0G3507 Pin Plan v1.3 — START_BUTTON / Buzzer DNC Amendment

> **Status**: `APPROVED FIRMWARE AMENDMENT / SYSCONFIG VALIDATED / NOT HARDWARE-TESTED`  
> **Date**: 2026-07-30  
> **Base**: `mspm0g3507-pin-plan-frozen-v1.2.md`. All v1.2 assignments remain unchanged except the two entries below.

| Pin | v1.3 owner | Configuration / policy | Evidence level |
|---|---|---|---|
| PB21 | `START_BUTTON` | GPIO input, internal `PULL_UP`; firmware interprets low as pressed and polls with 30 ms debounce; no IRQ | SysConfig validation + build only |
| PB27 | DNC | Removed from SysConfig; firmware does not intentionally configure, drive, toggle, or test it. Buzzer remains physically disconnected until type/polarity are known. | Static/build only |

PB21 is the only start path for the actuator-locked REQ-002 framework. UART and RF expose status only and cannot start the car. This amendment does not authorize motor PWM, a motor adapter, D36A motion, flashing, serial access, or hardware operation.

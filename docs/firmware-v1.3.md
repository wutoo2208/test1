# Firmware v1.3 Module and Safety Note

## Module map

| Directory | Ownership |
|---|---|
| `app/` | Application dispatch and actuator-locked REQ-002 state/timing framework |
| `bsp/` | SysTick timebase and continuously enforced motor/D36A safe lock |
| `drivers/` | UART diagnostics, nRF24 PTX, I2C diagnostics, line inputs, encoders, debounced PB21 button |
| `algorithm/` | Pure PID with output/integral limits, conditional anti-windup, integral freeze, derivative filtering |
| `config/` | Reviewed RF profile and explicit invalid/disabled REQ-002 physical/control gates |
| `firmware_tests/` | Host/source checks; excluded from the CCS target build |

`empty.c` now contains only SysConfig initialization, application init/service dispatch, and ISR forwarding.

## REQ-002 safety status

The framework exposes `IDLE`, `SAFE_LOCKED`, `BLOCKED_CALIBRATION`, `ARMED`, `DEPART_A`, `LAP_ACTIVE`, `RETURN_MARKER`, `STOPPING`, `COMPLETE`, and `FAULT`, with a 20 s timeout and elapsed/frozen timing fields. Marker/departure fields are scaffolding only: unknown line polarity/order cannot advance state.

Calibration, actuation, physical-parameter, actuator-adapter, and PID gates are deliberately `0`. A debounced PB21 press therefore reports `BLOCKED_CALIBRATION`, leaves timing frozen, and preserves the motor/D36A lock. UART/RF cannot start REQ-002. No TIMA0 PWM or motor adapter exists.

## Preserved RF behavior

The working-copy Baoqian profile remains enabled: channel 0, 2 Mbps, 16-bit CRC, 5-byte `FF:FF:FF:FF:FF` address, pipe-0 ACK, retry `0x1A`, static 32-byte payload with byte 0 holding user length (31-byte maximum). Boot diagnostics are kept on the wired UART and excluded from RF mirroring. Boot auto-arms and queues exactly `@RFTEST NF02PA LINK OK\r\n`; the radio disarms after the queued message either succeeds or exhausts hardware retries, so a failed one-shot cannot leave later diagnostic mirroring armed. The refactor does not add another automatic transmit path.

## Buzzer policy

PB27 is absent from SysConfig and is DNC. Firmware does not intentionally drive or test it and does not claim a safe polarity. Physical disconnection remains required until module type/polarity are known.

## Validation (2026-07-30)

- Standalone SysConfig 1.26.2 generation: PASS; informational STOP/STANDBY retention notices for SPI0, TIMG8 QEI, and TIMA1 Capture.
- CCS managed clean/full build, TI clang 5.1.1 LTS: PASS, 0 errors and 0 warnings.
- Link map after final fixes: FLASH `0x8538 / 0x20000`; SRAM `0x07bf / 0x8000`.
- Wireless CMSIS-DAP/OpenOCD probe: PASS at 1 MHz (`DPIDR=0x6ba02477`, Cortex-M0+ r0p1).
- OpenOCD flash and binary fallback verification: PASS at 500 kHz; target-side CRC acceleration timed out, then byte comparison reported no differences and `reset run` completed.
- DAP virtual UART COM8 at 115200: firmware `req002-safe-0.3` observed; software safe-output self-test PASS, motor `00/00`, D36A disabled, REQ-002 actuator lock active with all physical/control gates `0`.
- RF SPI register access: PASS (`CONFIG=0x0E`, `RF_CH=0x00`, `RF_SETUP=0x08`); one-shot ended in `MAX_RT`, then safely disarmed with queue empty. RF reception was not proven.
- PB21 physical press, TCRT polarity/order, encoder movement, OLED, buzzer, motor, D36A, and vehicle motion validation: **NOT RUN**.

# MSPM0G3507 Pin Plan — 最终候选快照

> **状态**：`SUPERSEDED BY mspm0g3507-pin-plan-frozen-v1.0.md`。
> **日期**：2026-07-30。
> **用途**：保留最终冻结前的收敛过程；**不得作为施工或 SysConfig 配置依据**。
> **当前工程事实**：`empty.syscfg` 仍只配置历史 `PB22` LED，本文件未修改该配置。

## 1. 已接受的路线

1. 优先使用拓展板现有 U8、MPU6050/GY、USART1、H10、H13、U6、U16 接口。
2. U2 仅作为改线后的 DRV8870 逻辑汇接区：隔离 `PB9/PB7/PB6`，保留 `PB12`，补 `PB14/PA7/PB24`。
3. DRV VIN/GND/AOUT/BOUT 不经过拓展板功率铜线；车轮电机直接接 DRV，U6/U16 只使用编码器四线。
4. K230 使用 USART1；USART0 保留 CH340；USART2 为未来保留且当前 DNC。
5. MS42CG V2 + D36A 单通道取代 RC 舵机作为摆杆执行器，使用 A/B/PWM/Z。
6. U3、U12、H8、所有 KEY/LED 不使用；旧无线 SPI 方案延期且不保留引脚。

## 2. 最终资源快照

| 子系统 | MCU 引脚 | 外设/模式 | 物理路径 |
|---|---|---|---|
| OLED | PA0 / PA1 | I2C0 SDA/SCL | U8 pad4/pad3 |
| MPU6050 | PB2 / PB3 | I2C1 SCL/SDA | GY_SCL/GY_SDA |
| K230 | PA8 / PA9 | UART1 TX/RX | USART1 pin2/pin1 |
| CH340 | PA10 / PA11 | UART0 TX/RX | 板载路径 |
| 右轮 DRV A | PB14 / PB12 | TIMA0 CCP0 / GPIO | 改线后 U2 pad14/pad15 |
| 左轮 DRV B | PA7 / PB24 | TIMA0 CCP2 / GPIO | 改线后 U2 pad12/pad11 |
| 右轮编码器 | PB10 / PB11 | TIMG8 QEI | U6 A1/B1 |
| 左轮编码器 | PB4 / PB5 | TIMA1 双 Capture | U16 A2/B2 |
| 五路循迹 | PB19/PB17/PA16/PA14/PB20 | GPIO inputs | H10 OUT_1..OUT_5 |
| 蜂鸣器 | PB27 | GPIO（仅有源模块） | H13 BEEP |
| D36A | PA26/PA24/PB0 | TIMG7 STEP / DIR GPIO / EN GPIO | H1.19/H1.18/H2.6 |
| MS42CG A/B | PA12/PA13 | TIMG0 双 Capture | H3.15/H4.15 |
| MS42CG PWM | PB26 | TIMG6 combined capture | H4.16 |
| MS42CG Z | PB23 | GPIO IRQ | H3.16 |

## 3. 已删除的旧候选

以下分配不再保留，只能在历史说明中出现：

- 无线 SPI0 的 `PB17/PB18/PB19/PB25/PA25/PA27`；
- RC 舵机 `PB15/TIMG7`；
- 五路红外旧 GPIO 池 `PB1/PB12/PA12/PA13/PB23`；
- 右轮编码器旧候选 `PA29/PA30`；
- 旧电机 PWM `PB13/TIMA0 CCP3`；
- “U2/U3 都保持空置”的旧描述。

## 4. 保留和禁止

- `PA19/PA20`：SWD；
- `PA2–PA6`：ROSC/晶体；
- `PA18`：BSL；
- `PA21/PA23`：VREF；
- `PB6–PB9`：板载 Flash Owner；只允许在拓展板 U2 侧切断到目标焊盘的冲突支路，不得破坏 MCU→Flash；
- `PB15/PB16`：USART2 future-reserved，当前不配置、不接线；
- `PB21/PB22`、拓展板 KEY1–4/LED1–2：不分配；
- U3、U12、H8：DNC。

权威资源和施工边界以：

- `docs/pin-plan/mspm0g3507-pin-plan-frozen-v1.0.md`
- `docs/pin-plan/mspm0g3507-adapter-harness.md`

为准。
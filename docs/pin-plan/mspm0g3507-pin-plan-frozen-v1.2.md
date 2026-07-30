# MSPM0G3507 最终 Pin Plan v1.2（最新版）

> **状态**：`FROZEN-DESIGN / USER-ACCEPTED / NOT WIRED`。
> **冻结日期**：2026-07-30。
> **适用硬件**：天猛星 MSPM0G3507 LQFP-64 + 拓展板 V2 + 双路 DRV8870 + 两轮编码器 + 五路 TCRT5000 + OLED + MPU6050 + K230 + D36A 通道1 + MS42CG V2 + SPI0 无线候选。
> **工具基线**：MSPM0 SDK `2.11.00.07`；当前 `empty.syscfg` 仍只配置历史 PB22 LED，尚未按本计划修改。
> **授权边界**：本冻结版只确认设计资源与最新版端点；不授权修改 `.syscfg`、接线、上电、构建、烧录、串口、SPI 或运动测试。

## 0. 最新版唯一性规则

从本版冻结起，**每个模块的 MCU 引脚、拓展板针位、线束端点和 DNC 均只以以下两个文件为准**：

1. `docs/pin-plan/mspm0g3507-pin-plan-frozen-v1.2.md`
2. `docs/pin-plan/mspm0g3507-adapter-harness-v1.2.md`

以下文件仅用于追溯历史，禁止作为接线、SysConfig 或施工依据：

- `mspm0g3507-pin-plan-frozen-v1.0.md`
- `mspm0g3507-adapter-harness.md`
- `mspm0g3507-pin-plan-candidate.md`
- `mspm0g3507-pin-plan-wireless-spi0-v1.1-candidate.md`
- `mspm0g3507-pin-plan-wireless-spi0-v1.2-candidate.md`

任何后续变化必须发布更高版本，禁止静默修改或混用旧表。

## 1. v1.2 冻结的拓扑决定

1. OLED 使用 I2C0，MPU6050 使用 I2C1，K230 使用 UART1，CH340 独占 UART0。
2. UART2 不再保留为完整串口：`PB16` 归无线 IRQ，`PB15` 保持 DNC。
3. SPI0 无线使用 `PB17/PA12/PB19/PB25`，并以 `PB1` 控制 CE、`PB16` 接 IRQ。
4. `PB18` 由用户确认连接 KEY3，标记为板载占用，禁止用于无线或其他新增功能。
5. 为释放 PB17/PB19，TCRT OUT1/OUT2 改到 PA25/PA27；H10 pin1/2/6 改作无线 MISO/MOSI/CSN。
6. MS42CG A/B 改到 PA29/PA30 的 TIMG6 双 Capture；PWM 改到 PA13/TIMG0 CCP1 单输入双边沿捕获候选；Z 保持 PB23。
7. U2 继续采用三隔离、三桥接、一保留；DRV 功率完全绕过拓展板。
8. D36A 只使用通道1；MS42CG 仍使用 3.3 V AB/PWM/Z 模式，不执行 SPI 改焊。
9. 无线 pin1/pin2 仍为 `UNKNOWN / DO-NOT-CONNECT`，所以本版没有冻结无线电源线。
10. 无线只作为开发期低速数据/遥测候选，不替代 H 题的视频图传，也不得用于正式比赛人工遥控。

## 2. MCU 唯一 Owner 表

| MCU 引脚 | 唯一 Owner | 外设/模式 | 最新物理路径 | 状态 |
|---|---|---|---|---|
| PA0 | OLED SDA | I2C0 SDA | U8 pad4 | FROZEN |
| PA1 | OLED SCL | I2C0 SCL | U8 pad3 | FROZEN |
| PA7 | 左轮 DRV `BIN1` | TIMA0 CCP2 PWM | U2 pad10→改线 pad12 | FROZEN |
| PA8 | K230 RX 的 MCU TX | UART1 TX | USART1 pin2 | FROZEN |
| PA9 | K230 TX 的 MCU RX | UART1 RX | USART1 pin1 | FROZEN |
| PA10 | CH340 日志 TX | UART0 TX | 板载 CH340 | FROZEN |
| PA11 | CH340 日志 RX | UART0 RX | 板载 CH340 | FROZEN |
| PA12 | 无线 SCK | SPI0 SCLK | H3 pin15 / AIN4 | FROZEN-DESIGN |
| PA13 | MS42CG PWM | TIMG0 CCP1 单输入 Capture 候选 | H4 pin15 / BIN3 | FROZEN-DESIGN / SYSCONFIG-PENDING |
| PA14 | TCRT OUT4 | GPIO input | H10 pin4 | FROZEN |
| PA16 | TCRT OUT3 | GPIO input | H10 pin3 | FROZEN |
| PA24 | D36A DIR1 | GPIO output | H1 pin18 | FROZEN |
| PA25 | TCRT OUT1 | GPIO input | H10 pin7 / OUT7 | FROZEN-DESIGN |
| PA26 | D36A ST1 | TIMG7 CCP0 STEP | H1 pin19 | FROZEN |
| PA27 | TCRT OUT2 | GPIO input | H10 pin8 / OUT8 | FROZEN-DESIGN |
| PA29 | MS42CG A | TIMG6 CCP0 Capture | H4 pin17 | FROZEN-DESIGN |
| PA30 | MS42CG B | TIMG6 CCP1 Capture | H2 pin7 | FROZEN-DESIGN |
| PB0 | D36A EN1 | GPIO output，低休眠 | H2 pin6 | FROZEN |
| PB1 | 无线 CE | GPIO output，默认低 | H1 pin6 | FROZEN-DESIGN |
| PB2 | MPU6050 SCL | I2C1 SCL | GY_SCL | FROZEN |
| PB3 | MPU6050 SDA | I2C1 SDA | GY_SDA | FROZEN |
| PB4 | 左轮编码器 A2 | TIMA1 CCP0 Capture | U16 pin4 | FROZEN |
| PB5 | 左轮编码器 B2 | TIMA1 CCP1 Capture | U16 pin3 | FROZEN |
| PB10 | 右轮编码器 A1 | TIMG8 CCP0/PHA | U6 pin4 | FROZEN |
| PB11 | 右轮编码器 B1 | TIMG8 CCP1/PHB | U6 pin3 | FROZEN |
| PB12 | 右轮 DRV `AIN2` | GPIO output | U2 pad15 | FROZEN |
| PB14 | 右轮 DRV `AIN1` | TIMA0 CCP0 PWM | U2 pad16→改线 pad14 | FROZEN |
| PB16 | 无线 IRQ（低有效） | GPIO input / IRQ 候选 | H3 pin7 | FROZEN-DESIGN / ELECTRICAL-PENDING |
| PB17 | 无线 MOSI | SPI0 PICO | H10 pin2 / OUT2 | FROZEN-DESIGN |
| PB19 | 无线 MISO | SPI0 POCI | H10 pin1 / OUT1 | FROZEN-DESIGN |
| PB20 | TCRT OUT5 | GPIO input | H10 pin5 | FROZEN |
| PB23 | MS42CG Z | GPIO IRQ input | H3 pin16 | FROZEN |
| PB24 | 左轮 DRV `BIN2` | GPIO output | H1 pin17→改线 U2 pad11 | FROZEN |
| PB25 | 无线 CSN | SPI0 CS0 | H10 pin6 / OUT6 | FROZEN-DESIGN |
| PB27 | 有源蜂鸣器 BEEP | GPIO output | H13 pin2 | CONDITIONAL-MODULE |

未列出的引脚保持未分配；禁止因“看起来空闲”自动启用。

## 3. 板载保留、DNC 和禁止资源

| 资源 | 最新状态 | 原因 |
|---|---|---|
| PA2-PA6 | FORBIDDEN | ROSC / LFX / HFX 时钟网络。 |
| PA18 | FORBIDDEN | BSL 启动网络。 |
| PA19/PA20 | FROZEN-RESERVED | SWDIO/SWCLK。 |
| PA21/PA23 | FORBIDDEN | VREF-/VREF+ 网络。 |
| PB6-PB9 | FORBIDDEN | 板载 SPI Flash Owner。 |
| PB15 | DNC | UART2 已释放，不自动分配剩余单线。 |
| PB18 | FORBIDDEN-FOR-RADIO / KEY3 | 用户确认连接 KEY3。 |
| PB21/PB22 | DNC / BOARD-LOADED | 板载 KEY/LED 不用于比赛功能。 |
| PB26 | DNC / RELEASED | v1.0 MS42CG PWM 旧引脚，v1.2 不使用。 |
| U3/U12/H8 | DNC | 旧接口/冲突资源不使用。 |
| 无线 pin1/pin2 | DNC / BLOCKED | 功能与电源身份未知。 |

## 4. 外设实例 Owner

| 外设实例 | 唯一 Owner | v1.2 配置契约 |
|---|---|---|
| I2C0 | OLED | PA0 SDA、PA1 SCL；全总线仅允许 3.3 V 上拉。 |
| I2C1 | MPU6050 | PB2 SCL、PB3 SDA；上拉电压/地址仍待实物关闭。 |
| UART0 | CH340 日志 | PA10 TX、PA11 RX；USART0 外接座 DNC。 |
| UART1 | K230 | PA8 TX、PA9 RX；电平/协议/供电待验证。 |
| UART2 | 无 Owner | 不创建实例；PB16 已归无线 IRQ，PB15 DNC。 |
| SPI0 | 无线 | CS0=PB25、PICO=PB17、SCLK=PA12、POCI=PB19；mode/频率待确认。 |
| TIMA0 | 两轮 DRV PWM | CCP0=PB14、CCP2=PA7；共周期。 |
| TIMG8 | 右轮编码器 | PB10/PB11 两相硬件 QEI。 |
| TIMA1 | 左轮编码器 | PB4/PB5 双 Capture；软件正交。 |
| TIMG7 | D36A STEP | PA26 CCP0；独占可变步频输出。 |
| TIMG6 | MS42CG A/B | PA29 CCP0、PA30 CCP1 双 Capture；软件正交。 |
| TIMG0 | MS42CG PWM | PA13 CCP1 单输入双边沿 Capture 候选；需 SysConfig 隔离验证。 |
| GPIO IRQ | MS42CG Z + 无线 IRQ | PB23 Z；PB16 无线低有效 IRQ，输出结构/上拉待确认。 |

## 5. H10 最新复用表

| H10 | 最新功能 | MCU | 模块端 |
|---:|---|---|---|
| 1 / OUT1 | 无线 MISO | PB19/SPI0 POCI | 无线 pin8 |
| 2 / OUT2 | 无线 MOSI | PB17/SPI0 PICO | 无线 pin5 |
| 3 / OUT3 | TCRT OUT3 | PA16 GPIO | TCRT OUT3 |
| 4 / OUT4 | TCRT OUT4 | PA14 GPIO | TCRT OUT4 |
| 5 / OUT5 | TCRT OUT5 | PB20 GPIO | TCRT OUT5 |
| 6 / OUT6 | 无线 CSN | PB25/SPI0 CS0 | 无线 pin3 |
| 7 / OUT7 | TCRT OUT1 | PA25 GPIO | TCRT OUT1 |
| 8 / OUT8 | TCRT OUT2 | PA27 GPIO | TCRT OUT2 |
| 9 | TCRT 5V 候选 | — | 电平/电源批准前不接 |
| 10 | 逻辑 GND | — | 回流方案确认后接 |

TCRT 必须使用独立跳线或交叉排线；禁止沿用 v1.0 的连续 pin1-pin5 线束。

## 6. 无线 SPI0 最新信号表

| 无线针脚 | 功能 | MCU / 路径 | 最新状态 |
|---:|---|---|---|
| 1 | UNKNOWN | DNC | 禁止推断供电/地。 |
| 2 | UNKNOWN | DNC | 禁止推断供电/地。 |
| 3 | CSN | PB25 / H10 pin6 | 冻结设计。 |
| 4 | CE | PB1 / H1 pin6 | 默认低；冻结设计。 |
| 5 | MOSI | PB17 / H10 pin2 | 冻结设计。 |
| 6 | SCK | PA12 / H3 pin15 | 冻结设计；PB18 不使用。 |
| 7 | IRQ 低有效 | PB16 / H3 pin7 | 冻结设计；上拉/输出结构待确认。 |
| 8 | MISO | PB19 / H10 pin1 | 冻结设计。 |

无线模块 pin1/pin2 未关闭前，整个无线模块保持不供电、不接线。

## 7. MS42CG 最新反馈表

| MS42CG | MCU / 路径 | 模式 |
|---|---|---|
| VCC | 经批准后的 3.3 V | 禁止 5 V 输出进 MSPM0。 |
| A | PA29 / H4 pin17 | TIMG6 CCP0 Capture。 |
| B | PA30 / H2 pin7 | TIMG6 CCP1 Capture。 |
| PWM | PA13 / H4 pin15 | TIMG0 CCP1 单输入双边沿捕获候选。 |
| Z | PB23 / H3 pin16 | GPIO IRQ。 |
| GND | 逻辑地 | 与功率回流分开规划。 |

MS42CG 不改 SPI 模式、不接 D36A 控制头、不直插 U6/U16。PWM 捕获方案必须在下一授权阶段用 SysConfig 隔离验证，冻结 Pin Plan 不等于已验证配置。

## 8. 其他模块最新版摘要

| 模块 | 最新 MCU/接口 | 最新接线路径 |
|---|---|---|
| OLED | PA0/PA1 I2C0 | U8 SDA/SCL；电源与 3.3 V 上拉待确认。 |
| MPU6050 | PB2/PB3 I2C1 | GY_SCL/GY_SDA；电源/地址/上拉待确认。 |
| K230 | PA8/PA9 UART1 | USART1 pin2/pin1；5V 针 DNC，独立供电。 |
| CH340 | PA10/PA11 UART0 | 板载路径；外部 USART0 DNC。 |
| DRV8870 控制 | PB14/PB12/PA7/PB24 | U2 改线后接 DRV H3 pin1/3/2/4。 |
| DRV8870 功率 | 无 MCU | VIN/GND/AOUT/BOUT 独立重载线，绕过拓展板。 |
| 右轮编码器 | PB10/PB11 TIMG8 | U6 pin4/pin3，U6 pin1/6 DNC。 |
| 左轮编码器 | PB4/PB5 TIMA1 | U16 pin4/pin3，U16 pin1/6 DNC。 |
| D36A 通道1 | PA26/PA24/PB0 | H1.19/H1.18/H2.6 → ST1/DIR1/EN1。 |
| 蜂鸣器 | PB27 GPIO | H13 pin2；仅低电流有源模块候选。 |

完整逐线要求见最新版 harness。

## 9. 冻结后仍开放的工程问题

- 无线 pin1/pin2、供电、IRQ 输出结构、SPI mode/频率、对端和协议；
- TCRT 输出电平/结构/极性和车体物理顺序；
- MS42CG PWM 单通道捕获的 SysConfig 可配置性、精度和最高边沿频率；
- D36A、DRV、电机、编码器、I2C、K230、电源与保护的既有未关闭项；
- 所有连接器观察面、Pin 1、实际线色和断电连续性。

## 10. 下一授权门槛

本版已冻结，但用户明确未授权：

```text
SysConfig 修改
接线
上电
构建/烧录
串口/SPI/探针
硬件运动
```

下一步若需要修改 `.syscfg`，必须另行批准精确文件范围和 L1/L2 验证等级。

## 11. 本轮未执行

- 未修改 `empty.syscfg`、源码、CCS 工程或生成文件；
- 未制作/连接任何线束；
- 未给无线、TCRT、MS42CG、D36A、DRV 或其他模块上电；
- 未运行 SysConfig、构建、烧录、探针、串口、SPI 或运动测试。

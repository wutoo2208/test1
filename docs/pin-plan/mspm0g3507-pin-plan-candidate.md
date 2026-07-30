# MSPM0G3507 候选 Pin Plan

> **状态**：`CANDIDATE`，2026-07-29。
> **适用工程**：天猛星 MSPM0G3507（LQFP-64）+ 天猛星拓展板 V2.0。
> **工具基线**：MSPM0 SDK `2.11.00.07`；当前 `empty.syscfg` 仅配置 `PB22` LED。
> **证据等级**：开发板/拓展板资料复核 + EPRO 静态网络 + 本地 SDK IOMUX 静态核验；无线模块排针 3–8 的名称和方向来自用户陈述。
> **使用限制**：本文件不是已确认接线，不授权修改 `.syscfg`、接线、上电、烧录或运动测试。无线模块排针 1/2 为 `UNKNOWN / DO-NOT-CONNECT`，不得根据常见 nRF 模块线序推断供电或地；标为 `CONDITIONAL` 的行不得进入冻结版。

## 1. 关键勘误与路线决定

1. 先前“DRV8870 已复用 TB6612 插座并实测通过”的消息已由用户撤回，**没有实测证据**。
2. 拓展板 `U2/U3` 是 TB6612 的 `2×8` 插座。此前给出的单路八针 `IN1/IN2/ISEN/VM/GND/OUT1/OUT2/VREF` 表，不对应已归档双路 DRV8870 模块的 H3，也不能直接插入 U2/U3。
3. 本候选计划要求：**U2/U3 保持空置；双路 DRV8870 通过独立逐针线束连接。**
4. 五路 TCRT5000 使用独立跳线到 GPIO，不使用 H10 的 `OUT_1`–`OUT_5` 作为未经闭合的 MCU 映射。
5. MPU6050 按用户选择，飞线至拓展板 `GY_SCL/GY_SDA`；由本地器件 IOMUX 复核得 `PB2/PB3` 可分别配置为 `I2C1 SCL/SDA`。
6. 舵机信号由 `PB15/TIMG7 CCP0` 直接跳线到舵机控制排针；不假定 `SERVO1`–`SERVO4` 已连接 MCU。
7. 改装无线发送模块不再按串口规划。用户提供的排针定义仅覆盖 `3=CSN`、`4=CE`、`5=MOSI`、`6=SCK`、`7=IRQ（低有效）`、`8=MISO`；排针 1/2 保持 `UNKNOWN / DO-NOT-CONNECT`。
8. 无线候选分配为 `SPI0`：`PB17=PICO/MOSI`、`PB18=SCLK`、`PB19=POCI/MISO`、`PB25=CS0/CSN`，另以 `PA25` 控制 CE、`PA27` 接收 IRQ。该组只能从开发板原始 `U22` 排针候选取线，拓展板 H1–H4 未引出这些网络；接线前仍须对照原始引脚图确认 U22 观察面和针号。
9. `PA8/PA9 UART1` 继续用于视觉坐标通信。新增无线链路仅登记为用途待定义的低速数据/遥测候选，不替代视觉 UART、摄像头或题目要求的视频发送、接收、显示、存储和回放链路，也不得用于比赛过程人工遥控。

## 2. 永久保留或禁止分配

| 资源 | 状态 | 原因 |
|---|---|---|
| `PA2` | `FORBIDDEN` | ROSC 网络。 |
| `PA3/PA4` | `FORBIDDEN` | 32.768 kHz 晶体网络。 |
| `PA5/PA6` | `FORBIDDEN` | 40 MHz 晶体网络。 |
| `PA18` | `FORBIDDEN` | BSL 启动网络。 |
| `PA19/PA20` | `FROZEN-RESERVED` | SWDIO/SWCLK。 |
| `PA21/PA23` | `FORBIDDEN` | VREF-/VREF+ 网络。 |
| `SPI1 / PB6`–`PB9` | `FORBIDDEN` | 板载 SPI Flash 的唯一 Owner；`PB8/PB9` 还与 LCD 共享。无线模块不得改用或跳接到该引脚组。 |
| `PA10/PA11` | `FROZEN-RESERVED` | 仅用于板载 CH340E/UART0 调试。 |
| 拓展板 `U2/U3` | `FORBIDDEN-FOR-DRV` | TB6612 机械/电气接口，不允许 DRV8870 直接插接。 |

## 3. 候选功能分配

| Pin ID | 子系统 | 模块引脚/角色 | MCU 引脚 | 外设实例/通道 | 拓展板物理取线点 | 方向 | 电源/电平 | 复位安全态 | 状态 | 冻结前关闭条件 |
|---|---|---|---|---|---|---|---|---|---|---|
| `PIN-I2C0-SDA` | OLED | SDA | `PA0` | `I2C0 SDA` | U8 pad4；H3 pin2 | 双向开漏 | 3.3 V 上拉 | 释放总线 | `CONDITIONAL-ELECTRICAL` | OLED 线序、地址、全部上拉电压与等效阻值。 |
| `PIN-I2C0-SCL` | OLED | SCL | `PA1` | `I2C0 SCL` | U8 pad3；H3 pin3 | 输出开漏 | 3.3 V 上拉 | 释放总线 | `CONDITIONAL-ELECTRICAL` | 同上。 |
| `PIN-IMU-SCL` | MPU6050 | `GY_SCL` | `PB2` | `I2C1 SCL` | H3 pin8；MPU6050 pin3 | 输出开漏 | 待确认模块上拉；MCU 仅 3.3 V | 释放总线 | `CONDITIONAL-ELECTRICAL` | 实物芯片/载板、上拉电压、地址、安装轴向。 |
| `PIN-IMU-SDA` | MPU6050 | `GY_SDA` | `PB3` | `I2C1 SDA` | H3 pin9；MPU6050 pin4 | 双向开漏 | 同上 | 释放总线 | `CONDITIONAL-ELECTRICAL` | 同上。 |
| `PIN-UART0-TX` | 调试日志 | MCU TX | `PA10` | `UART0 TX` | 板载 CH340E；USART0 | 输出 | 3.3 V | 高阻/空闲高 | `FROZEN-RESOURCE` | 波特率和帧格式可在固件阶段决定。 |
| `PIN-UART0-RX` | 调试日志 | MCU RX | `PA11` | `UART0 RX` | 板载 CH340E；USART0 | 输入 | 3.3 V | 输入 | `FROZEN-RESOURCE` | 同上。 |
| `PIN-VISION-TX` | 视觉通信 | MCU TX→视觉 RX | `PA8` | `UART1 TX` | USART1；H3 pin4 | 输出 | 必须确认 3.3 V | 空闲高 | `CONDITIONAL-PROTOCOL` | K230/视觉板精确型号、波特率、帧格式、电平。 |
| `PIN-VISION-RX` | 视觉通信 | MCU RX←视觉 TX | `PA9` | `UART1 RX` | USART1；H3 pin5 | 输入 | 必须确认 3.3 V | 输入 | `CONDITIONAL-PROTOCOL` | 同上；若单向坐标流，可只连接此线与 GND。 |
| `PIN-RADIO-CSN` | 改装无线发送模块 | pin3 `CSN` | `PB25` | `SPI0 CS0` | 开发板原始 `U22 pin7` 候选；非 H1–H4 | MCU→模块 | MCU 为 3.3 V 逻辑域；模块兼容性未知 | MCU 复位期高阻；未来须保证失能为高，偏置实现未知 | `PINMUX-VERIFIED / PLAN-CONDITIONAL` | 精确模块/版本、两端观察面、pin 1/2、电源与逻辑电平、CSN 偏置、U22 针号实图复核。 |
| `PIN-RADIO-CE` | 改装无线发送模块 | pin4 `CE` | `PA25` | GPIO output | 开发板原始 `U22 pin5` 候选；非 H1–H4 | MCU→模块 | 同上 | MCU 复位期高阻；未来期望失能为低，外部下拉待模块资料确认 | `PLAN-CONDITIONAL-DEFAULT-STATE` | CE 有效逻辑、复位偏置、上电时序及 U22 针号实图复核。 |
| `PIN-RADIO-MOSI` | 改装无线发送模块 | pin5 `MOSI` | `PB17` | `SPI0 PICO/MOSI` | 开发板原始 `U22 pin15` 候选；非 H1–H4 | MCU→模块 | 同上 | MCU 复位期高阻；CSN 偏置未知，模块选中状态未知 | `PINMUX-VERIFIED / PLAN-CONDITIONAL` | SPI 模式/电平/安全频率、模块与对端协议、U22 针号实图复核。 |
| `PIN-RADIO-SCK` | 改装无线发送模块 | pin6 `SCK` | `PB18` | `SPI0 SCLK` | 开发板原始 `U22 pin16` 候选；非 H1–H4 | MCU→模块 | 同上 | MCU 复位期高阻；空闲极性未知 | `PINMUX-VERIFIED / PLAN-CONDITIONAL` | SPI mode、最大安全 SCK、时序及 U22 针号实图复核。 |
| `PIN-RADIO-IRQ` | 改装无线发送模块 | pin7 `IRQ`，低有效 | `PA27` | GPIO input / interrupt 候选 | 开发板原始 `U22 pin3` 候选；非 H1–H4 | 模块→MCU | MCU 为 3.3 V 逻辑域；模块输出电平未知 | MCU 输入；输出结构、无效电平和上拉需求未知 | `PLAN-CONDITIONAL-ELECTRICAL` | IRQ 推挽/开漏、无效电压、上拉、轮询/中断策略及 U22 针号实图复核。 |
| `PIN-RADIO-MISO` | 改装无线发送模块 | pin8 `MISO` | `PB19` | `SPI0 POCI/MISO` | 开发板原始 `U22 pin17` 候选；非 H1–H4 | 模块→MCU | MCU 为 3.3 V 逻辑域；模块输出电平未知 | MCU 输入；CSN 无效时模块三态行为未知 | `PINMUX-VERIFIED / PLAN-CONDITIONAL` | 未选中三态行为、逻辑电平、时序及 U22 针号实图复核。 |
| `PIN-MOTOR-A-PWM` | 电机通道 A | DRV A 路 PWM 输入 | `PA7` | `TIMA0 CCP2` | H1 pin5 | 输出 | 3.3 V 逻辑 | MCU 复位期高阻；DRV 输入偏置未知 | `BLOCKED-DEFAULT-STATE` | H3 观察面、PWM/方向真值、输入下拉及通道对应车轮。 |
| `PIN-MOTOR-A-DIR` | 电机通道 A | DRV A 路方向输入 | `PB14` | GPIO output | H1 pin4 | 输出 | 3.3 V 逻辑 | MCU 复位期高阻；DRV 输入偏置未知 | `BLOCKED-DEFAULT-STATE` | 同上；确认最终接 AIN1 或 AIN2。 |
| `PIN-MOTOR-B-PWM` | 电机通道 B | DRV B 路 PWM 输入 | `PB13` | `TIMA0 CCP3` | H4 pin9 | 输出 | 3.3 V 逻辑 | MCU 复位期高阻；DRV 输入偏置未知 | `BLOCKED-DEFAULT-STATE` | 同上；确认最终接 BIN1 或 BIN2。 |
| `PIN-MOTOR-B-DIR` | 电机通道 B | DRV B 路方向输入 | `PB24` | GPIO output | H1 pin17 | 输出 | 3.3 V 逻辑 | MCU 复位期高阻；DRV 输入偏置未知 | `BLOCKED-DEFAULT-STATE` | 同上。 |
| `PIN-ENC1-A` | 编码器1 | A/PHA | `PA29` | `TIMG8 CCP0/PHA` | H4 pin17 | 输入 | 编码器资料称 3.3 V；待实物复核 | 输入 | `CONDITIONAL-MECHANICAL` | A/B 相、PPR/CPR、最高边沿频率、左右轮归属。 |
| `PIN-ENC1-B` | 编码器1 | B/PHB | `PA30` | `TIMG8 CCP1/PHB` | H2 pin7 | 输入 | 同上 | 输入 | `CONDITIONAL-MECHANICAL` | 同上。 |
| `PIN-ENC2-A` | 编码器2 | A/Capture0 | `PB4` | `TIMA1 CCP0` | 拓展板 `A2`；H4 pin4 | 输入 | 编码器资料称 3.3 V；待实物复核 | 输入 | `CONDITIONAL-MECHANICAL` | 确认 `A2` 实物线序、PPR/CPR、Capture 解码策略。 |
| `PIN-ENC2-B` | 编码器2 | B/Capture1 | `PB5` | `TIMA1 CCP1` | 拓展板 `B2`；H4 pin5 | 输入 | 同上 | 输入 | `CONDITIONAL-MECHANICAL` | 同上。 |
| `PIN-SERVO-PWM` | 摆杆舵机 | 信号 | `PB15` | `TIMG7 CCP0` | H3 pin6，再跳到舵机控制排针 | 输出 | 3.3 V 信号；电源为独立 `OUT+` 候选 | MCU 复位期高阻；初始化后先保持 PWM 禁用 | `CONDITIONAL-ACTUATOR` | 精确舵机型号、额定电压、脉宽范围、方向、中位、限位及高阻时行为。 |
| `PIN-LINE-1` | 五路循迹 | TCRT `OUT1` | `PB1` | GPIO input | H1 pin6 | 输入 | 输出高电平未知 | 输入，无危险输出 | `CONDITIONAL-ELECTRICAL` | 输出结构/高电平/极性、电平转换、左→右映射。 |
| `PIN-LINE-2` | 五路循迹 | TCRT `OUT2` | `PB12` | GPIO input | H4 pin8 的 `AIN2` 网络 | 输入 | 同上 | 输入 | `CONDITIONAL-ELECTRICAL` | `AIN2→PB12` 位置映射与实物连续性；U2 保持空置。 |
| `PIN-LINE-3` | 五路循迹 | TCRT `OUT3` | `PA12` | GPIO input | H3 pin15 的 `AIN4` 网络 | 输入 | 同上 | 输入 | `CONDITIONAL-ELECTRICAL` | `AIN4→PA12` 位置映射与实物连续性；U3 保持空置。 |
| `PIN-LINE-4` | 五路循迹 | TCRT `OUT4` | `PA13` | GPIO input | H4 pin15 的 `BIN3` 网络 | 输入 | 同上 | 输入 | `CONDITIONAL-ELECTRICAL` | `BIN3→PA13` 位置映射与实物连续性；U3 保持空置。 |
| `PIN-LINE-5` | 五路循迹 | TCRT `OUT5` | `PB23` | GPIO input | H3 pin16 | 输入 | 同上 | 输入 | `CONDITIONAL-ELECTRICAL` | 实物连接器方向与通道左右次序确认。 |
| `PIN-START` | 人机接口 | 启动按键 | `PB21` | GPIO input | 板载按键；H2 pin8 | 输入，内部上拉 | 3.3 V，按下接地 | 输入上拉、低有效 | `FROZEN-RESOURCE` | 消抖和长短按属于固件参数。 |
| `PIN-STATUS` | 人机接口 | 状态 LED | `PB22` | GPIO output | 板载 LED；H1 pin8 | 输出 | 板载 2 kΩ 负载 | 低/熄灭 | `FROZEN-RESOURCE` | 当前 `.syscfg` 已占用。 |
| `PIN-BEEP` | 人机接口 | BEEP | `PB27` | GPIO output 候选 | H3 pin17 | 输出 | H13 另有 3.3 V/GND；负载未知 | MCU 复位期高阻；静音电平未知 | `BLOCKED` | 蜂鸣器有源/无源、驱动电流、极性和是否需要三极管。 |

### 五路 TCRT 候选池说明

`PB1/PB12/PA12/PA13/PB23` 均可从 H1/H3/H4 取线，且不占用本计划的 Timer、UART、I2C、SWD、VREF、晶振或板载 Flash Owner。它们仍是候选资源，不是已确认线束：

- `PB12/PA12/PA13` 借用拓展板原 TB6612 信号网络，前提是 U2/U3 保持空置且先做断电连续性核对；
- 五路采用定时节拍同步轮询，不默认启用 5 路高优先级 GPIO 中断；
- 最终 `OUT1`–`OUT5` 与车体左→右顺序，以逐通道遮挡测试为准。

## 4. 外设实例资源预算

| 外设实例 | Owner | 通道/引脚 | 共享周期/约束 | 状态 |
|---|---|---|---|---|
| `I2C0` | OLED | PA0 SDA、PA1 SCL | 全总线只能使用安全 3.3 V 上拉；角度编码器不在当前版本 | `CONDITIONAL` |
| `I2C1` | MPU6050 | PB2 SCL、PB3 SDA | GY 接口飞线；与 I2C0 故障隔离 | `CONDITIONAL` |
| `UART0` | 调试日志 | PA10 TX、PA11 RX | 保留板载 CH340E | `FROZEN-RESOURCE` |
| `UART1` | 视觉坐标 | PA8 TX、PA9 RX | 不占用 TIMA0 CCP0/1；新增无线候选不释放或替代该资源 | `CONDITIONAL` |
| `SPI0` | 改装无线低速数据/遥测候选 | PICO=PB17、SCLK=PB18、POCI=PB19、CS0=PB25；CE=PA25、IRQ=PA27 为独立 GPIO | 仅从开发板原始 U22 候选取线；不使用 H1–H4 或 SPI1/PB6–PB9；用途、模式、频率、默认偏置和电气规格未关闭 | `PINMUX-VERIFIED / PLAN-CONDITIONAL` |
| `TIMA0` | 双电机 PWM | CCP2=PA7、CCP3=PB13 | 两路共用同一 PWM 周期 | `PINMUX-VERIFIED / PLAN-CONDITIONAL` |
| `TIMG8` | 编码器1 QEI | CCP0=PA29、CCP1=PA30 | 器件唯一计划 QEI Owner | `PINMUX-VERIFIED / PLAN-CONDITIONAL` |
| `TIMA1` | 编码器2双 Capture | CCP0=PB4、CCP1=PB5 | 不是硬件 QEI；方向/计数由固件完成 | `PINMUX-VERIFIED / PLAN-CONDITIONAL` |
| `TIMG7` | RC 舵机 PWM | CCP0=PB15 | USART2 不再使用 | `PINMUX-VERIFIED / PLAN-CONDITIONAL` |

## 5. MCU 引脚唯一占用表

| MCU 引脚 | Owner | 模式 | 状态 |
|---|---|---|---|
| PA0 | OLED | I2C0 SDA | conditional |
| PA1 | OLED | I2C0 SCL | conditional |
| PB2 | MPU6050 | I2C1 SCL | conditional |
| PB3 | MPU6050 | I2C1 SDA | conditional |
| PB4 | 编码器2 A | TIMA1 CCP0 | conditional |
| PB5 | 编码器2 B | TIMA1 CCP1 | conditional |
| PA7 | 电机通道 A PWM | TIMA0 CCP2 | conditional |
| PA8 | 视觉 TX | UART1 TX | conditional |
| PA9 | 视觉 RX | UART1 RX | conditional |
| PA10 | 调试 TX | UART0 TX | reserved |
| PA11 | 调试 RX | UART0 RX | reserved |
| PA12 | TCRT OUT3 | GPIO input | conditional |
| PA25 | 无线候选 CE | GPIO output | conditional |
| PA27 | 无线候选 IRQ（低有效） | GPIO input / interrupt 候选 | conditional |
| PA13 | TCRT OUT4 | GPIO input | conditional |
| PB1 | TCRT OUT1 | GPIO input | conditional |
| PB12 | TCRT OUT2 | GPIO input | conditional |
| PB23 | TCRT OUT5 | GPIO input | conditional |
| PB13 | 电机通道 B PWM | TIMA0 CCP3 | conditional |
| PB14 | 电机通道 A 方向 | GPIO output | conditional |
| PB15 | 舵机 PWM | TIMG7 CCP0 | conditional |
| PB17 | 无线候选 MOSI | SPI0 PICO/MOSI | conditional |
| PB18 | 无线候选 SCK | SPI0 SCLK | conditional |
| PB19 | 无线候选 MISO | SPI0 POCI/MISO | conditional |
| PB21 | 启动按键 | GPIO input | frozen-resource |
| PB22 | 状态 LED | GPIO output | frozen-resource |
| PB24 | 电机通道 B 方向 | GPIO output | conditional |
| PB25 | 无线候选 CSN | SPI0 CS0 | conditional |
| PB27 | 蜂鸣器候选 | GPIO output | blocked |
| PA29 | 编码器1 A | TIMG8 CCP0 | conditional |
| PA30 | 编码器1 B | TIMG8 CCP1 | conditional |

未出现的引脚保持未分配；禁止资源不得因“空闲”而自动启用。

## 6. 冻结门槛

候选版升级为 `mspm0g3507-pin-plan-frozen-v1.0.md` 前必须满足：

1. 五路 TCRT5000 的输出电平、输出结构、有效极性、H1/H3/H4 取线连续性和左右通道次序关闭。
2. DRV8870 使用独立线束；双路模块 H3 的 Pin 1 观察面、AIN/BIN/EA/EB 逐针方向确认；U2/U3 保持空置。
3. 编码器 PPR/CPR、A/B 相、最高边沿频率和两路归属确认。
4. OLED、MPU6050 的供电、地址、上拉位置和总线电压确认；未来若加入铰点角度编码器，须通过新版本重新审查 I2C0 地址与线束。
5. 视觉 UART 的精确板型、电平、波特率、帧格式和失帧策略确认。
6. 改装无线模块必须关闭：精确型号/版本、排针观察面与 pin 1 标记、pin 1/2 的权威身份、电源范围/地/峰值电流/去耦、全部逻辑电平、SPI mode 与安全 SCK、CSN/CE 复位偏置、IRQ 输出结构/无效电平/上拉与轮询或中断策略、MISO 未选中三态行为、对端设备与包协议、正式比赛启停/禁遥控策略，以及 U22 取线点的原始引脚图复核和实物可达性。任何电源线不得在 pin 1/2 关闭前设计或连接。
7. 无线链路的用途只能在上述证据关闭后定义；除非未来新版 Pin Plan 另行评审，否则不得替代 `PA8/PA9 UART1` 视觉坐标链路。无论如何，它不能替代独立的摄像头视频发送、接收、显示、存储和回放链路。
8. 舵机型号、供电、脉宽、中位、方向、机械限位和上电安全行为确认。
9. Pin Plan、线束矩阵、MCU 唯一占用表和外设实例表完全一致，包括无线六根信号、U22 候选针号、SPI0 Owner 和 pin 1/2 的 `UNKNOWN / DO-NOT-CONNECT` 状态。
10. 用户明确批准冻结版；冻结不自动授权 `.syscfg`、接线或上电。

## 7. 来源

- `docs/extracted/tianmengxing-expansion-board-v2-epro/mspm0-expansion-net-matrix.md`
- `docs/reviewed/tianmengxing-expansion-board-v2-source-facts.md`
- `docs/reviewed/tianmengxing-expansion-board-v2-photo-review.md`
- `docs/reviewed/drv8870-dual-module-schematic-pdf-review.md`
- `docs/reviewed/tcrt5000-five-channel-photo-review.md`
- `docs/reviewed/zjy096i0400wg01-new-oled-module-review.md`
- `docs/hardware/board-pinout.md`
- 用户于 2026-07-29 提供的改装无线模块排针定义，仅作为 pin 3–8 名称与方向的证据
- `docs/source-pdf/tianmengxing-mspm0g3507-pinout.pdf`
- `docs/source-pdf/tianmengxing-mspm0g3507-schematic.pdf`
- `docs/extracted/tianmengxing-mspm0g3507-pinout/tianmengxing-mspm0g3507-pinout.md`
- `docs/extracted/tianmengxing-mspm0g3507-schematic/tianmengxing-mspm0g3507-schematic.md`
- `docs/hardware/modules/nrf24l01p.md`（只用于 nRF24L01+ 接口/吞吐量背景，不用于推导改装模块 pin 1/2、供电或精确型号）
- `docs/requirements.md`
- `docs/h-task-master-plan.md`
- 本地 `mspm0_sdk_2_11_00_07/source/ti/devices/msp/m0p/mspm0g350x.h`
- 本地 MSPM0G3507 `timg_qei_mode` 官方示例

## 8. 本轮未执行

- 未修改 `.syscfg`、源码、CCS 工程或生成文件；
- 未运行 SysConfig、构建、烧录、探针或串口；
- 未识别或连接无线模块 pin 1/2，未给模块供电，未搭建无线线束，未测 SPI/IRQ 波形或通信；
- 未认定该无线候选实现视觉坐标链路或满足视频发送、接收、显示、存储和回放要求；
- 未接线、上电、测量电平、驱动电机/舵机或进行整车测试。

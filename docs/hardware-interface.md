# 硬件接口与接线登记

> 初始化日期：2026-07-28  
> 本文档区分“板级已复核事实”“候选接口”和“最终接线”。当前没有已确认的外接模块接线。

## 1. 安全声明

- 原始 PDF 是开发板硬件事实源；OCR/提取文本仅供检索。
- 标为 `待确认` 或 `候选` 的内容不得用于接线、上电或修改 `.syscfg`。
- 外接模块必须先确认精确型号、供电、逻辑电平、最大电流、协议和实物版本。
- 电机/执行器供电不得从开发板 3V3 路径直接推定；独立供电时仍需确认共地和回流路径。
- 本轮未连接、未上电、未测量任何硬件。

## 2. 硬件事实来源

| 层级 | 路径 | 用途 |
|---|---|---|
| 原始资料 | `docs/source-pdf/tianmengxing-mspm0g3507-pinout.pdf` | 引脚分配图，1 页 |
| 原始资料 | `docs/source-pdf/tianmengxing-mspm0g3507-schematic.pdf` | 原理图，3 页 |
| 已复核摘要 | `docs/reviewed/tianmengxing-mspm0g3507-source-facts.md` | 带原始页码的高价值事实 |
| 项目索引 | `docs/hardware/README.md` | 资料用途和使用规则 |
| 工作记忆 | `docs/hardware/board-overview.md` | 板载资源概览 |
| 工作记忆 | `docs/hardware/board-pinout.md` | 引脚规则与接口建议 |
| 工作记忆 | `docs/hardware/board-power-debug.md` | 电源、VREF、SWD、BSL、UART0 |
| 工作记忆 | `docs/hardware/resource-conflicts.md` | 板载资源冲突 |
| 工作清单 | `docs/hardware/bringup-checklist.md` | 后续接线和上电前检查 |

## 3. 已确认的开发板资源

以下是资料层面的板级事实，不表示外设已接线或已在实物上验证。

| ID | 状态 | 资源 | 已复核事实 | 证据 |
|---|---|---|---|---|
| `HW-001` | `[已确认|资料复核]` | 板载 LED | `PB22` 驱动 LED，高电平点亮。 | 原理图 p1；`docs/reviewed/tianmengxing-mspm0g3507-source-facts.md` |
| `HW-002` | `[已确认|资料复核]` | 板载按键 | `PB21` 接地，使用时应按低电平有效理解。 | 原理图 p1；同上 |
| `HW-003` | `[已确认|资料复核]` | SWD | `PA19/SWDIO`、`PA20/SWCLK` 引至调试接口。 | 原理图 p3；同上 |
| `HW-004` | `[已确认|资料复核]` | BSL | `PA18/BSL` 有 47 kΩ 下拉，BSL 按键接 3V3。 | 原理图 p1；同上 |
| `HW-005` | `[已确认|资料复核]` | 时钟相关 | `PA2` 为 ROSC 网络；`PA3/PA4`、`PA5/PA6` 与板载晶体网络相关。 | 原理图 p1；`docs/hardware/board-overview.md` |
| `HW-006` | `[已确认|资料复核]` | 板载串口路径 | `PA10/U0TX`、`PA11/U0RX` 接 CH340E 调试/下载网络。 | 原理图 p3；已复核摘要 |
| `HW-007` | `[已确认|资料复核]` | SPI Flash/LCD | `PB6–PB9` 接板载 SPI Flash；`PB8/PB9` 同时出现在 LCD 接口。 | 原理图 p3；已复核摘要 |
| `HW-008` | `[已确认|资料复核]` | ADC 参考 | `PA23/VREF+` 有参考电压选择网络；`PA21/PA23` 未确认前不作为普通 GPIO。 | 原理图 p2；`docs/hardware/board-power-debug.md` |
| `HW-009` | `[已确认|资料复核]` | 电源路径 | USB-C 5V 经 F1 500 mA，EXT_3V3 经 F2 500 mA。 | 原理图 p2；已复核摘要 |

## 4. 保留和冲突资源

| 资源 | 初始规则 | 原因 |
|---|---|---|
| `PA19/PA20` | 保留 | SWD 调试/下载 |
| `PA18` | 避免普通功能 | BSL 启动网络 |
| `PA2–PA6` | 未复核具体时钟方案前保留 | ROSC/晶体网络 |
| `PA21/PA23` | 未确认 VREF 配置前禁用 | ADC 参考网络 |
| `PA10/PA11` | 优先保留给板载 UART0 | CH340E 路径；改作其他用途会产生冲突 |
| `PB6–PB9` | 分配前审查 | 板载 SPI Flash；PB8/PB9 还与 LCD 共享 |
| `PB21/PB22` | 识别为板载按键/LED | 避免与用户功能冲突 |

## 5. 外部模块 BOM

当前已收到部分模块资料，但尚未完成实物丝印、供电电平和最终接线核对；以下条目均不是已确认物理 BOM。

“灰度模块、霍尔编码器、OLED、MPU6050、蜂鸣器、电机驱动”等名称只表示现有资料讨论过的模块类别，不能视为实际 BOM。

| 模块 ID | 精确型号/版本 | 数量 | 供电 | 逻辑电平 | 最大电流 | datasheet | 状态 |
|---|---|---:|---|---|---:|---|---|
| 待登记 | 待确认 | — | 待确认 | 待确认 | 待确认 | 待提供 | `[待确认|无]` |

## 6. 候选接口池

| ID | 状态 | 候选方向 | 依据 | 实施前门槛 |
|---|---|---|---|---|
| `IF-001` | `[候选|资料复核]` | `PA0/I2C0-SDA`、`PA1/I2C0-SCL` 可作为 I2C 方向候选。 | `docs/hardware/board-pinout.md` 和原理图/引脚图建议 | 确认模块型号、电压、地址、速率、上拉、实际复用及 SysConfig 资源 |
| `IF-002` | `[候选|资料复核]` | `PA10/U0TX`、`PA11/U0RX` 可保留为板载 UART0 日志方向。 | `HW-006` | 确认日志协议、波特率、引脚占用和 CH340E 使用方式 |

候选项不是接线指令，也不是当前 `.syscfg` 的配置事实。

## 7. 已确认接线矩阵

**当前没有任何已确认的外接模块接线。**

| Interface ID | 模块及精确型号 | 模块引脚 | 供电/电平 | 协议参数 | MCU 实例/引脚 | 状态 | DEC | TEST |
|---|---|---|---|---|---|---|---|---|
| 暂无 | — | — | — | — | — | `[待确认|无]` | — | — |

## 8. 电源与逻辑电平预算

| 项目 | 状态 | 当前记录 |
|---|---|---|
| 开发板输入/保护路径 | `[已确认|资料复核]` | 见 `HW-009`；保护器件额定信息不能直接等同于可用外设预算。 |
| 外部模块总电流 | `[待确认|无]` | 已归档 MP1584EN 参数图：宣称最大输出 3 A，但未确认实物、设定值、带载能力或热；仍缺 BOM、datasheet 与测量。 |
| 电机/执行器电源 | `[待确认|无]` | 新增 D36A 两相步进驱动候选：手册示例接外部 12 V，最大电流拨码档 1.44 A；实际步进电机相电流、输入范围、保护、热与供电路径未确认。 |
| 共地与回流路径 | `[待确认|无]` | 需结合实际电源、驱动和接线确认。 |
| 逻辑电平转换 | `[待确认|无]` | 需逐模块核对 VIH/VIL/VOH/VOL。 |
| 去耦、上拉和保护 | `[待确认|无]` | 需结合模块板载电路和总线参数确认。 |

## 9. 接线确认门槛

接口进入“已确认接线矩阵”前必须同时满足：

1. 模块精确型号、实物版本和 datasheet 已登记。
2. 供电电压、逻辑电平、最大电流和方向已复核。
3. 协议实例、速率/模式、上拉或极性等参数已确定。
4. MCU 引脚已通过原理图、引脚图、资源冲突和实际排针位置复核。
5. 负责人通过 `DEC-*` 接受该分配。
6. 后续修改 `.syscfg` 或接线前已获得用户明确授权。

## 10. 新增接口模板

```text
Interface ID: IF-NNN
Module exact model/revision:
Module pin and direction:
Supply and logic level:
Maximum/current budget:
Protocol and parameters:
MCU peripheral instance and pin:
Physical header position:
Conclusion status:
Datasheet/source/page:
Wiring photo/diagram:
Related REQ/DEC/TASK/TEST:
Supersedes:
Reviewed by:
```

## 9. 模块资料索引（未确认接线）

| 模块 | 已提取资料 | 当前关键事实 | 阻塞项 |
|---|---|---|---|
| nRF24L01+ | `docs/hardware/modules/nrf24l01p.md` | 3.3 V SPI 无线 IC/模块候选，最高 2 Mbps 空口速率。 | 实物模块排针、用途、图传能力。 |
| HiWonder/AiBlock LineFollower_6CH V1.0 | `docs/hardware/modules/linefollower-6ch-i2c.md` | 5 V/85 mA，I²C 7 位地址 0x5C，六路数字状态和六路 16 位模拟值；原理图 I²C 上拉到 3.3 V。 | 实物版本、线序、总线上拉、I²C 速率、通道左右顺序、数字极性和阈值寄存器冲突。 |
| 原八路灰度 / 五路 TCRT5000 | 历史资料 | 已被 LineFollower_6CH 主循迹选择替代。 | 保留资料与既有接线事实，不用于新模块接线或固件。 |
| MPU6000A/MPU6050 | `docs/hardware/modules/mpu6000a.md` | `NOT FITTED / HISTORICAL OPTION`；PB2/PB3 已由 frozen v1.5 转给 OLED。 | 若未来恢复，重新确认接口、上拉、安装、振动和收益，不沿用旧共享总线假设。 |
| ZJY096I0400WG01 OLED | `docs/hardware/modules/oled-0.96in-zjy096i0400wg01-new-module-facts.md` | 实物 `GND/VDD/SCK/SDA`；frozen v1.5 经原 MPU/GY 接口独占 I2C1 PB3/PB2。 | GY pad1观察面、断电连续性、SCK/SDA上拉电压、实际地址0x3C/0x3D。 |
| DRV8870 双路 | `docs/hardware/modules/drv8870-dual.md` | 双电机控制、四路编码器输入、VIN/电流/接口资料。 | 实物板版本、电机参数、电池与电平。 |
| 天猛星拓展板 V2.0 | `docs/hardware/modules/tianmengxing-expansion-board-v2.md` | EPRO 静态资料与实物正面装配照片均已归档；可见双稳压、TB6612、OLED、按键等区域。 | 背面照片、板号、排针线序、电平与实际供电路径。 |
| MP1584EN 可调降压 | `docs/hardware/modules/mp1584en-adjustable-step-down.md` | 用户称 MP1584EN；参数图称 4.5–28 V 输入、0.8–20 V 可调输出、最大 3 A。 | 实物板型、输出设定/测量、负载、保护、热与接线。 |
| K230 目标跟踪 | docs/hardware/modules/k230-target-tracking.md | PA8/PA9 UART1 端点保持；K230 目标框经球心、杆轴投影和像素到毫米标定后，作为钢球位置外环的唯一测量。 | 板型/电平、完整帧、捕获时间、帧率/延迟、标定精度、异常帧策略及完整视频图传链路。 |
| MS42CG 编码器 | `docs/hardware/modules/ms42cg-encoder.md` | 3.3–5 V 供电，输出 A/B/PWM/Z；手册示例 A/B 1000 线、四倍频 4000 计数/圈。 | 实物线束、3.3 V 供电、A/B/PWM/Z 取线、MCU 捕获资源和最高边沿频率。 |
| D36A 双路步进驱动 | `docs/hardware/modules/d36a-dual-stepper-driver.md` | 两路 STEP/DIR/EN，外部 12 V 示例、4 线两相步进输出，细分/电流拨码共用。 | 实际步进电机型号/相电流、控制电平时序、供电保护、热与物理连接。 |

当前设计端点以 `docs/pin-plan/README.md` 指向的最高修订和配套 harness 为准；本文件登记职责与接口状态。在 `docs/hardware/module-questions.md` 的实物、电气问题关闭且另获授权前，不得写入 `.syscfg` 或实施接线。

## 10. 最终 Pin Plan v1.0 设计登记（历史记录，已被 v1.2 取代）

> **历史状态**：下表只记录 v1.0 曾冻结的设计，已被 v1.2 取代，不得作为当前 SysConfig、接线或施工依据。当前权威文件见第 12 节。

| Interface ID | 模块/接口 | MCU / 外设 | 物理路径 | 当前开放条件 |
|---|---|---|---|---|
| `IF-V1-I2C0` | OLED U8 | PA0/PA1，I2C0 | U8 pad4/pad3 | 供电、地址、3.3V 上拉与实物线序 |
| `IF-V1-I2C1` | MPU6050/GY | PB2/PB3，I2C1 | GY_SCL/GY_SDA | 模块版本、供电、3.3V 上拉与安装轴向 |
| `IF-V1-UART1` | K230 | PA8/PA9，UART1 | USART1 pin2/pin1 | 电平、波特率、帧格式、供电及完整图传链 |
| `IF-V1-LINE` | 五路 TCRT5000 | PB19/PB17/PA16/PA14/PB20 | H10 pin1–5；pin6–8 DNC | OUT 高低电平、输出结构、极性和左右顺序 |
| `IF-V1-DRV-LOGIC` | 双路 DRV8870 | PB14/PB12/PA7/PB24 | U2 三隔离、三桥接、一保留后接 H3 pin1/3/2/4 | 切线点、H3 观察面、默认态和真值台架验证 |
| `IF-V1-DRV-PWR` | DRV 与左右轮 | 无 MCU 引脚 | VIN/GND 与 AOUT/BOUT 独立重载线，绕过拓展板功率铜线 | 保险、线径、限流、温升、左右轮方向 |
| `IF-V1-ENC-R` | 右轮编码器 U6 | PB10/PB11，TIMG8 QEI | U6 pin4/pin3；pin1/6 DNC | 实物方向、PPR/CPR、相位和最高边沿频率 |
| `IF-V1-ENC-L` | 左轮编码器 U16 | PB4/PB5，TIMA1 双 Capture | U16 pin4/pin3；pin1/6 DNC | 软件正交策略、PPR/CPR、相位和频率 |
| `IF-V1-D36A` | D36A 通道1 | PA26/PA24/PB0 | H1.19/H1.18/H2.6 → ST1/DIR1/EN1 | 输入门限、时序、EN/ST 硬件偏置、供电/热/限位 |
| `IF-V1-MS42` | MS42CG V2 | PA12/PA13/PB26/PB23 | H3.15/H4.15/H4.16/H3.16 → A/B/PWM/Z | 六针观察面、3.3V 输出、PWM 周期、Z 脉宽和方向 |
| `IF-V1-BEEP` | H13 | PB27 GPIO | H13 pin2 | 仅低电流有源模块；无源方案另审 |
| `IF-V1-UART2-RSV` | USART2 | PB15/PB16 | 全口 DNC | future-reserved；当前 NO-SYSCFG |

### v1.0 DNC / 禁止接入

- U3、U12、H8；
- 拓展板 KEY1–KEY4、LED1–LED2，以及开发板 PB21/PB22 KEY/LED；
- U6/U16 pin1、pin6；
- H10 pin6–8；
- D36A 5V/ADC/第二通道；
- USART0 外接座、USART1 5V、USART2 全口；
- 旧无线 SPI 和 RC 舵机路线。

`empty.syscfg` 未按本历史表修改；本节仅供追溯，不授权接线或上电。
## 11. 无线 SPI0 v1.1 候选（已否决）

> 该节是对 Pin Plan v1.0 的局部候选修订，不覆盖 v1.0 的其他冻结项。详见 `docs/pin-plan/mspm0g3507-pin-plan-wireless-spi0-v1.1-candidate.md`。
>
> **否决原因**：用户确认 PB18 为 KEY3，不可用于 SPI0 SCLK；不得按本节接线或配置。

| Interface ID | 模块/接口 | MCU / 外设 | 物理路径 | 当前开放条件 |
|---|---|---|---|---|
| `IF-V1.1-RADIO-SPI` | 改装无线模块 pin3/5/6/8 | PB25/PB17/PB18/PB19，SPI0 CS0/PICO/SCLK/POCI | H10 pin6/pin2、U22 PB18 原始飞线、H10 pin1 | 模块 pin1/2、供电、电平、SPI mode、U22 观察面与飞线可达性 |
| `IF-V1.1-RADIO-CE` | 改装无线模块 pin4 CE | PB1 GPIO output | H1 pin6 | 默认低、模块逻辑电平与物理线束确认 |
| `IF-V1.1-RADIO-IRQ` | 改装无线模块 pin7 IRQ（低有效） | PB16 GPIO input / IRQ 候选 | H3 pin7 | 用户已释放 UART2 PB16；确认输出类型、上拉与无效电平 |
| `IF-V1.1-TCRT-REWIRE` | 五路 TCRT5000 | PA25/PA27/PA16/PA14/PB20 | H10 pin7/pin8/pin3/pin4/pin5 | 输出电平、极性、模块物理左右顺序和改线线束 |

禁止把无线模块 pin1/pin2 接到任何电源或地；v1.1 已否决，禁止按本候选修改 `.syscfg`、接线或上电。
## 12. Pin Plan v1.2 基础资源登记（由 v1.3/v1.4/v1.5 分层修订）

> **状态含义**：v1.2 是未被后续修订覆盖的基础资源；必须再依次应用 v1.3、v1.4 和当前 v1.5。当前索引见 `docs/pin-plan/README.md`；LineFollower/OLED/MPU/TCRT/U12/U8 的端点只以 frozen v1.5 与 v1.5 harness 为准。

| Interface ID | 模块/接口 | MCU / 外设 | 物理路径 | 当前开放条件 |
|---|---|---|---|---|
| `IF-V1.2-RADIO-SPI` | 改装无线 pin3/5/6/8 | PB25/PB17/PA12/PB19，SPI0 CS0/PICO/SCLK/POCI | H10 pin6/pin2、H3 pin15、H10 pin1 | 模块 pin1/pin2/供电、电平、SPI mode/频率和线束仍待关闭 |
| `IF-V1.2-RADIO-CE` | 无线 pin4 CE | PB1 GPIO | H1 pin6 | 默认低与模块逻辑确认 |
| `IF-V1.2-RADIO-IRQ` | 无线 pin7 IRQ | PB16 GPIO IRQ | H3 pin7 | 输出结构、上拉、无效电平和触发策略 |
| `IF-V1.2-TCRT` | 五路 TCRT5000 | PA25/PA27/PA16/PA14/PB20 | H10 pin7/pin8/pin3/pin4/pin5 | 改线、输出电平/极性和物理顺序 |
| `IF-V1.2-MS42` | MS42CG A/B/PWM/Z | PA29/PA30/PA13/PB23 | H4 pin17/H2 pin7/H4 pin15/H3 pin16 | Timer 捕获隔离验证、边沿频率、PWM 周期与线束 |

`PB18` 按用户现场陈述标记为 `KEY3 / FORBIDDEN-FOR-RADIO`。v1.2 冻结只确认设计资源，不授权修改 `.syscfg`、接线、供电、构建、烧录或硬件测试。

## 13. 传感器与执行器职责变更（当前方案）

| 链路 | 当前职责 | 明确不承担 | 状态/门槛 |
|---|---|---|---|
| K230 → UART1 PA8/PA9 | 目标框经过标定后输出钢球相对 O 点的有符号位置，作为球位置外环唯一测量。 | 不直接证明杆角、执行器位置或视频图传已合规。 | 完整协议、时间戳、帧率/延迟、毫米标定和异常帧策略待验证。 |
| MS42CG → PA29/PA30/PA13/PB23 | D36A 步进执行器的位置/角度内层反馈、回零/丢步监测候选。 | 不测钢球位置。 | 零位、方向、计数、PWM/Z 语义、几何映射与失步策略待验证。 |
| D36A → PA26/PA24/PB0 | 控制步进电机改变摆杆右端高度，从而实现定轴转动。 | 不提供编码器反馈，也不能保证没有失步。 | 步频、加速度、限位、失能、相电流与温升待验证。 |
| LineFollower_6CH → I2C0 PA28/PA31 | 小车红外循迹主反馈；读取 0x05 数字状态和 0x06 六路模拟值候选。 | 不测钢球位置；不使用原 TCRT 五根 GPIO。 | frozen v1.5设计已批准；通道顺序、极性、总线速率、200 Hz调度和异常恢复待验证。 |
| OLED → I2C1 PB3/PB2 | 本地时间、模式和故障显示；经原 MPU/GY 接口重排线束接入。 | 不参与控制闭环。 | 实物 `VDD/SCK` 按正电源/I²C SCL映射；地址、上拉和线束连续性待验证。 |
| MPU6050 | `NOT FITTED`。 | 不装摆杆、不装车体、不采样、不提供前馈。 | 若未来恢复，必须新建方案决策和 Pin Plan 修订。 |

本节反映 frozen v1.5 设计职责，但不授权修改 `.syscfg`、接线、上电、构建、烧录或运动测试。

## 14. LineFollower/OLED/MPU v1.5 冻结接口

| Interface ID | 模块 | MCU 资源 | 物理路径 | 状态/阻塞 |
|---|---|---|---|---|
| `IF-V1.5-LINE-I2C0` | HiWonder/AiBlock LineFollower_6CH V1.0 | PA31 I2C0 SCL、PA28 I2C0 SDA | U12 pin2 SCL、pin3 SDA、pin1 5V、pin4 GND；模块端按信号交叉 | FROZEN-DESIGN；未改 SysConfig、未制作线束、未上电 |
| `IF-V1.5-OLED-I2C1` | ZJY096I0400WG01 / GM009605V4.3 OLED | PB2 I2C1 SCL、PB3 I2C1 SDA | 原 MPU/GY pad3 SCL、pad4 SDA、pad1 5V、pad2 GND；OLED `VDD/SCK` 分别对应正电源/SCL | FROZEN-DESIGN；禁止直通四芯线，地址/上拉待验 |
| `IF-V1.5-MPU-DNC` | MPU6050 | 无 | 模块移除并绝缘保存；原接口改由 OLED 使用 | USER-APPROVED / NOT FITTED |
| `IF-V1.5-TCRT-DNC` | 原五路 TCRT5000 | PA25/PA27/PA16/PA14/PB20 释放为 DNC | 旧交叉线束退出 | FROZEN-DNC；不自动授权分配给其他功能 |
| `IF-V1.5-U8-DNC` | 旧 OLED U8 | PA0/PA1 释放为 DNC | U8 pad1～4 不接 | FROZEN-DNC；板载上拉网络仍存在 |

H10 pin1/PB19、pin2/PB17、pin6/PB25 的无线 SPI0 Owner 不变。当前权威文件是 `docs/pin-plan/mspm0g3507-pin-plan-frozen-v1.5.md` 与 `docs/pin-plan/mspm0g3507-adapter-harness-v1.5.md`。

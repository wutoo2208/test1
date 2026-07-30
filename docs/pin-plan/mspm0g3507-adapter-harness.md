# MSPM0G3507 模块转接线候选矩阵

> **状态**：`CANDIDATE`，2026-07-29。
> **配套文件**：`mspm0g3507-pin-plan-candidate.md`。
> **边界**：本文件用于线束设计审查，不是接线或上电指令。所有线束在进入冻结版前必须核对模块实物、连接器观察面、电平与连续性。改装无线模块仅确认排针 3–8 的名称和方向；排针 1/2 为 `UNKNOWN / DO-NOT-CONNECT`，六根候选信号只从开发板原始 `U22` 取线，不使用拓展板 H1–H4。

## 1. 统一观察与标识规则

1. MCU 引脚名以 `PAx/PBx` 为准；H1–H4/H3/H4 针号以拓展板 EPRO `PAD_NET` 表为准。
2. 模块连接器必须同时记录：元件面/焊接面、针 1 标记、排针面对观察者的方向。没有观察面时，不允许仅凭“从左到右”冻结。
3. 线束标签建议使用 `功能-通道-序号`，例如 `MOT-A-IN1`、`ENC-B-EB`、`LINE-3`。
4. 功率地和逻辑地最终共参考，但电机、舵机回流不得经过 MCU/传感器细线。
5. `5V`、`3.3V`、`OUT+`、`VBAT_IN` 是不同电源网络，禁止输出对输出并联或反向供电。
6. 先做断电连续性和短路检查，再进行任何逻辑域上电；本文件不授权该动作。
7. 无线线束的 MCU 端以开发板原理图/引脚图 `U22` 针号为候选依据，不套用 H1–H4 针号；冻结前必须分别记录 U22 与模块排针的观察面和 pin 1 标记。
8. 无线模块 pin 1/2 分别标记为 `RADIO-UNKNOWN-1`、`RADIO-UNKNOWN-2` 并单独绝缘，禁止凭线色、常见 nRF 排针或相邻针推断其为电源或地。
9. 每根无线信号标签必须同时写 U22 针号和 MCU `PAx/PBx` 名称；文本提取的 U22 候选针号仍须对照原始引脚图复核。

## 2. 禁止直插和禁止复用

| 项目 | 状态 | 原因 |
|---|---|---|
| DRV8870 直接插 U2/U3 | `FORBIDDEN` | U2/U3 是 TB6612 2×8 插座；先前“已经实测通过”信息已撤回。单路八针 DRV 针表与 U2/U3 不匹配。 |
| 双路 DRV8870 H3 当作功率接口 | `FORBIDDEN` | H3 仅含 AIN/BIN 和 EA/EB，不含 VIN、GND、OUT。 |
| H10 直接插五路 TCRT5000 | `FORBIDDEN` | H10 为 10 针八路接口，五路板为 7 针；OUT 高电平与逻辑极性未知。 |
| GY 接口按 I2C0 配置 | `FORBIDDEN` | GY 网络对应 PB2/PB3，本计划将其配置为 I2C1。 |
| SERVO1–SERVO4 当作 MCU 已连接网 | `FORBIDDEN` | EPRO 中仅连接舵机接口与控制排针，没有闭合到可确认 MCU GPIO。 |
| DRV 模块 3.3V/5V 回灌主板 | `FORBIDDEN` | 可能造成稳压器输出并联与反向供电。 |
| 推断无线模块 pin 1/2 为 VCC/GND | `FORBIDDEN` | 用户未提供这两针功能；常见 nRF 八针顺序不是改装模块证据。 |
| 在 pin 1/2 和电气规格关闭前给无线模块供电 | `FORBIDDEN` | 可能反接或施加错误电压；当前没有无线电源线束。 |
| 将无线六根候选信号接到拓展板 H1–H4 | `FORBIDDEN` | 所选 `PA25/PA27/PB17/PB18/PB19/PB25` 未出现在 H1–H4；仅候选从开发板原始 U22 引出。 |
| 无线模块复用 `SPI1/PB6`–`PB9` | `FORBIDDEN` | 该组归板载 SPI Flash 所有；`PB8/PB9` 还与 LCD 共享。 |
| 将低速无线候选登记为强制视频图传 | `FORBIDDEN` | 未证明满足摄像头视频发送、接收、显示、存储和回放要求。 |

## 3. 五路 TCRT5000 线束

模块照片只确认排针标签 `GND/5V/OUT1/OUT2/OUT3/OUT4/OUT5`；下表中的 OUT 次序暂按模块标签，不等于车体左→右顺序。

| 线号 | TCRT 模块端 | 候选 MCU/拓展板端 | 方向 | 中间处理 | 状态 |
|---|---|---|---|---|---|
| `LINE-PWR` | `5V` | 受验证的 5 V 传感器支路 | 电源 | 先确认板允许供电和纹波 | `BLOCKED-POWER` |
| `LINE-GND` | `GND` | 拓展板 GND/逻辑地 | 电源回路 | 不与电机回流串联 | `CONDITIONAL` |
| `LINE-1` | `OUT1` | H1 pin6 → `PB1` | 模块→MCU | 待定：直连、开漏 3.3V 上拉或 5V→3.3V 转换 | `CONDITIONAL-ELECTRICAL` |
| `LINE-2` | `OUT2` | H4 pin8 `AIN2` → `PB12` | 模块→MCU | 同上；U2 必须空置 | `CONDITIONAL-ELECTRICAL` |
| `LINE-3` | `OUT3` | H3 pin15 `AIN4` → `PA12` | 模块→MCU | 同上；U3 必须空置 | `CONDITIONAL-ELECTRICAL` |
| `LINE-4` | `OUT4` | H4 pin15 `BIN3` → `PA13` | 模块→MCU | 同上；U3 必须空置 | `CONDITIONAL-ELECTRICAL` |
| `LINE-5` | `OUT5` | H3 pin16 → `PB23` | 模块→MCU | 同上 | `CONDITIONAL-ELECTRICAL` |

冻结前测量/资料闭环：

- 计划供电下每个 OUT 的高/低电压；
- 推挽/开漏和板载上拉；
- 黑线/白底对应极性；
- OUT1–OUT5 对应物理传感器位置；
- 若电平转换反相，在线束表和固件逻辑中只反相一次。

## 4. 双路 DRV8870 逻辑/编码器线束

### 4.1 归档双路模块 H3 事实

H3 是 `2×4` 交错编号：

```text
观察面和 Pin 1 方向：冻结前由实物照片确认

1 AIN1    2 BIN1
3 AIN2    4 BIN2
5 EA1     6 EB2
7 EB1     8 EA2
```

上表来自已复核原理图，不代表实物排针观察方向已经确认。

### 4.2 候选逻辑线束

每个 DRV8870 通道用两个输入。候选控制策略为“一路 Timer PWM + 一路 GPIO 方向”，但最终 PWM 接 IN1 还是 IN2、方向位真值必须在模块版本和安全状态确认后冻结。

| 线号 | MCU/拓展板端 | DRV H3 候选端 | 功能 | 复位/上电状态 | 状态 |
|---|---|---|---|---|---|
| `MOT-A-PWM` | H1 pin5 → `PA7/TIMA0 CCP2` | H3 pin1 `AIN1` 候选 | A 路 PWM | MCU复位期高阻；DRV输入偏置未知 | `BLOCKED-DEFAULT-STATE` |
| `MOT-A-DIR` | H1 pin4 → `PB14/GPIO` | H3 pin3 `AIN2` 候选 | A 路方向 | MCU复位期高阻；DRV输入偏置未知 | `BLOCKED-DEFAULT-STATE` |
| `MOT-B-PWM` | H4 pin9 → `PB13/TIMA0 CCP3` | H3 pin2 `BIN1` 候选 | B 路 PWM | MCU复位期高阻；DRV输入偏置未知 | `BLOCKED-DEFAULT-STATE` |
| `MOT-B-DIR` | H1 pin17 → `PB24/GPIO` | H3 pin4 `BIN2` 候选 | B 路方向 | MCU复位期高阻；DRV输入偏置未知 | `BLOCKED-DEFAULT-STATE` |
| `ENC-A-A` | H4 pin17 → `PA29/TIMG8 PHA` | H3 pin5 `EA1` | 编码器 A1 | 输入 | `CONDITIONAL-PHASE` |
| `ENC-A-B` | H2 pin7 → `PA30/TIMG8 PHB` | H3 pin7 `EB1` | 编码器 B1 | 输入 | `CONDITIONAL-PHASE` |
| `ENC-B-A` | H4 pin4 `A2` → `PB4/TIMA1 CCP0` | H3 pin8 `EA2` | 编码器 A2 | 输入 | `CONDITIONAL-PHASE` |
| `ENC-B-B` | H4 pin5 `B2` → `PB5/TIMA1 CCP1` | H3 pin6 `EB2` | 编码器 B2 | 输入 | `CONDITIONAL-PHASE` |

### 4.3 功率线束（不经 H3）

| 线号 | 来源 | 目的 | 规则 | 状态 |
|---|---|---|---|---|
| `DRV-VIN` | 3S 电池受保护电机支路 | DRV `VIN_IN/VIN` | 独立保险/总开关；不经 MCU 3V3/USB 5V | `BLOCKED-POWER-TREE` |
| `DRV-GND` | 星形功率地 | DRV GND | 与 MCU 共参考但不让电机电流经过逻辑地细线 | `BLOCKED-POWER-TREE` |
| `MOTOR-A-1/2` | DRV `AOUT1/AOUT2` | 电机通道 A（车轮归属待定） | 通道、车轮归属与转向待架空低速测试确认 | `CONDITIONAL-MECHANICAL` |
| `MOTOR-B-1/2` | DRV `BOUT1/BOUT2` | 电机通道 B（车轮归属待定） | 同上 | `CONDITIONAL-MECHANICAL` |

### 4.4 安全约束

- DRV8870 没有独立 STBY/PWM 针；PWM 施加到 IN1/IN2 之一。
- 两输入低是候选的软件安全状态；MCU 复位期引脚为高阻，必须确认 DRV 输入自带下拉或增加合适的外部下拉后，才能保证上电无意外运动。实际滑行/休眠时序仍以最终 DRV8870 资料为准。
- 在方向切换前先将 PWM 归零，并设置经实测确定的死区，避免 H 桥瞬态冲击。
- 归档 DRV8870 使用 VREF/ISEN；`nFAULT` 属于其他器件/模块描述，不得写入本模块 Pin Plan，除非实物型号证明存在该输出。

## 5. MPU6050 飞线至 GY/I2C1

| 线号 | MPU/拓展板端 | MCU | 方向 | 状态 |
|---|---|---|---|---|
| `IMU-SCL` | `GY_SCL`；MPU接口 pin3；H3 pin8 | `PB2/I2C1 SCL` | MCU→IMU 开漏 | `CONDITIONAL-ELECTRICAL` |
| `IMU-SDA` | `GY_SDA`；MPU接口 pin4；H3 pin9 | `PB3/I2C1 SDA` | 双向开漏 | `CONDITIONAL-ELECTRICAL` |
| `IMU-GND` | MPU接口 pin2 GND | 逻辑地 | 回路 | `CONDITIONAL` |
| `IMU-VCC` | MPU接口 pin1 标为 5V | 待定供电 | 电源 | `BLOCKED` |

说明：

- “接口标 5V”不等于 SDA/SCL 可上拉到 5V；冻结前必须确认模块上拉电压。
- 推荐让 I2C1 总线逻辑域保持 3.3V。若模块载板只能在 5V 供电且把总线上拉到 5V，需移除/隔离上拉或加双向电平转换。
- IMU 放在上层/摆杆附近时，线束需柔软、短、固定并做应力释放。

## 6. OLED / I2C0

| 线号 | OLED 模块候选端 | 拓展板/MCU端 | 状态 |
|---|---|---|---|
| `OLED-GND` | pin1 GND | U8 pin1 GND | `CONDITIONAL-PIN-ORDER` |
| `OLED-VCC` | pin2 VDD/VCC | U8 pin2 5V（是否直接使用待确认） | `BLOCKED-ELECTRICAL` |
| `OLED-SCL` | pin3 SCK/SCL | U8 pin3 → `PA1/I2C0 SCL` | `CONDITIONAL-PIN-ORDER` |
| `OLED-SDA` | pin4 SDA | U8 pin4 → `PA0/I2C0 SDA` | `CONDITIONAL-PIN-ORDER` |

OLED 数据域上限按资料保留为 3.3V。铰点 I2C 角度编码器不属于当前 Pin Plan；未来若增加，必须通过新版 Pin Plan 重新审查 PA0/PA1 总线的地址、供电、线束和总上拉阻值。

## 7. 视觉 UART1

| 线号 | MCU/拓展板端 | 视觉模块端 | 状态 |
|---|---|---|---|
| `VISION-TX` | `PA8/UART1 TX`；USART1 TX 位置 | 模块 RX | `CONDITIONAL-PROTOCOL` |
| `VISION-RX` | `PA9/UART1 RX`；USART1 RX 位置 | 模块 TX | `CONDITIONAL-PROTOCOL` |
| `VISION-GND` | USART1 GND | 模块 GND | `CONDITIONAL` |
| `VISION-PWR` | 不默认使用 USART1 5V 针 | 视觉独立稳定电源 | `BLOCKED-POWER` |

冻结前必须有：模块精确型号、3.3V 电平、波特率、帧格式、时间戳/帧号、坐标有效标志、超时和失帧行为。

新增 `SPI0` 无线候选不释放或替代 `PA8/PA9 UART1`。只有获得精确模块、用途、协议、电平和延迟证据并发布新版 Pin Plan 后，才可重新评审 UART1；无论如何，低速无线链路不得登记为题面要求的实时视频图传。

## 8. SPI0 无线发送模块候选线束

### 8.1 用户提供的模块排针事实

| 模块 pin | 已知功能 | 模块方向 | 状态 |
|---:|---|---|---|
| 1 | `UNKNOWN` | 未知 | `DO-NOT-CONNECT` |
| 2 | `UNKNOWN` | 未知 | `DO-NOT-CONNECT` |
| 3 | `CSN` | 输入 | `USER-SUPPLIED / CONDITIONAL` |
| 4 | `CE` | 输入 | `USER-SUPPLIED / CONDITIONAL` |
| 5 | `MOSI` | 输入 | `USER-SUPPLIED / CONDITIONAL` |
| 6 | `SCK` | 输入 | `USER-SUPPLIED / CONDITIONAL` |
| 7 | `IRQ`，低有效 | 输出 | `USER-SUPPLIED / CONDITIONAL` |
| 8 | `MISO` | 输出 | `USER-SUPPLIED / CONDITIONAL` |

上述事实只确认 pin 3–8 的名称与方向；在模块排针观察面、pin 1 标记和针号方向确认前，pin 3–8 也不得实际接线。pin 1/2 不得根据 nRF24L01+ IC 或常见八针模块经验补全。

### 8.2 候选信号线束

| 线号 | MCU/原始排针候选端 | 模块端 | 方向 | 状态 |
|---|---|---|---|---|
| `RADIO-CSN` | `U22 pin7 → PB25/SPI0 CS0` | pin3 `CSN` | MCU→模块 | `PINMUX-VERIFIED / PLAN-CONDITIONAL` |
| `RADIO-CE` | `U22 pin5 → PA25/GPIO` | pin4 `CE` | MCU→模块 | `CONDITIONAL-DEFAULT-STATE` |
| `RADIO-MOSI` | `U22 pin15 → PB17/SPI0 PICO` | pin5 `MOSI` | MCU→模块 | `PINMUX-VERIFIED / PLAN-CONDITIONAL` |
| `RADIO-SCK` | `U22 pin16 → PB18/SPI0 SCLK` | pin6 `SCK` | MCU→模块 | `PINMUX-VERIFIED / PLAN-CONDITIONAL-PROTOCOL` |
| `RADIO-IRQ` | `U22 pin3 → PA27/GPIO input` | pin7 `IRQ`，低有效 | 模块→MCU | `CONDITIONAL-ELECTRICAL` |
| `RADIO-MISO` | `U22 pin17 → PB19/SPI0 POCI` | pin8 `MISO` | 模块→MCU | `PINMUX-VERIFIED / PLAN-CONDITIONAL` |

这里的 U22 针号来自开发板原理图/引脚图的静态复核，是冻结前待原始图观察面复核的候选取线点。该六线未由拓展板 H1–H4 引出，禁止虚构 H1–H4 针号。不建立 `RADIO-VCC` 或 `RADIO-GND` 行：pin 1/2 的身份与模块电气规格关闭前，电源线束无法设计。

### 8.3 电气与复位安全边界

- pin 1/2 保持断开、分别绝缘，不接 U22 `EXT_3V3`、`VBUS_5V`、拓展板 `3.3V/5V` 或外部稳压器。
- MCU 复位期候选输出为高阻。未来期望的无效状态是 `CSN=高`、`CE=低`，但外部偏置方向、阻值、电源轨和模块兼容性必须依据精确模块验证，当前不构成接线建议。
- `IRQ` 低有效不等于已知为开漏；其推挽/开漏结构、无效电压和上拉需求未知，当前不增加上拉。
- 不推断 `CSN` 为高时 MISO 必然三态；须由精确模块资料或后续隔离测试关闭。
- 本轮不选择 SPI mode、SCK 空闲极性、采样边沿或频率，也不决定 IRQ 采用轮询还是中断。
- 信号束不得拼接到 `PB6`–`PB9` 板载 Flash 网络；远离电机桥输出和高电流线。模块/天线精确形态确认后再审查天线净空、回流和去耦位置。

### 8.4 功能边界

当前状态为 `UNDEFINED / LOW-RATE-DATA-OR-TELEMETRY-CANDIDATE`：

- 不替代 `PA8/PA9 UART1` 视觉坐标链路；
- 比赛行驶过程中不得用于人工遥控；
- 不满足摄像头视频发送、接收、显示、存储和回放要求；
- 冻结前必须定义对端设备、用途、字段、更新率、超时、重传以及正式比赛启用/禁用策略。

## 9. 舵机 PB15 直接跳线

| 线号 | 来源 | 目的 | 状态 |
|---|---|---|---|
| `SERVO-SIG` | H3 pin6 → `PB15/TIMG7 CCP0` | 舵机控制排针目标信号针 / 目标舵机信号线 | `CONDITIONAL-PIN-ORDER` |
| `SERVO-V+` | 经验证的 `OUT+`/独立 BEC | 舵机正极 | `BLOCKED-POWER` |
| `SERVO-GND` | 舵机电源地 | 舵机 GND，并与 MCU 信号地共参考 | `BLOCKED-POWER` |

规则：

- USART2 因 PB15 被占用而保持不用；
- 初次测试不得装球，必要时先不接连杆；
- 冻结前确认精确舵机型号、线色/针序、额定电压、峰值电流、PWM 帧周期和安全脉宽；
- 舵机电源不经过 MCU 3V3/USB 5V，信号线旁伴随一根地线。

## 10. 线束布置建议

- TCRT 与编码器信号走低功率线束区；电机和舵机供电沿车体边缘走。
- 电机两根功率线成对绞合；编码器 A/B 与 GND 成组，避免和电机功率线长距离并行。
- I2C 飞线短而柔软，远离舵机电源和电机桥臂输出；必要时降低总线速率，但不先猜具体数值。
- UART1 只接 TX/RX/GND 时，避免误接接口上的 5V 针。
- SPI0 无线信号从开发板 U22 原始排针单独成束；在 pin 1/2 和电源/GND 身份关闭前不形成完整插头。CSN/CE 安全偏置、IRQ 回路和天线位置不得由线色或常见模块经验推断。
- 每个插头使用带锁连接器或热缩标签，禁止仅凭杜邦线颜色识别功能。

## 11. 冻结前逐线检查单

1. 模块精确型号与连接器观察面已拍照。
2. 每根信号仅有一个 MCU Owner，没有多路外部驱动。
3. U2/U3 空置，DRV 使用独立线束。
4. TCRT 5 路经安全电平处理后才到 MCU。
5. I2C0/I2C1 的上拉均不超过 3.3V 逻辑域。
6. UART TX/RX 已交叉，空闲电平安全。
7. 舵机信号、电源、地线序无误，OUT+ 电压已独立确认。
8. VIN、5V、3.3V、OUT+ 不发生输出并联或反灌。
9. 电机/舵机高电流回流不经过 MCU 和传感器地细线。
10. 无线模块精确型号/版本、模块观察面、pin 1 标记已由照片或权威图确认；pin 1/2 在任何供电前已被权威识别，未按常见模块经验推断。
11. 原始引脚图已核对 `U22 pin3/5/7/15/16/17` 分别对应 `PA27/PA25/PB25/PB17/PB18/PB19`，且所有无线信号均明确为 U22 取线而非 H1–H4。
12. SPI mode 与安全频率、CSN/CE 复位偏置、IRQ 输出结构/上拉、MISO 未选中行为均已关闭。
13. 无线用途、对端和包协议已定义；`PB6`–`PB9` 仍仅归板载 Flash，`PA8/PA9` 仍归视觉 UART1，独立视频发送/接收/显示/存储/回放链路仍然存在。
14. 两份文档中 pin 1/2 始终为 `UNKNOWN / DO-NOT-CONNECT`，没有无线 VCC/GND 线束行，直到上述证据关闭并发布新版。
15. 所有“已撤回/未实测”信息没有被写成已验证事实。

## 12. 来源与证据边界

- 用户于 2026-07-29 提供的改装无线模块排针定义：仅证明 pin 3–8 的名称和方向。
- `docs/source-pdf/tianmengxing-mspm0g3507-pinout.pdf` 与 `docs/source-pdf/tianmengxing-mspm0g3507-schematic.pdf`：U22 候选取线点；接线前仍需用原始图确认观察面。
- `docs/extracted/tianmengxing-mspm0g3507-pinout/tianmengxing-mspm0g3507-pinout.md` 与 `docs/extracted/tianmengxing-mspm0g3507-schematic/tianmengxing-mspm0g3507-schematic.md`：静态文本辅助，不单独作为接线观察面证据。
- `docs/extracted/tianmengxing-expansion-board-v2-epro/mspm0-expansion-net-matrix.md`：证明 H1–H4 网络表未引出所选六线。
- 本地 MSPM0 SDK `mspm0g350x.h`：SPI0 PinMux 静态核验。
- `docs/hardware/modules/nrf24l01p.md`：仅用于 nRF24L01+ 接口和吞吐量背景，不用于推断改装模块 pin 1/2、供电或精确型号。
- `docs/requirements.md` 与 `docs/h-task-master-plan.md`：视频链路和禁止人工遥控的边界。

## 13. 本轮未执行

未探测或识别无线模块 pin 1/2，未制作无线线束，未连接无线电源或地，未接线、未测连续性/短路、未上电、未调压、未测 SPI/IRQ 波形或通信；未运行 SysConfig、未修改源码、未构建、未烧录，也未驱动电机或舵机，未认定无线候选满足视频图传要求。

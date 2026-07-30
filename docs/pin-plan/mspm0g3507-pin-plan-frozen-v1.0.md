# MSPM0G3507 最终 Pin Plan v1.0

> **状态**：`FROZEN-DESIGN / USER-ACCEPTED / NOT WIRED`。  
> **冻结日期**：2026-07-30。  
> **适用硬件**：天猛星 MSPM0G3507 LQFP-64 + 天猛星拓展板 V2 + 双路 DRV8870 + U6/U16 车轮编码器 + 五路 TCRT5000 + OLED + MPU6050 + K230 + D36A 单通道 + MS42CG V2。  
> **工具基线**：MSPM0 SDK `2.11.00.07`；当前 `empty.syscfg` 尚未按本计划配置，只保留历史 PB22 LED 空工程。  
> **证据等级**：用户决定 + 模块资料/EPRO/开发板原理图复核 + 本地 SDK IOMUX 静态核验；不是接线、SysConfig、构建或硬件实测证据。

## 1. 冻结的拓扑决定

1. 优先使用现有模块接口，避免无必要飞线。
2. U2 不插 TB6612/DRV；在 U2 侧隔离 `PB9/PB7/PB6` 冲突支路，再用 `PB14/PA7/PB24` 重建 DRV 四输入，`PB12` 保留。
3. 板载 Flash 保留，不能为了电机控制重用 PB6–PB9。
4. DRV VIN/GND/AOUT/BOUT 全部绕过拓展板功率铜线；右/左车轮电机直接接 DRV A/B 输出。
5. U6/U16 仅传编码器 3.3V/GND/A/B，两根 AO/BO 功率针 DNC。
6. K230 固定 UART1；UART0 留 CH340；UART2 未来保留但当前不配置、不接线。
7. MS42CG V2 + D36A 单通道取代 RC 舵机，使用 A/B/PWM/Z 完整反馈。
8. U3、U12、H8、扩展板 KEY/LED、开发板 KEY/LED 不使用。
9. 无线 SPI 方案延期，不占用或预留任何 MCU 引脚。

## 2. MCU 唯一 Owner 表

| MCU 引脚 | 唯一 Owner | 外设/模式 | 物理路径 | 状态 |
|---|---|---|---|---|
| PA0 | OLED SDA | I2C0 SDA | U8 pad4 | FROZEN |
| PA1 | OLED SCL | I2C0 SCL | U8 pad3 | FROZEN |
| PA7 | 左轮/B `BIN1` | TIMA0 CCP2 PWM | U2 pad10→改线 pad12 | FROZEN |
| PA8 | K230 RX 的 MCU TX | UART1 TX | USART1 pin2 | FROZEN |
| PA9 | K230 TX 的 MCU RX | UART1 RX | USART1 pin1 | FROZEN |
| PA10 | CH340 日志 TX | UART0 TX | 板载 CH340 | FROZEN |
| PA11 | CH340 日志 RX | UART0 RX | 板载 CH340 | FROZEN |
| PA12 | MS42CG A | TIMG0 CCP0 Capture | H3 pin15/AIN4 | FROZEN |
| PA13 | MS42CG B | TIMG0 CCP1 Capture | H4 pin15/BIN3 | FROZEN |
| PA14 | TCRT OUT4 | GPIO input | H10 pin4 | FROZEN |
| PA16 | TCRT OUT3 | GPIO input | H10 pin3 | FROZEN |
| PA24 | D36A DIR1 | GPIO output | H1 pin18 | FROZEN |
| PA26 | D36A ST1 | TIMG7 CCP0 | H1 pin19 | FROZEN |
| PB0 | D36A EN1 | GPIO output，低休眠 | H2 pin6 | FROZEN |
| PB2 | MPU6050 SCL | I2C1 SCL | GY_SCL | FROZEN |
| PB3 | MPU6050 SDA | I2C1 SDA | GY_SDA | FROZEN |
| PB4 | 左轮 A2 | TIMA1 CCP0 Capture | U16 pin4 | FROZEN |
| PB5 | 左轮 B2 | TIMA1 CCP1 Capture | U16 pin3 | FROZEN |
| PB10 | 右轮 A1 | TIMG8 CCP0/PHA | U6 pin4 | FROZEN |
| PB11 | 右轮 B1 | TIMG8 CCP1/PHB | U6 pin3 | FROZEN |
| PB12 | 右轮/A `AIN2` | GPIO output | U2 pad15 | FROZEN |
| PB14 | 右轮/A `AIN1` | TIMA0 CCP0 PWM | U2 pad16→改线 pad14 | FROZEN |
| PB17 | TCRT OUT2 | GPIO input | H10 pin2 | FROZEN |
| PB19 | TCRT OUT1 | GPIO input | H10 pin1 | FROZEN |
| PB20 | TCRT OUT5 | GPIO input | H10 pin5 | FROZEN |
| PB23 | MS42CG Z | GPIO IRQ input | H3 pin16 | FROZEN |
| PB24 | 左轮/B `BIN2` | GPIO output | H1 pin17→改线 U2 pad11 | FROZEN |
| PB26 | MS42CG PWM | TIMG6 CCP0 combined capture | H4 pin16/AIN3 | FROZEN |
| PB27 | 有源蜂鸣器 BEEP | GPIO output | H13 pin2 | CONDITIONAL-MODULE |

未列出的引脚保持未分配；不得因“空闲”在 SysConfig 中自动启用。

## 3. 外设实例 Owner

| 外设实例 | 唯一 Owner | 配置契约 |
|---|---|---|
| I2C0 | OLED | PA0 SDA、PA1 SCL；全总线 3.3V 上拉 |
| I2C1 | MPU6050 | PB2 SCL、PB3 SDA；全总线 3.3V 上拉 |
| UART0 | CH340 日志 | PA10 TX、PA11 RX；USART0 外接座 DNC |
| UART1 | K230 | PA8 TX、PA9 RX；K230 独立供电 |
| UART2 | 未来保留 | PB15/PB16；当前 NO-SYSCFG、DNC |
| TIMA0 | 两轮 DRV PWM | 单实例、共周期；CCP0=PB14、CCP2=PA7 |
| TIMG8 | 右轮编码器 | PB10/PB11 两相硬件 QEI |
| TIMA1 | 左轮编码器 | 单 Capture 实例、PB4/PB5 双通道；软件正交 |
| TIMG7 | D36A STEP | CCP0=PA26，独占可变步频输出 |
| TIMG0 | MS42CG A/B | 单 Capture 实例、PA12/PA13 双通道；软件正交 |
| TIMG6 | MS42CG PWM | PB26/CCP0 单输入 combined capture；整个实例独占 |
| GPIO IRQ | MS42CG Z | PB23；边沿/脉宽待实测 |

TIMA1/TIMG0 均不能建立两个分别占有同一 Timer 的 Capture 实例。TIMG6 的 combined capture 即使只外接 CCP0，也禁止把 CCP1 分给其他功能。

## 4. U2 改线与 DRV 控制

| DRV 输入 | MCU Owner | U2 最终焊盘 | 修改 |
|---|---|---:|---|
| AIN1（右轮 PWM） | PB14/TIMA0 CCP0 | pad14 | 隔离原 PB9，pad16→pad14 |
| AIN2（右轮第二输入） | PB12/GPIO | pad15 | 保留原路径 |
| BIN1（左轮 PWM） | PA7/TIMA0 CCP2 | pad12 | 隔离原 PB7，pad10→pad12 |
| BIN2（左轮第二输入） | PB24/GPIO | pad11 | 隔离原 PB6，H1.17→pad11 |

U2 只作逻辑汇接，pad1–9、pad13 DNC。PB6/PB7/PB9 继续由板载 Flash 独占；隔离仅发生在拓展板 U2 支路。

DRV H3：`1 AIN1 / 2 BIN1 / 3 AIN2 / 4 BIN2 / 5 EA1 / 6 EB2 / 7 EB1 / 8 EA2`；本项目 H3 pin5–8 DNC。

DRV 两输入低为候选停止态，但“PWM=0”不能无条件等同安全：固件必须按 IN1/IN2 真值建立有符号占空比，换向时先双低再等待死区。

## 5. 逐接口冻结

### 5.1 H10 五路

| H10 | MCU | 状态 |
|---:|---|---|
| 1 OUT_1 | PB19 | TCRT OUT1 |
| 2 OUT_2 | PB17 | TCRT OUT2 |
| 3 OUT_3 | PA16 | TCRT OUT3 |
| 4 OUT_4 | PA14 | TCRT OUT4 |
| 5 OUT_5 | PB20 | TCRT OUT5 |
| 6–8 | — | DNC |
| 9 5V | — | 供电验证后使用 |
| 10 GND | — | 逻辑地 |

需要 7→10 有方向适配线束。电平处理不会改变 MCU Owner，但必须在接线前确认。

### 5.2 U6/U16

- U6：pin2 GND、pin3 PB11/B1、pin4 PB10/A1、pin5 3.3V；pin1/pin6 DNC。
- U16：pin2 GND、pin3 PB5/B2、pin4 PB4/A2、pin5 3.3V；pin1/pin6 DNC。
- 车轮功率线不进入 U6/U16。

### 5.3 D36A

- ST1=PA26/H1.19；DIR1=PA24/H1.18；EN1=PB0/H2.6；另接逻辑 GND。
- EN1 低休眠；必须增加或确认外部下拉，保证 MCU 复位期间仍为低。
- ST1 需要复位期低态/无毛刺；D36A 5V、ADC、第二通道均 DNC。
- D36A A+/A-/B+/B- 只接经资料或断电电阻测量确认的两相步进绕组，不接 MS42CG 编码器 A/B/PWM/Z。实际电机绕组针号、线序、中心抽头结构和相电流仍是上电前阻塞项；本版本不冻结这些功率端点。

### 5.4 MS42CG V2

手册插座视角：`GND | Z | PWM | B | A | VCC`。

- VCC=3.3V；A=PA12；B=PA13；PWM=PB26；Z=PB23；GND=逻辑地。
- 不直插 U6/U16，不接 D36A 控制头，不改 SPI 模式。
- A/B 为 1000 线，×4 软件计数候选为 4000 count/rev；方向符号需台架标定。

### 5.5 UART/I2C/BEEP

- K230 TX→USART1 pin1/PA9；K230 RX←pin2/PA8；pin3 GND；pin4 5V DNC。
- USART0 外接座 DNC，PA10/PA11 仅归 CH340。
- USART2 全口 DNC，PB15/PB16 只作未来保留。
- U8：pad3 PA1/SCL、pad4 PA0/SDA；供电和上拉待电气核验。
- MPU6050/GY：PB2 SCL、PB3 SDA；“接口标 5V”不代表数据线允许 5V 上拉。
- H13：PB27 仅用于确认过的低电流有源蜂鸣器；若实物为无源，H13 保持 DNC 并发布新版资源评审。

## 6. 禁止和未来保留

| 资源/接口 | 状态 | 原因 |
|---|---|---|
| PA2–PA6 | FORBIDDEN | ROSC/晶体 |
| PA18 | FORBIDDEN | BSL |
| PA19/PA20 | RESERVED | SWD |
| PA21/PA23 | FORBIDDEN | VREF |
| PB6–PB9 | FLASH-OWNER | 板载 SPI Flash；U2 支路必须隔离 |
| PB15/PB16 | FUTURE-RESERVED | USART2，当前不配置、不接线 |
| PB21/PB22 | DNC | 开发板 KEY/LED 不使用 |
| KEY1–KEY4/LED1–LED2 | DNC | 拓展板 KEY/LED 不使用；KEY1/2 还触及 VREF |
| U3/U12/H8 | DNC | 当前不用；H8 与 PB10/PB11/PB14/PB26 冲突 |
| 无线 SPI | DEFERRED | 不保留引脚、不配置、不接线 |
| RC 舵机/SERVO1–4 | DEFERRED | 已被 D36A+MS42CG 路线取代 |

## 7. 电源、回流与安全默认态

- DRV 和 D36A 使用受保护功率支路；MCU 3.3V/USB 5V 不承载电机功率。
- DRV AOUT/BOUT 到车轮使用短粗线；D36A 到步进绕组使用匹配线径。
- DRV、D36A、MS42CG、MCU 共参考，但大电流回流不得经过传感器/MCU 细地线。
- D36A 5V 是输出，必须 DNC；任何稳压输出不得并联。
- 上电/复位默认：DRV 四输入低、D36A EN1 低、ST1 无脉冲、所有 Timer 输出禁用。
- 仅靠初始化完成后的 GPIO 状态不足以保证复位期安全；EN/ST 和必要的 DRV 输入需硬件偏置及实测确认。

## 8. 后续 SysConfig 契约

本版本不修改 `.syscfg`。未来获得单独授权后：

1. 只编辑 `empty.syscfg`，保留 device/package/SDK/SysConfig 元数据和 SWD；
2. 移除历史 PB22 LED Owner；
3. 建立本文件中的 I2C0/I2C1、UART0/UART1、TIMA0 PWM、TIMG8 QEI、TIMA1 双输入 Capture、TIMG7 STEP、TIMG0 双输入 Capture、TIMG6 combined capture、GPIO/IRQ；
4. 不建立 UART2、无线 SPI、RC 舵机、KEY/LED、U3/U12/H8；
5. 不使用 `$assignAllowConflicts`；不手改 `Debug/**` 或生成文件；
6. 在隔离输出目录运行 SysConfig，逐项核对 PINCM、实例、IRQ、生成名称和所有 warning。

## 9. 接线前验证门槛

- 确认所有连接器观察面和 Pin 1；
- 完成 U2 改线前后连续性表，证明冲突支路断开且 Flash 原路径完好；
- 验证 H10 输出高低电平、结构、极性和左右顺序；
- 验证 I2C0/I2C1 上拉均为 3.3V；
- 验证 K230 UART 电平、协议和供电；
- 验证 D36A 3.3V 输入门限、STEP 时序、EN 默认态、细分/电流、供电和散热；
- 验证 MS42CG 六针观察面、3.3V 输出、PWM 周期和 Z 脉宽；
- 确认 BEEP 类型；
- 所有 DNC 针分别绝缘。

## 10. 变更控制

任何 MCU Pin、Timer Owner、U2 切线点、接口 DNC 或功率路径变化，必须发布 `frozen-v1.1` 或 `v2.0` 并注明 `Supersedes`。不得静默修改本版本后继续施工。

**本版本未执行**：`.syscfg`、源码、SysConfig、构建、烧录、探针、串口、PCB 切线、接线、连续性测量、上电或运动测试。
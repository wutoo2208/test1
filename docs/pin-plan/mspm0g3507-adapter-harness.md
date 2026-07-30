# MSPM0G3507 最终转接线与改线矩阵（v1.0 历史版）

> **版本**：配套 `mspm0g3507-pin-plan-frozen-v1.0.md`，2026-07-30。
> **状态**：`SUPERSEDED BY mspm0g3507-adapter-harness-v1.2.md / HISTORICAL / NOT FOR CURRENT USE`。
> **禁止**：本文件不得用于当前接线、线束制作或施工；所有模块接线以 v1.2 最新版为准。
> **边界**：本文冻结端点和 DNC，不代表已切线、已接线或已上电。

## 1. 统一施工规则

1. 每个插头在施工记录中同时拍摄：元件面、焊接面、插接面、Pin 1 标记；严禁只写“从左到右”。
2. `DNC` 表示不连接并分别绝缘，不允许悬空裸露后与相邻针碰触。
3. VIN、5V、3.3V、OUT+、D36A 5V 输出互不并联；所有输出对输出连接均禁止。
4. 电机和步进电机功率线走重载路径；MCU/编码器/I2C/UART 只走逻辑路径。功率地与逻辑地在明确的星形点共参考。
5. PCB 切线和连续性检查前必须断开电池、USB、D36A、DRV 和全部外设，并确认电容已放电。

## 2. H10 五路 TCRT5000：7→10 适配

H10 已经通过拓展板 80-pin 网络闭合到 MCU，不再二次飞线至 U22。

| TCRT 端 | H10 | MCU | 状态/处理 |
|---|---:|---|---|
| OUT1 | 1 `OUT_1` | PB19 | 电平验证后接入 |
| OUT2 | 2 `OUT_2` | PB17 | 同上 |
| OUT3 | 3 `OUT_3` | PA16 | 同上 |
| OUT4 | 4 `OUT_4` | PA14 | 同上 |
| OUT5 | 5 `OUT_5` | PB20 | 同上 |
| — | 6–8 `OUT_6..8` | — | DNC |
| 5V | 9 | 传感器 5 V | 供电批准后连接 |
| GND | 10 | 逻辑地 | 不串联电机回流 |

冻结 MCU 映射不等于允许直接输入。接线前仍需确认 OUT 为推挽/开漏、高电平、电平极性和传感器从车体左到右顺序；如需电平转换，将器件和是否反相写入线束标签。

## 3. U2 三隔离、三桥接、一保留

U2 不插 TB6612 或 DRV8870 模块，只作为改线后的四路逻辑汇接焊盘区。

### 3.1 修改前记录

| 目标焊盘 | 原网络/来源 | 风险 |
|---:|---|---|
| U2 pad14 `AIN1` | PB9 | 板载 Flash SCLK 冲突 |
| U2 pad12 `BIN1` | PB7 | 板载 Flash POCI 冲突 |
| U2 pad11 `BIN2` | PB6 | 板载 Flash CS0 冲突 |
| U2 pad15 `AIN2` | PB12 | 可保留 |
| U2 pad16 | PB14 | 新右轮 PWM 源 |
| U2 pad10 | PA7 | 新左轮 PWM 源 |

### 3.2 隔离与桥接

“隔离”是切断原 MCU 冲突网络到目标 U2 焊盘的支路，同时保留 U2 目标焊盘本身；不得切断开发板 MCU 到板载 Flash 的原连接。

| 操作 | 修改后结果 |
|---|---|
| 隔离 PB9→U2 pad14；桥接 pad16→pad14 | pad14=`PB14/TIMA0 CCP0/AIN1` |
| 保留 PB12→U2 pad15 | pad15=`PB12/GPIO/AIN2` |
| 隔离 PB7→U2 pad12；桥接 pad10→pad12 | pad12=`PA7/TIMA0 CCP2/BIN1` |
| 隔离 PB6→U2 pad11；桥接 H1.17→pad11 | pad11=`PB24/GPIO/BIN2` |

U2 pad1–9、pad13 全部 DNC，不用于 DRV VIN、GND、5V、AO/BO 或车轮电机输出。

### 3.3 修改后断电验收

- pad14↔PB9、pad12↔PB7、pad11↔PB6 不得导通；
- pad14↔PB14、pad12↔PA7、pad11↔PB24、pad15↔PB12 应低阻；
- MCU↔板载 Flash 的 PB6/PB7/PB9 原路径必须保持；
- U2 目标焊盘不得短接 5V、VBAT_IN、AO/BO 或相邻焊盘。

具体阻值以表笔、走线和原理图解释为准，不把电容充电造成的瞬态蜂鸣当作短路。

## 4. U2→DRV8870 H3 逻辑线

H3 按 `2×4` 奇偶交错编号；实物 Pin 1 观察面仍须确认。

| U2 改线后端 | DRV H3 | 功能 |
|---|---:|---|
| pad14 / PB14 | 1 `AIN1` | 右轮 A PWM |
| pad15 / PB12 | 3 `AIN2` | 右轮 A 第二输入 |
| pad12 / PA7 | 2 `BIN1` | 左轮 B PWM |
| pad11 / PB24 | 4 `BIN2` | 左轮 B 第二输入 |
| — | 5–8 `EA/EB` | DNC；编码器不经过 H3 |

DRV 控制程序后续必须遵守 IN1/IN2 真值表，停车/换向显式双低并加入死区，不能把两方向都套同一 PWM 极性。

## 5. DRV 和车轮功率旁路

| 来源 | 目的 | 线束要求 |
|---|---|---|
| 受保护 3S 电机支路 | DRV VIN/GND | 保险、总开关、足够线径；GND 到星形功率地 |
| DRV AOUT1/AOUT2 | 右轮电机 | 短粗双线，绕过 U2/U6 |
| DRV BOUT1/BOUT2 | 左轮电机 | 短粗双线，绕过 U2/U16 |

DRV 的 3.3V/5V 输出不接开发板或拓展板电源；U6/U16 的 AO/BO 针不接电机功率线。

## 6. U6/U16 车轮编码器四线

| 接口 | Pin | 功能 | MCU | 状态 |
|---|---:|---|---|---|
| U6 右轮 | 1 AO2 | 旧电机输出 | — | DNC |
| U6 | 2 GND | 编码器地 | GND | CONNECT |
| U6 | 3 B1 | 右轮 B | PB11/TIMG8 CCP1 | CONNECT |
| U6 | 4 A1 | 右轮 A | PB10/TIMG8 CCP0 | CONNECT |
| U6 | 5 3.3V | 编码器电源 | 3.3V | 电源预算后 CONNECT |
| U6 | 6 AO1 | 旧电机输出 | — | DNC |
| U16 左轮 | 1 BO2 | 旧电机输出 | — | DNC |
| U16 | 2 GND | 编码器地 | GND | CONNECT |
| U16 | 3 B2 | 左轮 B | PB5/TIMA1 CCP1 | CONNECT |
| U16 | 4 A2 | 左轮 A | PB4/TIMA1 CCP0 | CONNECT |
| U16 | 5 3.3V | 编码器电源 | 3.3V | 电源预算后 CONNECT |
| U16 | 6 BO1 | 旧电机输出 | — | DNC |

若现有车轮线束为六针一体插头，必须制作只接 2–5 的适配线束，不能把两根车轮功率线带回 U6/U16。

## 7. D36A 单通道控制

| D36A 控制头 | MCU/拓展板端 | 状态 |
|---|---|---|
| ST1 | H1.19 → PA26/TIMG7 CCP0 | CONNECT；上升沿 STEP |
| DIR1 | H1.18 → PA24/GPIO | CONNECT |
| EN1 | H2.6 → PB0/GPIO | CONNECT；低休眠；要求外部下拉 |
| GND | 独立逻辑参考线 | CONNECT |
| 5V | — | DNC；D36A 输出，禁止反灌 |
| ADC | — | DNC |
| ST2/DIR2/EN2 | — | DNC |
| 第二路电机输出 | — | DNC |

控制头必须以 D36A 实物背面丝印和 Pin 1/列方向复核，不能从正面镜像推断。复位、烧录或 SWD 暂停时必须保持 EN1 低、ST1 无有效上升沿；仅靠 SysConfig 初始化值不足以保证复位期安全。

D36A 的 A+/A-/B+/B- 只可连接到经精确电机资料或断电电阻测量确认的两相绕组。当前不冻结绕组针号、线色、中心抽头结构或 A/B 相顺序；确认前所有功率端分别绝缘。相电流、细分、电源和机械限位也必须在上电前单独关闭。

## 8. MS42CG V2 编码器六线

按手册所示**编码器插座对接面**视角：

```text
GND | Z | PWM | B | A | VCC
```

线束插头背面会镜像，必须按实际卡扣和连续性建立针号表。

| MS42CG | MCU/拓展板端 | 模式 |
|---|---|---|
| VCC | 已核定 3.3V | 只用 3.3V，不用 5V |
| A | H3.15 `AIN4` → PA12/TIMG0 CCP0 | Capture |
| B | H4.15 `BIN3` → PA13/TIMG0 CCP1 | Capture |
| PWM | H4.16 `AIN3` → PB26/TIMG6 CCP0 | combined capture |
| Z | H3.16 → PB23/GPIO IRQ | 零位事件 |
| GND | 逻辑地 | 共参考 |

约束：U3 保持空置；MS42CG 不直插 U6/U16，不接 D36A 控制头，不执行 SPI 改焊。TIMG6 整个实例归 PWM 捕获，不能再给蜂鸣器或其他外设。

## 9. UART

### K230 / USART1

| USART1 | MCU 角色 | K230 |
|---:|---|---|
| pin1 PA9 | UART1 RX | K230 TX |
| pin2 PA8 | UART1 TX | K230 RX |
| pin3 GND | 共参考 | GND |
| pin4 5V | DNC | K230 使用独立稳定电源 |

### USART0

PA10/PA11 只归板载 CH340/UART0。外部 USART0 不接 K230，也不允许另一个推挽 TX 与 CH340 TX 并联。

### USART2

PB15/PB16 为未来资源；当前不创建 SysConfig 实例，USART2 pin1–4 全部 DNC，不再有 RC 舵机 Owner。

## 10. OLED、MPU6050、蜂鸣器

| 接口 | 针位 | MCU/电源 | 条件 |
|---|---|---|---|
| U8 OLED | 1 GND / 2 VCC / 3 SCL / 4 SDA | GND / 待核电源 / PA1 / PA0 | I2C 上拉必须为 3.3V 域 |
| MPU6050 四针/GY | VCC/GND/GY_SCL/GY_SDA | 待核电源/GND/PB2/PB3 | 不把接口 5V 丝印等同于 5V 数据线 |
| H13 | 1 GND / 2 BEEP / 3 3.3V | GND/PB27/3.3V | 仅低电流有源模块；无源保持 DNC 并重审驱动 |

项目口语“U1 MPU6050”是模块编号，不等于 EPRO designator `U1`；施工表必须写“MPU6050 四针/GY 接口”。

## 11. 全局 DNC

- U3、U12、H8；
- 拓展板 KEY1–KEY4、LED1–LED2；
- 开发板 PB21 KEY、PB22 LED；
- H10 pin6–8；
- U6/U16 pin1、pin6；
- U2 除改线后的 pad10/11/12/14/15/16 外的焊盘；
- DRV H3 pin5–8；
- D36A 第二通道、ADC、5V；
- USART1 5V、USART2 全口；
- 旧无线模块全部引脚。

## 12. 首次逻辑上电门槛

在电机和步进电机功率均断开的条件下，先证明：

- DRV 四个输入均低；
- D36A EN1 为低、ST1 无脉冲；
- 所有 Timer 输出保持禁用；
- H10 输入、I2C 上拉、K230 TX、MS42CG A/B/PWM/Z 均不超过 MCU 允许范围；
- 所有 DNC 针已绝缘且无意外导通。

本文件未执行上述任何动作。
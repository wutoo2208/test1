# MSPM0G3507 Pin Plan v1.5 候选 — 六路 I²C 巡线替换

> **状态**：`HISTORICAL / SUPERSEDED / MATERIALIZED AS FROZEN v1.5`。
> **历史说明**：本稿的 H10/GY共享 I2C1 方案已被用户否决；当前方案见 `mspm0g3507-pin-plan-frozen-v1.5.md`，不得按本稿接线。  
> **日期**：2026-07-31。  
> **基线快照**：`mspm0g3507-pin-plan-frozen-v1.4.md`，本轮读取时 SHA-256 为 `BE3C0819221B9BD478FB2073517A7A7D30F9A2AA9AE45F97C6F0D710ACAB7876`；该文件由 Claude Code 持有，本候选不修改它。  
> **资料**：`docs/reviewed/linefollower-6ch-pdf-review.md`。  
> **授权边界**：只提出资源与线束候选；不授权修改 `.syscfg`、源码、构建、烧录、接线、切线、上电或运动测试。

## 1. 变更目标

- 主循迹模块由五路一体 TCRT5000 改为 HiWonder/AiBlock LineFollower_6CH V1.0。
- 新模块是 4 线 I²C 设备，不使用 6 个 GPIO。
- 与 MPU6050 共享 I2C1：PB2 SCL、PB3 SDA。
- 保留当前无线 SPI0、K230、OLED、DRV8870、编码器、D36A、MS42CG 和 KEY2 v1.4 修订。

## 2. MCU Owner 变更

| MCU 引脚/资源 | 旧 Owner | v1.5 候选 Owner | 状态 |
|---|---|---|---|
| PB2 / I2C1 SCL | MPU6050 | MPU6050 + LineFollower_6CH shared bus | CANDIDATE / ELECTRICAL-PENDING |
| PB3 / I2C1 SDA | MPU6050 | MPU6050 + LineFollower_6CH shared bus | CANDIDATE / ELECTRICAL-PENDING |
| PA25 | TCRT OUT1 | DNC / RELEASED | CANDIDATE |
| PA27 | TCRT OUT2 | DNC / RELEASED | CANDIDATE |
| PA16 | TCRT OUT3 | DNC / RELEASED；仅 H10 改板方案中隔离于 pin3 | CANDIDATE |
| PA14 | TCRT OUT4 | DNC / RELEASED；仅 H10 改板方案中隔离于 pin4 | CANDIDATE |
| PB20 | TCRT OUT5 | DNC / RELEASED | CANDIDATE |

模块地址：LineFollower=`0x5C`，MPU6050=`0x68/0x69` 候选；地址静态不冲突。I²C 速率、总线上拉和失电行为仍未确认。

## 3. H10 复用可行性结论

| H10 | MCU | 硬件 I²C 能力 |
|---:|---|---|
| 1 | PB19 | 无；继续无线 MISO |
| 2 | PB17 | 无；继续无线 MOSI |
| 3 | PA16 | I2C1 SDA 可选，但不能与 PB3 同时作为独立 SDA 端点使用 |
| 4 | PA14 | 无 I²C |
| 5 | PB20 | 无 I²C |
| 6 | PB25 | 无；继续无线 CSN |
| 7 | PA25 | 无 I²C |
| 8 | PA27 | 无 I²C |
| 9 | 5V | 电源候选 |
| 10 | GND | 地 |

**H10 原始网络没有完整硬件 I²C 对。** 不能把模块 SDA/SCL 随意接到两根空闲 OUT，也不采用软件模拟 I²C 作为当前主方案。

## 4. 两个线束候选

### 4.1 方案 A — 推荐，无 PCB 切线

| 模块 | 拓展板端点 | MCU/电源 |
|---|---|---|
| 5V | H10 pin9 | 5 V；85 mA 资料负载，供电预算待测 |
| GND | H10 pin10 | GND |
| SDA | GY_SDA 并联/Y 线 | PB3 / I2C1 SDA |
| SCL | GY_SCL 并联/Y 线 | PB2 / I2C1 SCL |

H10 pin1/2/6 无线保持不变；pin3/4/5/7/8 释放后 DNC。优点是不切 PCB，缺点是电源和数据不在同一个 H10 插头。

### 4.2 方案 B — 单一 H10 插头，需要隔离/桥接

| 模块 | H10 | 必要改板 |
|---|---:|---|
| SDA | pin3 | 隔离 H10 pin3 与 PA16/OUT3 原网络，再桥接到 GY_SDA/PB3 |
| SCL | pin4 | 隔离 H10 pin4 与 PA14/OUT4 原网络，再桥接到 GY_SCL/PB2 |
| 5V | pin9 | 无信号改线；供电能力待测 |
| GND | pin10 | 共地路径待核 |

未确认具体切线点、焊盘观察面和连续性前，方案 B 只是机械布线候选，不得施工。

## 5. 未变化资源

- H10 pin1/PB19、pin2/PB17、pin6/PB25 继续属于无线 SPI0；
- PB1 CE、PA12 SCK、PB16 IRQ 不变；
- OLED I2C0 PA0/PA1 不变；
- K230 UART1 PA8/PA9、CH340 UART0 PA10/PA11 不变；
- D36A、MS42CG、DRV8870、左右轮编码器和 v1.4 KEY2 修订不变。

## 6. 软件接口候选

```text
I2C1 shared bus
  - LineFollower_6CH: 0x5C
  - MPU6050: 0x68 or 0x69 (actual AD0 pending)

line sample candidate
  - digital_state: read 0x05, 1 byte
  - analog[6]: read 0x06, 12 bytes, little-endian
  - threshold: disabled until protocol conflict is resolved
```

本文件不冻结 200 Hz、I²C 时钟、超时、滤波、PID 或通道权重。

## 7. 批准前必须回答

1. 采用方案 A（无切线）还是方案 B（H10 单插头改板）。
2. 提供六路模块正反面和四线插头照片，确认实物版本与线序。
3. 若采用 B，提供 H10 附近拓展板正反面高清照片，确定隔离和桥接点。
4. 断电测量 H10 pin3/4 到 PA16/PA14、GY_SDA/GY_SCL 到 PB3/PB2 的连续性。
5. 上电前确认 H10 5 V 极性、空载/带载电压和 85 mA 负载能力。
6. 实测 SDA/SCL 空闲电压与并联上拉等效阻值。
7. 冻结模块通道 1→6 的车体左/右顺序。

用户批准后才可生成 frozen v1.5，并同步 Pin Plan README/正式 harness；在此之前 v1.4 仍是已存在的最新修订快照，但其中旧 TCRT 线路不得用于新六路模块。

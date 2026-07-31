# MSPM0G3507 Adapter Harness v1.5 候选 — LineFollower_6CH

> **状态**：`HISTORICAL / SUPERSEDED BY mspm0g3507-adapter-harness-v1.5.md / DO NOT WIRE`。
> **历史说明**：H10/GY共享方案已取消；当前LineFollower使用U12，OLED使用原MPU接口。  
> **配套 Pin Plan**：`mspm0g3507-pin-plan-linefollower-6ch-v1.5-candidate.md`。  
> **边界**：不修改 Claude Code 持有的 v1.4，不授权接线、切线、上电或 SysConfig。

## 1. 模块端四线

资料和图示顺序：

```text
5V | GND | SDA | SCL
```

该顺序不能代替用户实物插头观察面；制作线束前必须按丝印和正反面照片确认。

## 2. 方案 A — 推荐无切线线束

```text
LineFollower 5V  → H10 pin9  / 5V
LineFollower GND → H10 pin10 / GND
LineFollower SDA → GY_SDA    / PB3 I2C1 SDA
LineFollower SCL → GY_SCL    / PB2 I2C1 SCL
```

- SDA/SCL 与 MPU6050 并联共享 I2C1；不得串接。
- 不将模块 5 V 接到 MCU 3.3 V。
- H10 pin1/2/6 保留无线；pin3/4/5/7/8 不接六路模块。
- 线束应有明确方向标识，避免把 H10 10 针顺序与模块 4 针顺序误认为直通。

## 3. 方案 B — H10 单插头改板候选

### 改板前网络

```text
H10 pin3 → OUT3 → PA16
H10 pin4 → OUT4 → PA14
```

### 目标网络

```text
H10 pin3 → LineFollower SDA → PB3/GY_SDA
H10 pin4 → LineFollower SCL → PB2/GY_SCL
H10 pin9 → LineFollower 5V
H10 pin10 → LineFollower GND
```

### 必须执行的隔离原则

- pin3 与 PA16 原网络必须断开后才可桥接 PB3；
- pin4 与 PA14 原网络必须断开后才可桥接 PB2；
- 禁止将 PA16 与 PB3、PA14 与 PB2 直接短接；
- 精确切线/焊盘位置尚未确认，本文件不提供施工点；
- 改板后必须形成逐针断电连续性表和相邻针短路检查。

## 4. 断电连续性验收表模板

| 测量 | 期望 | 实测 | 结论 |
|---|---|---|---|
| H10 pin3 ↔ PB3/GY_SDA | 低阻 | — | NOT RUN |
| H10 pin3 ↔ PA16 | 开路（方案 B） | — | NOT RUN |
| H10 pin4 ↔ PB2/GY_SCL | 低阻 | — | NOT RUN |
| H10 pin4 ↔ PA14 | 开路（方案 B） | — | NOT RUN |
| H10 pin9 ↔ 5V | 低阻 | — | NOT RUN |
| H10 pin10 ↔ GND | 低阻 | — | NOT RUN |
| SDA/SCL/5V/GND 相邻短路 | 不应短路 | — | NOT RUN |

## 5. 上电前门槛

- 确认采用 A 或 B；
- 确认模块和 H10 观察面；
- 确认 H10 5 V 极性和供电预算；
- 确认模块 SDA/SCL 上拉到约 3.3 V 而非 5 V；
- 确认 MPU6050 与六路模块并联后的上拉和总线电容；
- 首次上电只接逻辑/传感器域，不连接电机、D36A 或整车运动。

# 项目当前状态（轻量上下文）

> **更新时间**：2026-07-31（frozen v1.5 设计已批准：LineFollower→U12/I2C0，OLED→原MPU接口/I2C1，MPU移除；Claude Code 固件工作保持隔离）。
> **用途**：供新的 Claude Code / Codex 短会话快速了解当前项目；它不是正式题面、决策或测试记录的替代品。发生冲突时，按 `AGENTS.md` 的证据优先级处理。

## 1. 当前阶段与授权

- 项目：2026 TI 杯赛区赛 H 题“车载平衡滚球运动控制系统”。
- 阶段：方案、资料与接口收敛；当前仅进行文档与静态资料核验。
- 未获授权：修改 `.syscfg`、源码、CCS 工程或生成文件；运行 SysConfig、构建、烧录、探针、串口；真实硬件接线、上电与运动测试。
- 每轮只处理一个明确任务；没有证据不得把候选方案写成已确认接线或实测通过。

## 2. 题面关键门槛

- 小车为车载电池供电的轮式循线系统，车体包络不超过 `35 cm × 25 cm`。
- 仅允许使用红外光电模块循迹；钢球位置检测必须使用摄像头，并要求车载发送、场外接收显示/存储、录像与回放。
- 黑线宽 `1.8 ± 0.2 cm`；单圈循线停车时间不超过 20 s、停车偏差不超过 2 cm；动态平衡相关位置误差要求不超过 1 cm。
- 正式、完整要求和评分以 `docs/requirements.md` 为准。

## 3. 已确认的项目事实

- 用户于 2026-07-30 明确陈述：DRV8870、两台车轮电机、两路编码器、五路 TCRT 循迹、nRF24L01、OLED 和 MPU6050 已完成实物接线。该陈述保留为历史现场事实；用户于 2026-07-31 决定五路 TCRT 性能不足，后续主循迹改用 HiWonder/AiBlock LineFollower_6CH V1.0。新模块尚未接线或实测。
- `empty.syscfg` 与现有固件仍处于 Claude Code 工作状态；当前已构建代码包含旧 TCRT GPIO 原始输入路径、编码器、OLED/MPU I2C 诊断和默认锁止的 nRF24 PTX 框架。LineFollower_6CH I2C 驱动尚未实现，本轮文档任务不修改源码、SysConfig 或生成文件。
- 无线框架已包含：PB25 软件 CSN、SPI0 有界事务、寄存器初始化与读回、32-byte 分片、16-frame 队列、SysTick 时限、CE 短脉冲、`TX_DS/MAX_RT/timeout` 处理、统计与 `radio arm/disarm/status/test` 命令。
- 由于 COM7 接收适配器的 RF channel、地址、air rate、CRC、payload mode/width 和 ACK 策略未知，当前 `RADIO_ALLOW_TX=0`、`RADIO_PROFILE_VALID=0`、`RADIO_AUTO_ARM=0`；正常执行无法进入唯一 CE-high 路径，不会发射 RF。
- SysConfig 1.26.2 隔离生成及 TI clang 5.1.1 LTS clean build/link 已通过，生成 `Debug/test1.out`；Flash 使用 `0x5f70/0x20000`，SRAM 使用 `0x478/0x8000`。这仅是构建证据，当前无线版尚未烧录或发射。
- 当前诊断命令：`help`、`status`、`pins`、`line`、`enc`、`i2c`、`mpu`、`radio_regs`、`radio_status`、`radio_arm`、`radio_disarm`、`radio_test`、`stop`、`selftest`。`radio_arm` 在未知 profile 下返回 `BLOCKED`。

- 主控开发板：立创天猛星 `MSPM0G3507`；MCU/Timer/接口资源以 `docs/pin-plan/mspm0g3507-pin-plan-frozen-v1.2.md` 为引脚 Owner 基线。固件将 PB25 从 SPI0 硬件 CS0 改为同一物理网络上的 GPIO 软件 CSN，以保证完整 nRF 命令帧持续选中；其余 v1.2 端点不变。
- 驱动电机：额定 12 V、堵转电流 3.2 A（每个）、减速比 1:28；编码器输入/输出均为 3.3 V（用户陈述）。
- 电池：3S 锂电池组；电池标签标称 11.1 V、充电限制 12.6 V、额定容量 2800 mAh。BMS 与持续/峰值放电能力未确认。
- 电机驱动：库存为 DRV8870 双路模块；长期堵转热、电流、模块版本和保护方案仍需核实。
- 原八路蓝光灰度模块和五路一体 TCRT5000 均退出后续主循迹方案；旧资料、旧接线和固件状态仅保留追溯。
- 主循迹已选 HiWonder/AiBlock LineFollower_6CH V1.0：5 V、资料电流 85 mA、I²C 7 位地址 0x5C，可读六路数字状态和六路 16 位模拟值。frozen v1.5 指定经 U12 独占 I2C0：PA28 SDA、PA31 SCL；模块 `5V/GND/SDA/SCL` 与 U12 `5V/SCL/SDA/GND` 必须按信号交叉，尚未制作线束或实测。
- MPU6050 当前 `NOT FITTED`；原 MPU6050/GY 接口改接 OLED 并独占 I2C1：PB3 SDA、PB2 SCL。OLED 实物丝印为 `GND/VDD/SCK/SDA`，其中 `VDD` 按正电源、`SCK` 按 I²C `SCL` 使用，必须重排线束，地址0x3C/0x3D和上拉待实测。K230 单向UART协议v1已由用户批准：IO9/TXD→PA9/UART1_RX、115200 8N1、固定14字节、CRC-8/ATM；PA8保留但当前DNC。该协议尚未写入SysConfig/源码或实测。UART0仍留给开发板PA10/PA11；COM7无线接收适配器和nRF24约束不变。
- 天猛星拓展板 V2.0 的正面装配照片已归档；背面、实际供电路径和排针线序仍未确认。用户称有 MP1584EN 可调降压模块；参数图宣称 4.5–28 V 输入、0.8–20 V 可调输出、最大 3 A，但实际板型、5 V 设定和带载能力未测。
- 摆杆执行器路线已确定为 D36A 通道1 + MS42CG V2，取代 RC 舵机；使用 ST1/DIR1/EN1 和 A/B/PWM/Z，MS42CG 固定 3.3 V 逻辑域。实际绕组、相电流、细分、输入时序、机械限位和散热仍待验证。
- DRV8870 车轮控制采用 U2 三隔离、三桥接、一保留；车轮功率绕过拓展板直连 DRV。U6/U16 只接编码器四线；U12 已由 frozen v1.5 分配给 LineFollower，U3/H8 不使用，PB18 因板载 KEY3 占用而禁止用于无线。

## 4. 当前高优先级阻塞

1. LineFollower_6CH 实物版本/插头观察面、U12 Pin 1观察面、交叉线束连续性、5 V负载、SDA/SCL空闲电压与上升沿、I²C速率/超时、通道1～6左右顺序、数字极性、模拟范围和阈值寄存器冲突。
2. U2 实际切线点、桥接点、DRV H3 Pin 1 和 U6/U16 观察面；PCB 修改后必须完成断电连续性表。
3. K230 的精确板型和供电能力；协议v1实际脚本、3.3 V/115200波形、帧率、延迟、抖动、坐标标定，以及车载发送、场外接收、显示/录像完整链路。
4. 电池 BMS、持续/峰值能力、保险/急停；DRV8870 限流、散热及车轮功率线规格。
5. 左右轮编码器 PPR/CPR、相位、最高边沿频率和正方向标定。
6. D36A 输入门限/STEP 时序、EN/ST 复位偏置、步进绕组/相电流/细分、MS42CG 六针观察面、PWM/Z 时序、机械限位和散热。
7. OLED 的 GY pad1观察面、重排线束连续性、`VDD`供电范围、SCK/SDA上拉电压及实际地址0x3C/0x3D；MPU6050当前不装。H13蜂鸣器有源/无源与输入电流仍待确认。
8. 无线：车载模块 VCC/GND/去耦和 CE 复位外部下拉；COM7 USB 接收适配器的 channel、地址、air rate、CRC、payload width/mode、ACK/重试和串口输出格式。建议通过购买链接/手册或被动抓取接收适配器上电时 nRF SPI 配置获得；未知前 RF 保持锁止。

## 5. 建议的新会话读取顺序

1. `AGENTS.md`、`CLAUDE.md`、本文件和 `docs/requirements.md`。
2. 本轮任务的最新 `docs/handoffs/*.md`（如存在）。
3. 仅读取本任务需要的模块资料、接口/决策/架构/测试文件。
4. 只有需要追溯具体事实时才读取 `docs/reviewed/**`、`docs/source-pdf/**` 或 `docs/extracted/**`。

## 6. 权威文件索引

- 正式题面与评分：`docs/requirements.md`
- 总计划及阶段门：`docs/h-task-master-plan.md`
- 决策：`docs/decisions.md`
- 接口收敛：`docs/hardware-interface.md`
- 硬件知识库：`docs/hardware/`
- 已复核资料：`docs/reviewed/`
- 验证证据：`docs/test-log.md`
- 基础 MCU/Timer/PinMux 资源：`docs/pin-plan/mspm0g3507-pin-plan-frozen-v1.2.md`，再依次应用 v1.3/v1.4/v1.5
- 当前模块逐线线束：`docs/pin-plan/mspm0g3507-adapter-harness-v1.5.md`
- 当前 LineFollower/OLED/MPU/U12/I2C 修订：`docs/pin-plan/mspm0g3507-pin-plan-frozen-v1.5.md`（FROZEN-DESIGN / NOT SYSCONFIG-APPLIED）
- K230单向UART协议：`docs/protocols/k230-ball-position-uart-v1.md`（`USER-APPROVED DESIGN / NOT IMPLEMENTED / NOT TESTED`）
- Pin Plan 版本索引：`docs/pin-plan/README.md`

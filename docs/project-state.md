# 项目当前状态（轻量上下文）

> **更新时间**：2026-07-30（诊断固件首版已构建）。
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

- 用户于 2026-07-30 明确陈述：DRV8870、两台车轮电机、两路编码器、五路 TCRT 循迹、nRF24L01、OLED 和 MPU6050 已完成实物接线。该陈述更新“是否已接线”的现场状态，但不自动证明端点、电平、极性、Pin 1、供电或连续性正确。
- `empty.syscfg` 与 `empty.c` 已进入首版 bring-up：UART0/CH340 为 `115200 8N1` 诊断口；电机与 D36A 仅配置软件低电平安全锁；已加入 TCRT 原始输入、右轮 TIMG8 QEI、左轮 TIMA1 Capture/软件正交、I2C0 OLED 探测、I2C1 MPU `WHO_AM_I` 和 SPI0 nRF 寄存器只读诊断命令。
- SysConfig 1.26.2 隔离生成及 TI clang 5.1.1 LTS clean build/link 已通过，生成 `Debug/test1.out`；Flash 使用 `0x2e00/0x20000`，SRAM 使用 `0x467/0x8000`。这仅是构建证据，尚未烧录或实测任何模块。
- 当前诊断命令：`help`、`status`、`pins`、`line`、`enc`、`i2c`、`mpu`、`radio`、`stop`、`selftest`。没有电机运动、PWM、步进脉冲、无线发射或任意寄存器写命令。

- 主控开发板：立创天猛星 `MSPM0G3507`；MCU/Timer/接口资源已按 `docs/pin-plan/mspm0g3507-pin-plan-frozen-v1.2.md` 冻结为当前唯一设计基线，配套线束见 `docs/pin-plan/mspm0g3507-adapter-harness-v1.2.md`；`empty.syscfg` 尚未修改，硬件尚未接线/上电。
- 驱动电机：额定 12 V、堵转电流 3.2 A（每个）、减速比 1:28；编码器输入/输出均为 3.3 V（用户陈述）。
- 电池：3S 锂电池组；电池标签标称 11.1 V、充电限制 12.6 V、额定容量 2800 mAh。BMS 与持续/峰值放电能力未确认。
- 电机驱动：库存为 DRV8870 双路模块；长期堵转热、电流、模块版本和保护方案仍需核实。
- 原八路蓝光灰度模块已排除，不作为 H 题最终循迹方案。
- 主循迹已确定为五路一体 TCRT5000；v1.2 最新端点为 OUT1/OUT2→H10 pin7/pin8→PA25/PA27，OUT3/OUT4/OUT5→H10 pin3/pin4/pin5→PA16/PA14/PB20，必须使用交叉线束。输出电平、结构、极性和车体左右顺序仍待台架关闭。
- MPU6050 规划为 PB2/PB3 的 I2C1；K230 规划为 PA8/PA9 的 UART1；UART0 留 CH340。无线 SPI0 v1.1 因 `PB18=KEY3` 已否决；v1.2 已获批准并冻结，使用 PB17/PA12/PB19/PB25 + PB1 CE + PB16 IRQ，PB18 标记为 `KEY3 / FORBIDDEN-FOR-RADIO`。该设计尚未接线，无线 pin1/pin2、供电、IRQ 输出结构和 SPI 参数仍阻塞；低速无线不能替代题面视频图传。
- 天猛星拓展板 V2.0 的正面装配照片已归档；背面、实际供电路径和排针线序仍未确认。用户称有 MP1584EN 可调降压模块；参数图宣称 4.5–28 V 输入、0.8–20 V 可调输出、最大 3 A，但实际板型、5 V 设定和带载能力未测。
- 摆杆执行器路线已确定为 D36A 通道1 + MS42CG V2，取代 RC 舵机；使用 ST1/DIR1/EN1 和 A/B/PWM/Z，MS42CG 固定 3.3 V 逻辑域。实际绕组、相电流、细分、输入时序、机械限位和散热仍待验证。
- DRV8870 车轮控制采用 U2 三隔离、三桥接、一保留；车轮功率绕过拓展板直连 DRV。U6/U16 只接编码器四线；U3/U12/H8 不使用，PB18 因板载 KEY3 占用而禁止用于无线。

## 4. 当前高优先级阻塞

1. TCRT5000 的 OUT 高低电压、推挽/开漏、有效极性和 OUT1–OUT5 车体左右顺序；在此之前 H10 只冻结资源，不允许直连 MCU。
2. U2 实际切线点、桥接点、DRV H3 Pin 1 和 U6/U16 观察面；PCB 修改后必须完成断电连续性表。
3. K230 的精确板型、电平、波特率/帧格式、供电以及车载发送、场外接收、显示/录像完整链路。
4. 电池 BMS、持续/峰值能力、保险/急停；DRV8870 限流、散热及车轮功率线规格。
5. 左右轮编码器 PPR/CPR、相位、最高边沿频率和正方向标定。
6. D36A 输入门限/STEP 时序、EN/ST 复位偏置、步进绕组/相电流/细分、MS42CG 六针观察面、PWM/Z 时序、机械限位和散热。
7. OLED/MPU6050 的供电、地址和 I2C 上拉电压；H13 蜂鸣器有源/无源与输入电流。
8. 无线模块 pin1/pin2、供电、IRQ 输出结构、SPI mode/频率/协议和线束；TCRT 改线后的电平、极性、物理顺序及交叉线束连续性；MS42 PWM 捕获仍需未来 SysConfig 隔离验证。

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
- 当前唯一 MCU/Timer/PinMux 资源：`docs/pin-plan/mspm0g3507-pin-plan-frozen-v1.2.md`
- 当前唯一改线与线束：`docs/pin-plan/mspm0g3507-adapter-harness-v1.2.md`
- Pin Plan 版本索引：`docs/pin-plan/README.md`

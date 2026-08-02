# 验证与测试日志

> 初始化日期：2026-07-28  
> 静态阅读、构建验证和真实硬件实测是不同证据等级。源码中存在某段逻辑，不等于它已构建、烧录或在板运行。

## 1. 证据等级

| 类型 | 可证明内容 | 不可据此声称 |
|---|---|---|
| 资料复核 | 原理图、题面、datasheet 或审阅资料中的事实 | 固件已实现、硬件已运行 |
| 静态核验 | 文件存在、源码路径、配置内容和文档一致性 | 编译通过、烧录成功、实物行为正确 |
| 构建验证 | 指定版本源码在记录环境中编译/链接的结果 | 硬件接口或控制效果正确 |
| 实测 | 指定硬件、固件、接线和步骤下的观测结果 | 未覆盖环境、参数或边界条件 |

## 2. 结果状态

| 状态 | 含义 |
|---|---|
| `PASS` | 已按记录步骤获得符合预期的证据 |
| `FAIL` | 已执行且实际结果不符合预期 |
| `BLOCKED` | 前置输入、授权或条件缺失 |
| `NOT RUN` | 明确未执行 |
| `INCONCLUSIVE` | 已执行但证据不足以判定 |

不得用“预计正常”“代码看起来正确”替代 `PASS`。

## 3. 当前禁止项

依据 `CON-003`、`CON-004` 和 `DEC-005`，当前不允许：

- SysConfig 生成或校验；
- CCS Build、编译或链接；
- 烧录、调试和探针访问；
- 串口扫描、发送或抓取日志；
- 接线、上电或测量真实硬件。

## 4. 初始基线观察

| ID | 日期 | 类型 | 观察 | 结果 | 证据与局限 |
|---|---|---|---|---|---|
| `TEST-001` | 2026-07-28 | 静态核验 | `docs/` 已存在 PDF 工作流、硬件资料、审阅记录和原始 PDF。 | PASS | 路径只读检查；未重新验证 PDF 内容。 |
| `TEST-002` | 2026-07-28 | 静态核验 | `empty.c` 调用 `SYSCFG_DL_init()`，随后循环延时并翻转 PB22 LED 对应宏。 | PASS | 只证明源码结构，不证明已构建或 LED 已闪烁。 |
| `TEST-003` | 2026-07-28 | 静态核验 | `empty.syscfg` 当前配置 MSPM0G3507 与 PB22 LED GPIO，未见控制题外设配置。 | PASS | 只读配置检查；未运行 SysConfig。 |
| `TEST-004` | 2026-07-28 | 静态核验 | 初始化前 7 个目标根文档均不存在。 | PASS | 创建前路径检查。 |

## 5. 静态验证记录

| ID | 日期 | 检查 | 预期 | 实际 | 结果 | 证据 |
|---|---|---|---|---|---|---|
| `TEST-005` | 2026-07-28 | 文档增补范围 | 只新增 7 个指定 Markdown | 待完成后验复核 | NOT RUN | 初始化时预留；见本轮最终验证报告 |
| `TEST-006` | 2026-07-28 | 既有文件完整性 | 创建前后既有文件哈希一致 | 待完成后验复核 | NOT RUN | 初始化时预留；哈希不写入项目 |
| `TEST-007` | 2026-07-28 | 状态与事实边界 | 无未经证实的硬件通过结论 | 待完成后验复核 | NOT RUN | 初始化时预留 |

> 注：为遵守“已有内容不修改”，完成后的后验结果由执行报告给出；本文件的初始化行不回写伪造结果。

## 6. 构建验证记录

| 项目 | 状态 | 原因/证据 |
|---|---|---|
| SysConfig 生成/验证 | PASS | 2026-07-30 使用项目声明的 SysConfig `1.26.2+4477`、SDK `2.11.00.07` 隔离验证并由正常 build 路径重新生成。 |
| CCS Build | PASS | 2026-07-30 使用 CCS 所带 `gmake.exe clean all`。 |
| 编译与链接 | PASS | TI clang `5.1.1 LTS` 完成编译链接并生成 `Debug/test1.out`；没有硬件含义。 |

## 7. 台架与实测记录

当前没有由本会话执行的真实台架或硬件测试记录；用户已陈述外设完成接线，但尚未形成电压、连续性、串口或功能实测证据。

| 项目 | 状态 | 原因 |
|---|---|---|
| 固件烧录 | NOT RUN | 用户禁止。 |
| 调试器/探针连接 | NOT RUN | 用户禁止。 |
| 串口访问 | NOT RUN | 用户禁止。 |
| 板上 LED 观察 | NOT RUN | 未连接硬件；源码行为不等于实测。 |
| 外设接线和供电 | NOT RUN | BOM 与接线未确认，且用户禁止。 |
| 控制算法测试 | NOT RUN | 算法与验收指标尚未定义。 |

## 8. 未执行项目

- 未重新生成 `ti_msp_dl_config.c/.h`。
- 未确认当前探针、串口或目标板是否连接。
- 未测量电压、电流、波形、频率、延时或控制误差。
- 未确认现有构建产物与当前源码完全对应。
- 未验证任何外接模块、电机、传感器或通信链路。

## 9. 2026-07-30 首版诊断固件记录

| ID | 类型 | 检查/动作 | 结果 | 证据与局限 |
|---|---|---|---|---|
| `TEST-008` | 用户陈述 | DRV8870、电机、编码器、五路 TCRT、nRF24L01、OLED、MPU6050 已接线。 | RECORDED | 只更新现场状态；未由照片、连续性、电压或代理实测独立确认。 |
| `TEST-009` | SysConfig 隔离验证 | 使用 SysConfig `1.26.2+4477` 和 SDK `2.11.00.07` 生成 v1.2 bring-up 配置。 | PASS | 0 error；有 SPI/QEI/Capture 在 STOP/STANDBY 不保持寄存器的 info，当前固件不进入 STOP/STANDBY。 |
| `TEST-010` | 构建验证 | CCS `gmake clean all`，TI clang `5.1.1 LTS` 编译链接 `empty.c` 与生成配置。 | PASS | 无编译/链接错误；生成 `Debug/test1.out`。不证明硬件正确。 |
| `TEST-011` | 静态核验 | 启动和主循环持续保持 DRV 四输入、D36A EN/DIR/STEP、nRF CE 与蜂鸣器为低；不存在运动命令。 | PASS | 仅证明初始化后的软件行为；复位/烧录期间高阻窗口仍需外部下拉和实测关闭。 |
| `TEST-012` | 资源核验 | v1.2 UART/I2C/SPI/QEI/Capture/GPIO PinMux 由 SysConfig 求解。 | PASS | TCRT=PA25/PA27/PA16/PA14/PB20；无线=PB25/PB17/PA12/PB19/PB1/PB16；两编码器与 v1.2 一致。 |
| `TEST-013` | 烧录/串口/台架 | 烧录、CH340 日志、I2C/SPI、传感器、编码器和电机实测。 | NOT RUN | 等待用户确认断开电机功率、3.3V 电平/上拉和外部安全偏置后，另行 L3 授权。 |

构建后资源：Flash `0x2e00 / 0x20000`，SRAM `0x467 / 0x8000`。当前 `Debug/**` 是构建生成物，不得手工修改。

## 10. 2026-07-30 无线传输框架记录

| ID | 类型 | 检查/动作 | 结果 | 证据与局限 |
|---|---|---|---|---|
| `TEST-014` | 用户澄清 | COM7 是独立 `CH340 + 控制器 + nRF24` USB 无线接收适配器，不是开发板 PA10/PA11 UART。 | RECORDED | 未收到接收器型号、手册、购买链接或 RF 配置。 |
| `TEST-015` | SysConfig 隔离验证 | PB25 改为初始高 GPIO 软件 CSN；SPI0 保留 PA12/PB17/PB19、MOTO3 mode0、500 kHz。 | PASS | 0 error；STOP/STANDBY retention 为 info，当前固件不进入这些模式。 |
| `TEST-016` | 构建验证 | nRF PTX 框架经 TI clang `5.1.1 LTS` clean build/link。 | PASS | 生成 `Debug/test1.out`；Flash `0x5f70/0x20000`，SRAM `0x478/0x8000`。 |
| `TEST-017` | 静态安全核验 | `RADIO_ALLOW_TX=0`、`RADIO_PROFILE_VALID=0`、`RADIO_AUTO_ARM=0`；唯一 CE-high 只能经成功 profile validation 和 arm 到达。 | PASS | 当前构建不会发 RF；不证明复位高阻期间 CE 安全，仍需外部下拉。 |
| `TEST-018` | 功能静态核验 | 已实现软件 CSN、有界 SPI、初始化/读回、队列/分片、SysTick 状态机、`TX_DS/MAX_RT/timeout` 和统计。 | PASS | 尚未填入接收器 profile；dynamic-payload `ACTIVATE` 与 CRC/power profile 扩展仅在需要时补充。 |
| `TEST-019` | 无线端到端 | 车载 nRF 配置、发包、COM7 接收和透明串口重组。 | BLOCKED | 缺 RF channel、地址/字节序、速率、CRC、payload 模式/宽度、ACK/重试及 COM7 输出格式。 |

## 11. 2026-07-31 六路巡线替换记录

| ID | 类型 | 检查/动作 | 结果 | 证据与局限 |
|---|---|---|---|---|
| `TEST-020` | 用户决定 | 原五路 TCRT5000 性能不足，主循迹改用 HiWonder/AiBlock LineFollower_6CH V1.0。 | RECORDED | 只证明选型变化；新模块尚未接线、上电或跑赛道。 |
| `TEST-021` | 资料复核 | 4 份 PDF、2 张尺寸图、原理图和多平台示例完成提取/页面复核。 | PASS | 资料确认 5 V、85 mA、I²C 0x5C、六路数字/模拟；阈值寄存器存在冲突。 |
| `TEST-022` | 静态 IOMUX 核验 | H10 OUT1～OUT8 的 MCU 复用中不存在完整硬件 I²C SCL/SDA 对；PB2/PB3 I2C1 可与 MPU6050 共享候选。 | PASS | 依据 SysConfig 1.28 MSPM0G350X deviceData；不证明实物电气或 SysConfig 已配置。 |
| `TEST-023` | 新模块台架/赛道 | I²C 读数、200 Hz 调度、通道方向、黑线极性、弯道和停车性能。 | NOT RUN | frozen v1.5设计已批准；仍等待 SysConfig/源码阶段授权、线束、电气和后续L3/L5授权。 |
| `TEST-024` | 用户批准 + 静态网络核验 | 冻结 v1.5：LineFollower 经 U12→PA28/PA31 I2C0；OLED 经原MPU/GY→PB3/PB2 I2C1；MPU6050移除。 | PASS | 证明设计决策和文档/EPRO网络一致；不证明SysConfig、线束或硬件。 |
| `TEST-025` | 用户实物照片复核 | OLED正面丝印再次清楚显示 pin1～4=`GND/VDD/SCK/SDA`。 | PASS | 设计将VDD映射为正电源、SCK映射为I²C SCL；未测供电、上拉、地址或通信。 |
| `TEST-026` | v1.5实施验证 | SysConfig生成、构建、断电连续性、空闲电压、I²C扫描和显示/巡线读取。 | NOT RUN | 当前任务仅L1文档；禁止把静态核验写成硬件通过。 |

## 12. 测试记录模板

```text
Test ID: TEST-NNN
Date/time:
Operator/agent:
Related REQ/IF/ARC/DEC/TASK:
Verification type: 资料复核 / 静态核验 / 构建验证 / 实测
Hardware board and revision:
Module exact model:
Firmware revision/hash:
Power and wiring:
Preconditions:
Procedure:
Expected result:
Actual result:
Result: PASS / FAIL / BLOCKED / NOT RUN / INCONCLUSIVE
Evidence path:
Limitations:
Follow-up:
```

## 13. 2026-08-01 REQ-002 第二问最终提交版本

| ID | 类型 | 检查/动作 | 结果 | 证据与局限 |
|---|---|---|---|---|
| `TEST-027` | 构建验证 | 单轮急弯制动版本经固件静态测试和 CCS `MotorSelfTest` 构建。 | PASS | 静态单元测试 11/11；CCS gmake/链接成功；SysConfig 生成文件报告 Unchanged。最终 ELF 大小 `630476 bytes`，SHA-256 `BDE6EFE861A40E0AA3DACCFC68093E32AB54C026FA8DA3B825545ECBB15FEBAA`。 |
| `TEST-028` | 台架工具实测 | 使用 Horco CMSIS-DAP `da941ca0`、MSPM0G3507、TI Pack、100 kHz、sector erase、ELF、`--no-reset` 烧录最终 ELF。 | PASS | pyOCD 退出码 0；擦除/编程均为 `72704 bytes`、71 sectors/pages。见 `docs/handoffs/2026-08-01-req002-right-wheel-brake-flash-complete.md`；只证明写入成功。 |
| `TEST-029` | 用户运动场景陈述 | 用户报告该版本循迹第二问“效果很好”，并明确决定最终作品使用该版本。 | RECORDED | 作为最终版本选择依据；未由代理取得完整视频、正式计时和停车偏差测量，不能替代 `REQ-002` 正式验收。最终冻结见 `docs/handoffs/2026-08-01-req002-final-submission-freeze.md`。 |

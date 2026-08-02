# 2026 TI 杯 H 题：题目覆盖与烧录固件总清单

> 日期：2026-08-01  
> 工程：`C:\Users\Administrator\workspace_ccstheia\test1_2.2_recovered`  
> 用途：提交前唯一烧录索引。发生冲突时，正式题面 `docs/requirements.md`、最终冻结记录和实际文件 SHA-256 优先。

## 1. 最终结论

当前工程只形成了一个最终比赛功能烧录件：

- **REQ-002 / 第二问：单圈顺时针循迹与 A 点停车**。
- 最终文件：`MotorSelfTest/test1_motor_selftest.out`。
- 最终 SHA-256：`BDE6EFE861A40E0AA3DACCFC68093E32AB54C026FA8DA3B825545ECBB15FEBAA`。
- 用户现场报告该版本效果很好，并明确决定第二问使用本版本。

第1问和第3～6问没有形成可在本工程中独立烧录并宣称完成验收的最终固件。`Debug/test1.out` 是安全诊断构建，不是滚球控制或图传完整作品固件。

## 2. 各题目最终固件矩阵

| 题目 | 正式要求 | 当前软件状态 | 应烧录文件 | 提交结论 |
|---|---|---|---|---|
| REQ-001 / 第1问 | 钢球画面实时显示、录像、存储和回放 | MSPM0 侧存在 K230 UART 14 字节帧解析器和 nRF24 单向诊断发送基础；工程中没有 K230 摄像头识别/视频编码程序、场外视频接收/录像/回放程序，也没有对应 K230 `.kfpkg/.bin` | **无最终烧录件** | 不得用 `Debug/test1.out` 或 REQ-002 镜像声称完成图传题目 |
| REQ-002 / 第2问 | 顺时针单圈循迹，`<=20 s`，A 点停车偏差 `<=2 cm`，显示时间 | 全右弯状态机、编码器直线 PI、急弯右轮电气制动、终点双轮制动、OLED 状态显示已集成；用户现场选择为最终版本 | `MotorSelfTest/test1_motor_selftest.out`，SHA-256 `BDE6...FEBAA` | **最终提交版本** |
| REQ-003 / 第3问 | 静止时 O→+5 cm→-5 cm，`<=5 s`，误差 `<=1 cm` | K230 接收解析存在，但没有球位 PID、步进执行器驱动、方向闭合、行程/限位与位置标定；D36A 在当前固件中保持禁用 | **无最终烧录件** | 未实现，不得烧录第二问镜像冒充 |
| REQ-004 / 第4问 | A→B 行驶时球保持 O 点，`<=8 s`、误差 `<=1 cm` | 只有第二问底盘循迹和 K230 诊断输入，没有滚球闭环或 D36A 输出 | **无最终烧录件** | 未实现 |
| REQ-005 / 第5问 | 单圈行驶时球保持 O 点，`<=30 s`、误差 `<=1 cm` | 同 REQ-004；无滚球执行器闭环 | **无最终烧录件** | 未实现 |
| REQ-006 / 第6问 | 单圈行驶时球保持任意指定位置，`<=30 s`、误差 `<=1 cm` | 无目标位置输入、滚球闭环、执行器控制和正式标定 | **无最终烧录件** | 未实现 |
| REQ-007 | 其他评分 | 题面未展开具体固件项 | 无 | 等裁判细则 |
| REQ-008 | 设计报告 | 属于文档提交，不是 MCU 烧录任务 | 无 | 使用设计报告文件，不烧录 |

## 3. 当前工作区两个 `.out`

### 3.1 最终第二问执行镜像

```text
文件：MotorSelfTest/test1_motor_selftest.out
大小：630476 bytes
SHA-256：BDE6EFE861A40E0AA3DACCFC68093E32AB54C026FA8DA3B825545ECBB15FEBAA
构建宏：MOTOR_SELFTEST_BUILD=1
用途：REQ-002 最终比赛循迹
状态：FINAL / FROZEN / 已有烧录成功证据
```

虽然目录名为 `MotorSelfTest`，该构建宏也是当前唯一开放 REQ-002 电机执行路径的比赛候选配置；默认 Debug 构建保持执行器锁止。

### 3.2 Debug 安全诊断镜像

```text
文件：Debug/test1.out
大小：556004 bytes
当前 SHA-256：31C66A2FDE6826792DED87E98962B7DD1F39C6074BB58C253B069870CB9C8EC8
构建宏：MOTOR_SELFTEST_BUILD=0
用途：K230 UART、编码器、传感器、OLED、串口等安全诊断
状态：DIAGNOSTIC / STALE / 非最终比赛固件
```

限制：

- 默认不开放 REQ-002 执行器；
- 当前文件是历史 Debug 构建，与后续最终源码和最终 MotorSelfTest 镜像不同步；
- 不得用于第2问正式测试；
- 不含滚球执行器闭环，不能用于第3～6问；
- 历史 K230 UART1 专用 Debug 候选曾为大小 `551800 bytes`、SHA-256 `D5053F353AF5F37987FF2DA0DDB2F35FB9F846D4622D6EF35A62165DDA642D4C`，但交接记录为未完成烧录，且当前 `Debug/test1.out` 已不是该文件。

## 4. REQ-002 最终控制摘要

### 直线

```text
基础：850 permille
右侧 trim：420 permille
轮速 PI：Kp=4.0，Ki=12.0，输出限幅=50 permille
左编码器 1x → 右 QEI 4x 归一化系数：4.0
左速目标比例：1.20
```

### 普通右弯

```text
RIGHT_CURVE_APPROACH：左820，右420
触发误差：0.06，确认10 ms，最长150 ms

RIGHT_CURVE_ARC：左800，右0（0/0滑行）
触发误差：0.10，确认5 ms
最短300 ms，最长700 ms
回中：误差<=0.04连续60 ms

RIGHT_CURVE_RECOVER：200 ms
左800→850，右450→430
```

### 严苛急弯右轮制动

```text
触发：右偏误差>=0.15且右转方向连续10 ms
次数：每个锁存右弯最多一次
左轮：800
右轮：准备1 ms后进入DRV8870 A通道1/1制动100 ms
结束：释放右轮制动，返回ARC 800/0
```

所有右弯阶段关闭轮速 PI；左向恢复修正力度为 `400`，仅用于弱纠偏。

### 终点停车

```text
返回标记首次识别
→ 两轮IN1连续高，准备1 ms
→ 两路DRV8870执行1/1电气制动
→ 标记确认50 ms
→ 释放为0/0滑行
→ COMPLETE软件锁
```

## 5. 最终第二问烧录记录

权威交接：`docs/handoffs/2026-08-01-req002-right-wheel-brake-flash-complete.md`。

```text
探针：Horco CMSIS-DAP UID da941ca0
目标：MSPM0G3507
TI Pack：TexasInstruments.MSPM0G1X0X_G3X0X_DFP.1.3.1.pack
SWD：100 kHz
擦除：sector erase
输入：ELF
结束复位：--no-reset
pyOCD退出码：0
擦除：72704 bytes / 71 sectors
编程：72704 bytes / 71 pages
```

烧录后需由操作者按一次物理 RESET。不要运行第二次“一致性烧录”。

## 6. 历史已烧录版本

以下均为调试/调参历史，不再用于最终提交：

| SHA-256 前缀 | 用途 | 状态 |
|---|---|---|
| `2095D7E6...82CF` | 早期 MotorSelfTest 与 NF-02-PA 单向无线诊断 | 已烧录后又执行 MAIN 擦除；废弃 |
| `2B6F49CF...C854` | REQ-002 终点双轮制动基线/完整重测 | 已被后续弯道版本替代 |
| `AE4E1793...43A1` | 普通右弯 91/43、长急弯 | 废弃 |
| `3FA13468...8F39` | 普通右弯 88/42 | 废弃 |
| `1715F88C...0E05` | 全右弯四阶段状态机 | 废弃 |
| `0234C68B...B015` | 起点固定 85/43 直行尝试 | 实测起步直接右转，禁止使用 |
| `D67164B6...8F39` | RIGHT_CURVE_APPROACH 82/42，无单轮制动 | 最终版前身，已替代 |
| `BDE6EFE8...FEBAA` | 82/42＋严苛急弯右轮单独制动 | **最终第二问版本** |

## 7. 第1、3～6问相关但不可作为最终固件的项目

### K230 UART 接收模块

文件：`drivers/k230_link.c/.h`。

已实现固定 14 字节帧、CRC-8/ATM、STATUS/SEQ/POS/VEL/CONF/X/Y、超时和统计。它只是接收与诊断模块：

- 不包含 K230 摄像头端识别程序；
- 不覆盖视频图传、录像和回放；
- 不包含滚球 PID 或 D36A 控制；
- 不能单独完成第1、3、4、5、6问。

### nRF24 PTX

当前代码包含已批准的一次性 Baoqian 无线诊断 profile，历史上证明过 MCU→COM7 的单向文本链路。它不是视频链路，也不是滚球控制协议，不能替代第1问图传。

### D36A / 摆杆执行器

当前只有安全关闭逻辑：EN/DIR/STEP 保持低并检查禁用状态。工程中没有步进脉冲规划、限位、回零、方向闭合、位置环或滚球控制器。因此第3～6问没有可烧录完成件。

## 8. 提交前冻结与备份规则

1. 第二问只认大小 `630476 bytes`、SHA-256 `BDE6...FEBAA` 的 `MotorSelfTest/test1_motor_selftest.out`。
2. 不得把 `Debug/test1.out` 烧入第二问车辆。
3. 不得烧录任何历史 REQ-002 调参版本。
4. 最终提交前不得重新构建；重新构建会产生新文件，必须重新核对并视为新版本。
5. 不得修改 `app/req002.c`、`config/firmware_config.h`、`drivers/motor_driver.c/.h`、`.syscfg` 或生成文件。
6. 若目标板需要恢复，只按第5节固定参数烧录一次，成功后物理 RESET。
7. 第1、3～6问若没有外部独立工程或设备固件，不得声称本工作区已经提供对应烧录程序。

## 9. 证据索引

- 正式题目：`docs/requirements.md`
- 第二问最终冻结：`docs/handoffs/2026-08-01-req002-final-submission-freeze.md`
- 第二问最终烧录：`docs/handoffs/2026-08-01-req002-right-wheel-brake-flash-complete.md`
- 测试证据：`docs/test-log.md`
- K230 UART协议：`docs/protocols/k230-ball-position-uart-v1.md`
- K230模块交接：`docs/handoffs/2026-07-31-k230-module-code.md`

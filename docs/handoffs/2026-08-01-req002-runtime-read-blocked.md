# 交接：REQ-002 无复位状态读取被执行审批器阻塞

日期：2026-08-01

## 当前物理状态（用户确认）

- 12V 行走电机功率已物理断开。
- 开发板逻辑域与 Horco CMSIS-DAP 保持供电。
- KEY2 地面试验后未复位。
- 用户授权无复位、只读读取 REQ-002 状态。

## 已知固件

- `MotorSelfTest/test1_motor_selftest.out`
- SHA-256：`271DEFC2ADB90DCAF0A7D19C02941CF261991213764D6C1BABB1A0C539CA94D9`
- 已在上一会话成功烧录并由用户物理 RESET。

## 目标探针

- Horco CMSIS-DAP
- UID：`da941ca0`
- 只允许 Generic Cortex-M、100 kHz、`-M attach`，禁止复位和写入。

## 最新 map 中的 RAM 地址

- `req002.o (.bss.gStatus)`：`0x20200598`，长度 `0x78`
- `line_sensors.o (.bss.gSample)`：`0x202006F4`，长度 `0x24`
- `app.o (.bss.gStartButton)`：`0x20200744`，长度 `0x08`
- `motor_driver.o (.bss.gStatus)`：`0x2020074C`，长度 `0x06`
- `board_safety.o` 状态字节：`0x20200798`、`0x20200799`

## 阻塞

尝试在沙箱外启动 pyOCD commander 时，审批器因当前任务上下文过长拒绝；命令未启动，未访问探针，未读取内存，未复位、擦除或写入目标。

## 下一任务唯一动作

1. 读取本交接。
2. 在用户仍确认 12V 断开且未复位后，运行：
   `pyocd commander --no-config -u da941ca0 -t cortex_m -f 100k -M attach`
3. 只读上述 RAM 区域，记录原始字节后立即 `exit`。
4. 解析 REQ-002 state、blockReason、buttonAttempts、左右需求、controlSequence、传感器 valid/digitalBits/errorCount 与 BoardSafety 锁止状态。
5. 不修改源码、不构建、不烧录、不复位、不接通 12V。

# 交接：LineFollower I2C恢复镜像已构建，待新任务烧录

日期：2026-08-01

## 当前物理与授权状态

- 用户确认12V电机功率保持断开。
- 只允许使用 Horco CMSIS-DAP UID `da941ca0` 和指定 TI MSPM0G3507 Pack 烧录。
- 烧录后由用户物理 RESET。
- 只验证 LineFollower I2C恢复和KEY2启动门槛；不进行电机运动测试。

## 已实施修改

仅修改：

- `drivers/line_sensors.c`
- `drivers/line_sensors.h`
- `config/firmware_config.h`
- `firmware_tests/test_firmware.py`

内容：200 ms启动延时、开漏式9脉冲SCL恢复、STOP生成、I2C0 PinMux恢复、恢复诊断字段。未修改`empty.syscfg`、PID、REQ-002控制或电机方向。

## 验证

- `python -m unittest firmware_tests.test_firmware`：9/9 PASS。
- MotorSelfTest构建/链接：PASS，无编译警告。
- 镜像：`MotorSelfTest/test1_motor_selftest.out`
- 大小：554696 bytes
- SHA-256：`2B0FE6A200F1EB875E13A8A7941DBC228641510BCFA8C2816680CE9F9F90555C`

## 工具

- pyOCD：`C:\Users\Administrator\AppData\Local\Temp\codex-pyocd\bin\pyocd.exe`
- PYTHONPATH：`C:\Users\Administrator\AppData\Local\Temp\codex-pyocd`
- TI Pack：`C:\Users\Administrator\AppData\Local\Temp\TexasInstruments.MSPM0G1X0X_G3X0X_DFP.1.3.1.pack`

## 待执行唯一任务

1. 再次核对SHA-256。
2. `pyocd list --no-config`确认唯一探针UID `da941ca0`。
3. 使用 TI Pack、目标 `MSPM0G3507`、100 kHz、`-M halt`、sector erase、ELF格式烧录上述镜像；禁止软件复位，使用`--no-reset`或等价行为。
4. 只有出现明确擦除、编程和校验成功证据才能报告成功。
5. 用户物理RESET后，12V继续断开；等待至少500 ms。
6. 先只读确认 LineSensors `sequence>0`、`valid=true`、`lastSuccessMs>0`；若恢复发生，读取 `recoveryCount/recoveryFailureCount/lastBusLevels/lastRecoverySucceeded`。
7. 将传感器置于至少四路黑区域，按KEY2；OLED保持READY表示可能进入DEPART_A，FAULT表示门槛仍被拒绝。运动功率不得接通。

## 当前阻塞

当前任务的外部执行审批器因上下文过长，在 `pyocd list` 启动前拒绝操作。未枚举探针、未烧录、未复位或写入目标。必须在新任务中继续，不得在本任务绕过审批器。

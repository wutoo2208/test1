# 交接：REQ-002低速软启动镜像待烧录

日期：2026-08-01

## 用户确认的物理状态与授权

- 12V已物理断开，小车已停止。
- 允许使用Horco CMSIS-DAP UID `da941ca0`烧录新镜像。
- 烧录时禁止软件复位；成功后由用户物理RESET。

## 新控制参数

- 取消100%/120ms启动脉冲。
- 实际1kHz PWM软启动：0→600‰，300ms。
- 基础输出600‰。
- 最大输出800‰。
- 转向范围±150‰。
- 保留KEY2中止、I2C/丢线/超时/BoardSafety停车。

## 验证

- Python合同测试：9/9 PASS。
- MotorSelfTest构建/链接：PASS，无警告。
- 镜像：`MotorSelfTest/test1_motor_selftest.out`
- 大小：554216 bytes
- SHA-256：`50B6D86567CFB8B0F0BDF2CB4B3C49967D1784CB611B5F17F8F391AD4296BCE7`
- 本任务已再次核对哈希，完全匹配。

## 工具

- pyOCD：`C:\Users\Administrator\AppData\Local\Temp\codex-pyocd\bin\pyocd.exe`
- PYTHONPATH：`C:\Users\Administrator\AppData\Local\Temp\codex-pyocd`
- TI Pack：`C:\Users\Administrator\AppData\Local\Temp\TexasInstruments.MSPM0G1X0X_G3X0X_DFP.1.3.1.pack`

## 新任务唯一动作

1. `pyocd list --no-config`确认UID `da941ca0`。
2. 使用指定TI Pack、目标MSPM0G3507、100kHz、sector erase、ELF格式烧录上述镜像。
3. 使用`--no-reset`或等价无软件复位方式。
4. 必须取得明确擦除、编程、校验和退出码0证据。
5. 不修改代码、不构建、不接通12V。
6. 成功后只要求用户按一次物理RESET。

## 当前阻塞

当前任务的外部执行审批器因上下文过长，在`pyocd list`启动前拒绝。未连接探针、未烧录、未复位或写入目标。必须在新任务继续。

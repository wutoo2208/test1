# 交接：REQ-002 全右弯专用状态机烧录就绪

日期：2026-08-01  
会话任务：针对顺时针赛道全部为右弯，实现四阶段右弯控制，构建并烧录。  
任务状态：构建完成，烧录被长任务审批阻塞  
授权等级：L2 已完成；L3 命令未启动

## 已确认实现
- 工程：`C:\Users\Administrator\workspace_ccstheia\test1_2.2_recovered`。
- `RIGHT_CURVE_APPROACH`：右偏 `>=0.06` 连续 `10 ms` 进入；左 `820`、右 `450` permille，最长 `150 ms`。
- `RIGHT_CURVE_ARC`：右偏 `>=0.10` 连续 `5 ms`；左 `800`、右 `0`（滑行），最短 `300 ms`、最长 `700 ms`；回中 `<=0.04` 连续 `60 ms` 才退出。
- `RIGHT_CURVE_RECOVER`：`200 ms` 内左 `800→850`、右 `450→430`；若再次明显右偏，立即返回 ARC。
- ARC 中短时丢线继续右转最多 `100 ms`，随后按既有安全逻辑故障停车。
- 轮速 PI 仅在直线工作；所有右弯阶段关闭 PI。左向恢复转向力度由 `800` 降为 `400`。
- 修改：`app/req002.c`、`config/firmware_config.h`、`firmware_tests/test_firmware.py`。
- 静态单元测试：11/11 PASS；`git diff --check` PASS。
- CCS gmake `MotorSelfTest`：PASS；SysConfig 生成文件均报告 Unchanged。

## 待烧录产物
- 文件：`MotorSelfTest/test1_motor_selftest.out`
- 大小：`625736 bytes`
- SHA-256：`1715F88CE0B32E34671ABC6EA9D2287B8ED4EFFF6D0372C927D4271B7D970E05`

## 固定烧录参数
- Horco CMSIS-DAP UID：`da941ca0`
- pyOCD：`C:\Users\Administrator\AppData\Local\Temp\codex-pyocd\bin\pyocd.exe`
- PYTHONPATH：`C:\Users\Administrator\AppData\Local\Temp\codex-pyocd`
- TI Pack：`C:\Users\Administrator\AppData\Local\Temp\TexasInstruments.MSPM0G1X0X_G3X0X_DFP.1.3.1.pack`
- MSPM0G3507、SWD 100 kHz、sector erase、ELF、`--no-reset`。

## 验证与边界
- 证据等级：构建验证。
- pyOCD 在 CreateProcess 前因当前任务历史过长被审批系统拒绝；没有连接探针、擦除、编程或软件复位。
- NOT RUN：烧录、复位、串口和运动测试。

## 下一步唯一动作
- 新短任务读取本交接，核对 SHA-256 后按固定参数烧录；成功后让用户按一次物理 RESET。

# 交接：REQ-002 起点固定直行优化烧录就绪

日期：2026-08-01  
会话任务：修复 KEY2 起步时横向起点标记参与循迹导致的左靠，构建并烧录。  
任务状态：构建完成，烧录被长任务审批阻塞  
授权等级：L2 已完成；L3 命令未启动

## 已确认实现
- 工程：`C:\Users\Administrator\workspace_ccstheia\test1_2.2_recovered`。
- 根因：`DEPART_A` 阶段仍将横向四黑/六黑起点标记的加权位置送入循迹 PID；全黑位置约为 0，减去中心偏置 `0.0821` 后产生起步左向修正。
- 起点标记期间固定直行软启动：左 `850`、右 `430` permille，沿用 `300 ms` 软启动；禁用循迹修正、直线速度 PI 和右弯状态机。
- 离开起点标记并确认后继续固定直行 `100 ms`，随后交给循迹与全右弯状态机。
- 起点边缘短时丢线在既有 `100 ms` 故障确认窗口内继续固定直行，之后仍按安全逻辑故障停车。
- 全右弯参数保持不变：APPROACH `820/450`，ARC `800/0`、`300..700 ms`，回中 `60 ms`，RECOVER `200 ms`。
- 修改：`app/req002.c`、`config/firmware_config.h`、`firmware_tests/test_firmware.py`。
- 静态测试：11/11 PASS；`git diff --check` PASS。
- CCS gmake `MotorSelfTest`：PASS；SysConfig 生成文件均报告 Unchanged。

## 待烧录产物
- 文件：`MotorSelfTest/test1_motor_selftest.out`
- 大小：`629092 bytes`
- SHA-256：`0234C68B871151E3B843AC8175F7DCA57F987301536820FC5F3E1DF5FF09B015`

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

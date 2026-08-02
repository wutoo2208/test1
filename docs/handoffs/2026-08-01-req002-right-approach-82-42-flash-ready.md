# 交接：REQ-002 右弯入弯减速 82/42 烧录就绪

日期：2026-08-01  
会话任务：将 `RIGHT_CURVE_APPROACH` 调整为左 82%、右 42%，先构建后烧录。  
任务状态：构建完成，烧录被长任务审批阻塞  
授权等级：L2 已完成；L3 命令未启动

## 已确认事实
- 工程：`C:\Users\Administrator\workspace_ccstheia\test1_2.2_recovered`。
- 当前源码已回退“固定起步 85/43＋100 ms 交接”改动，保留全右弯四阶段状态机。
- 本版唯一新参数：`REQ002_RIGHT_APPROACH_LEFT_PERMILLE=820`，`REQ002_RIGHT_APPROACH_RIGHT_PERMILLE=420`。
- ARC 保持 `800/0`、最短 `300 ms`、最长 `700 ms`；回中 `60 ms`；RECOVER `200 ms`。
- 静态单元测试：11/11 PASS；`git diff --check` PASS。
- CCS gmake `MotorSelfTest`：PASS；SysConfig 生成文件均报告 Unchanged。

## 待烧录产物
- 文件：`MotorSelfTest/test1_motor_selftest.out`
- 大小：`625736 bytes`
- SHA-256：`D67164B64D85B25D8D0979C270C1D5CD3147B11E1EECF4DED0486317B0608F39`

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

# 交接：REQ-002 半圆弯道 88/42 固件烧录就绪

日期：2026-08-01  
会话任务：将普通右弯改为左 88%、右最低 42%，构建并烧录。  
任务状态：构建完成，烧录被长任务审批阻塞  
授权等级：L2 已完成；L3 命令未启动

## 已确认事实
- 工程：`C:\Users\Administrator\workspace_ccstheia\test1_2.2_recovered`。
- 普通右弯前馈：左 `880`、右最低 `420` permille；一侧转弯 PI 可给左轮增加最多 `50` permille。
- 急右弯保持左 `800`、右 `0` permille；单次 `350 ms`；回中 `<=0.05` 连续 `20 ms`；同弯最多两次；总窗口 `800 ms`。
- 修改：`app/req002.c`、`config/firmware_config.h`、`firmware_tests/test_firmware.py`。
- 单元测试：11/11 PASS。
- CCS gmake `MotorSelfTest`：PASS；SysConfig 生成文件均报告 Unchanged。

## 待烧录产物
- 文件：`MotorSelfTest/test1_motor_selftest.out`
- 大小：`622508 bytes`
- SHA-256：`3FA13468CD30BBF511792D9F7D65C27115C6017EEE9D7AD754DCAFA384930052`

## 固定烧录参数
- Horco CMSIS-DAP UID：`da941ca0`
- pyOCD：`C:\Users\Administrator\AppData\Local\Temp\codex-pyocd\bin\pyocd.exe`
- PYTHONPATH：`C:\Users\Administrator\AppData\Local\Temp\codex-pyocd`
- TI Pack：`C:\Users\Administrator\AppData\Local\Temp\TexasInstruments.MSPM0G1X0X_G3X0X_DFP.1.3.1.pack`
- 目标 MSPM0G3507、SWD 100 kHz、sector erase、ELF、`--no-reset`。

## 验证与边界
- 证据等级：构建验证。
- pyOCD 在 CreateProcess 前因当前任务历史过长被审批系统拒绝；没有连接探针、没有擦除、编程或软件复位。
- NOT RUN：烧录、复位、串口和运动测试。

## 下一步唯一动作
- 新短任务读取本交接，核对 SHA-256 后按固定参数烧录；成功后让用户按一次物理 RESET。

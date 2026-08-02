# 交接：REQ-002 严苛触发右轮单独制动烧录就绪

日期：2026-08-01  
会话任务：烧录严苛触发的右轮 DRV8870 1/1 单独制动固件。  
任务状态：构建与哈希核对完成，烧录被长任务审批阻塞  
授权等级：L2 已完成；L3 命令未启动

## 已确认实现
- 工程：`C:\Users\Administrator\workspace_ccstheia\test1_2.2_recovered`。
- `RIGHT_CURVE_APPROACH`：左 `820`、右 `420` permille。
- 严苛触发：右偏误差 `>=0.15` 且右转方向连续 `10 ms`，每个锁存右弯最多一次。
- 触发后：左 `800`；右轮先将 AIN1 连续置高并保持 AIN2 低 `1 ms`，随后 AIN1/AIN2=`1/1` 电气制动 `100 ms`；之后释放为右轮 `0%` 滑行并返回普通 ARC `800/0`。
- 制动期间循迹失效会立即释放右轮制动，再进入既有有界右向丢线处理。
- 新增驱动 API：`MotorDriver_prepareRightBrake`、`MotorDriver_engageRightBrake`、`MotorDriver_releaseRightBrake`。
- 静态单元测试：11/11 PASS；`git diff --check` PASS。
- CCS gmake `MotorSelfTest`：PASS；SysConfig 生成文件均报告 Unchanged。

## 待烧录产物
- 文件：`MotorSelfTest/test1_motor_selftest.out`
- 大小：`630476 bytes`
- SHA-256：`BDE6EFE861A40E0AA3DACCFC68093E32AB54C026FA8DA3B825545ECBB15FEBAA`
- 本会话烧录前重新核对，哈希完全匹配。

## 固定烧录参数
- Horco CMSIS-DAP UID：`da941ca0`
- pyOCD：`C:\Users\Administrator\AppData\Local\Temp\codex-pyocd\bin\pyocd.exe`
- PYTHONPATH：`C:\Users\Administrator\AppData\Local\Temp\codex-pyocd`
- TI Pack：`C:\Users\Administrator\AppData\Local\Temp\TexasInstruments.MSPM0G1X0X_G3X0X_DFP.1.3.1.pack`
- MSPM0G3507、SWD 100 kHz、sector erase、ELF、`--no-reset`。

## 验证与边界
- 证据等级：构建验证。
- pyOCD 在 CreateProcess 前因当前任务历史过长被审批系统拒绝；没有连接探针、擦除、编程或软件复位。
- NOT RUN：烧录、复位、串口、架空测试和赛道测试。

## 下一步唯一动作
- 新短任务读取本交接，核对 SHA-256 后按固定参数烧录；成功后让用户按一次物理 RESET。
- RESET 后首次必须架空短时触发，确认右轮制动而非反转、左轮保持正转且驱动无异常，再决定是否进入赛道。

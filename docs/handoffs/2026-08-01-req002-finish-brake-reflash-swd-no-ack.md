# 交接：REQ-002 终点电气制动固件重新烧录被 SWD No ACK 阻塞

日期：2026-08-01  
会话任务：重新核对指定 ELF 后，使用 Horco CMSIS-DAP 与指定 TI Pack 再次烧录 MSPM0G3507；禁止软件复位。  
任务状态：阻塞  
授权等级：L3（仅本次探针枚举与烧录尝试）

## 已确认事实
- 目标文件：`MotorSelfTest/test1_motor_selftest.out`，大小 `620928 bytes`。
- SHA-256：`2B6F49CF5358FD3A917F9FAAB91A97EC8B8B91045B5B88758FF48FDD18DEC854`，与用户指定值完全匹配。
- pyOCD 与 `TexasInstruments.MSPM0G1X0X_G3X0X_DFP.1.3.1.pack` 均存在。
- 只读 `pyocd list --no-config` 可识别 Horco CMSIS-DAP UID `da941ca0`，说明探针 USB 枚举正常。

## 本会话结果
- 使用 `MSPM0G3507`、SWD `100 kHz`、connect mode `halt`、`sector erase`、ELF、`--no-reset` 发起两次连接/烧录尝试。
- 两次均在 `DebugPortSetup` 阶段失败：`SWD/JTAG communication failure (No ACK)`，退出码均为 `1`。
- 两次失败日志均未进入 `Erasing` 或 `Programming`；本轮重新烧录没有开始写 Flash。
- 创建：`docs/handoffs/2026-08-01-req002-finish-brake-reflash-swd-no-ack.md`。
- 未修改：源码、`.syscfg`、生成文件、工程配置和构建产物。

## 验证与边界
- 证据等级：台架工具实测（探针枚举和 SWD 连接失败日志）。
- 未重新构建；未执行软件复位；未访问串口；未执行电机、制动或整车运动测试。
- 本轮不报告重新烧录成功，也不要求用户在当前状态下按物理 RESET 启动新烧录结果。

## 当前阻塞 / 待用户确认
1. 请在高功率负载断能、驱动轮架空的安全状态下确认目标板逻辑电源、探针 GND、SWDIO、SWCLK 和线缆连接可靠。
2. 若需通过物理 RESET、逻辑电源重上电或其他恢复方式后重试，需要用户明确告知已完成的现场动作并再次授权烧录。

## 下一步唯一动作
- 用户排除目标供电/SWD 物理链路问题并确认安全状态后，再以原参数执行一次烧录；不得自行改频率、连接模式或复位策略。

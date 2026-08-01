# 交接：OLED 与行走电机前进极性实测

日期：2026-08-01  
会话任务：用无线 CMSIS-DAP 完成 OLED 显示、MotorSelfTest 双轮方向与地面短点动验证  
任务状态：完成；正式 REQ-002 执行适配尚未实现  
授权等级：L5（仅两次 100%/120 ms 地面方向点动）

## 已确认事实
- OLED 接原 GY 接口并独占 I2C1：PB3 SDA、PB2 SCL/SCK。修复 `drivers/oled_ssd1306.c` 过早把 `BUSY=0` 判成完成的问题后，必须等待 `TX_DONE`、检查错误并等待总线释放；实物在 7 位地址 `0x3C` 正常显示。
- `algorithm/line_tracking.c` 和 `algorithm/pid.c` 的影子 PID 自 2.1 起存在且已由历史实物数据验证；当前 REQ-002 合同仍是观测型，`motionAuthorized=false`、`actuatorLocked=true`，不得声称已连接电机。
- DRV8870 当前车辆前进极性经地面实测冻结：左轮 B 通道使用反向模式（BIN2/PB24=1，BIN1/TIMA0 C2 使用补占空比）；右轮 A 通道使用 Primary 模式（AIN2/PB12=0，AIN1/TIMA0 C0 正常占空比）。相反组合“左 Primary + 右反向”在地面 120 ms 点动中确认向后；冻结组合在第二次 120 ms 点动中确认向前。
- `MotorDriver_stopAll()`、超时停止和显式 `stop` 后均读回：左右软件占空比 0、CC0/CC2=31999、PB12=0、PB24=0。
- 右轮硬件 QEI 可计数且换向后计数方向改变；左电机可转，但左编码器累计计数和无效跳变始终为 0，PB4/PB5 Capture 链路仍未闭合。
- LineFollower_6CH 当前能持续产生有效样本；A 点宽标记由用户确认是“六路中至少四路黑”，不是固定 `0x3F`。

## 本会话结果
- 修改：`drivers/oled_ssd1306.c/.h`、`app/app.c`、`drivers/motor_driver.c/.h`、`app/motor_test.c`、`drivers/diag_console.c`、`config/firmware_config.h`。
- 当前烧录镜像：`MotorSelfTest/test1_motor_selftest.out`，SHA-256 `754B7906BCFF71E29583239ADCB8AE7242D775CA1C1832F16C3F4367E995C965`，参数仅为临时 `1000‰/120 ms`。
- 未修改：`empty.syscfg`、影子 PID、六路标定数组、正式 REQ-002 执行门。

## 验证与边界
- 静态测试：`python -m unittest firmware_tests.test_firmware`，9/9 PASS。
- TI clang 5.1.1 LTS：MotorSelfTest 编译、链接成功；无线 CMSIS-DAP 烧录成功。
- 实测：OLED 点亮；双轮架空；两次受限地面方向点动；停止状态 SWD 回读。
- 当前物理状态：用户确认 12V 已断开；DAP 会话已退出。
- NOT RUN：PID 驱动电机、自动循迹、四黑标记停车、完整跑圈、带球测试。

## 当前阻塞 / 待确认
1. 左编码器仍为 0；若正式第二问不依赖双编码器，可作为后续独立维修任务。
2. OLED 当前应用层存在周期整屏刷新代码；接入 200 Hz 控制前应改为仅状态变化/完成时刷新，避免阻塞控制周期。
3. 临时 100%/120 ms 参数禁止直接带入正式循迹。

## 下一步唯一动作
- 由 Claude Code读取本交接，只新增“已验证 `shadowCorrection` → 冻结车辆前进极性”的最小执行适配设计；先保持执行门关闭并完成静态/构建审查，不直接地面跑圈。
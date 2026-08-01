# 交接：REQ-002右轮补偿与弯道增强镜像待烧录

日期：2026-08-01

## 本轮用户现场结论

- 相同PWM下右轮比左轮快，车辆产生向左偏航。
- 左轮编码器当前不可用，因此本轮不实施双轮速度闭环。
- 上一版弯道差速不足，车辆仍会向左冲出赛道。

## 本轮修改范围

仅修改：

- `app/req002.c`
- `config/firmware_config.h`
- `firmware_tests/test_firmware.py`

未修改：

- `empty.syscfg`
- `algorithm/line_tracking.c`
- PID增益
- 电机方向与Pin Plan
- I2C、OLED、K230、无线模块

## 新控制参数与行为

- 左轮直线基础需求：600‰。
- 右轮静态补偿：比左轮低40‰，直线基础需求560‰。
- 弯道动态降速：随 `abs(correction)` 将两轮基础需求最多降低150‰。
- 转向范围：由150‰提高到250‰。
- 最大输出：800‰。
- 软启动：0到目标需求，300 ms。
- 保留KEY2中止、LineFollower I2C故障停车、丢线停车、超时停车、返回A点停车和BoardSafety锁止。

软启动结束后的典型需求：

| correction | 左轮‰ | 右轮‰ |
|---:|---:|---:|
| -1.0（强右转） | 700 | 160 |
| -0.5 | 650 | 360 |
| 0 | 600 | 560 |
| +0.5 | 400 | 610 |
| +1.0（强左转） | 200 | 660 |

## 验证

- `python -m unittest firmware_tests.test_firmware`：9/9 PASS。
- MotorSelfTest构建/链接：PASS，未见编译或链接警告。
- SysConfig：NOT RUN / NOT MODIFIED。

## 新镜像

- 文件：`MotorSelfTest/test1_motor_selftest.out`
- 大小：554428 bytes
- SHA-256：`BF01F1F8125C2925C7D37E03248BA5AD5A35CB75E519732235F5A163520B3D9C`

## 当前状态和下一门槛

- 新镜像尚未烧录，板上固件不得视为本版本。
- 烧录前必须确认12V电机功率物理断开、小车停止。
- 获得明确L3烧录授权后，使用Horco CMSIS-DAP UID `da941ca0`、TI MSPM0G3507 Pack、100 kHz烧录；禁止软件复位，完成后由用户物理RESET。
- 地面测试属于L5。烧录和无12V逻辑检查完成后，测试时必须确保赛道清空、无球、D36A断电且物理断能可立即触及；出现反向、原地转、出线趋势或异常发热立即断开12V。

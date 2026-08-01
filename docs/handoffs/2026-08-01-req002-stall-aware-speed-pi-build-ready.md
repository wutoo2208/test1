# 交接：REQ-002 重载停轮可恢复速度 PI 已构建，待烧录

日期：2026-08-01

## 当前结论

- 左编码器 PB5 A 相 1 倍频在手转与线束晃动测试中连续计数，无效跳变为 0。
- 落地测试中左轮曾在约 0.54 s 后接近停转，右轮继续运动；当前源码此前把任一侧 10 ms 增量为 0 当作反馈无效并立即清空 PI，因此无法从重载停轮中恢复。
- 本轮继续采用速度 PI，不采用开环兜底，也未恢复 TIMA1 Capture。

## 已实现

- 单侧零速仍作为有效测量进入 PI：慢轮增扭、快轮减速。
- 双侧零速保持当前有界修正，不继续无依据积分。
- 连续缺失任一侧反馈 500 ms 后进入 `ENCODER_FEEDBACK_INVALID` FAULT。
- 直线目标比例保持左 `1.10`、右 `1.00`。
- PI 参数：Kp=4.0、Ki=12.0、输出限幅 ±200 permille、积分限幅 ±16。
- 基准左右均 600 permille，最大 800 permille。
- `Req002Status` 增加最后应用需求、最后/峰值 PI 修正、最大连续缺反馈时长和事件数，供停机后只读。

## 修改文件

- `app/req002.c`
- `app/req002.h`
- `config/firmware_config.h`
- `firmware_tests/test_firmware.py`

## 验证

- `python -m unittest firmware_tests.test_firmware`：11/11 PASS。
- MotorSelfTest SysConfig：PASS，仅既有 STOP/STANDBY retention info。
- MotorSelfTest 编译/链接：PASS，未见 warning。
- 输出：`MotorSelfTest/test1_motor_selftest.out`
- 大小：590344 bytes
- SHA-256：`53DDDB696B502EB86BEB87F9D49C0E7E45AA8F74D2422685531099E212F01EEF`
- 零左速、右侧约 17.2 counts/10 ms 的离线响应：10 ms trim +78‰；100 ms +98.4‰；250 ms +132.4‰；500 ms +189.2‰，对应约左 78.9%、右 41.1%。

## 最新 map

- `req002.o (.bss.gStatus)`：`0x2020045C`，长度 `0x8C`
- `gEncoderSpeedShadow`：`0x2020066C`，长度 `0x60`
- `gSpeedBalancePi`：`0x202007C4`，长度 `0x30`
- `gEncoderFeedbackMissingSinceMs`：`0x202008BC`
- `gSpeedBalanceTrimPermille`：`0x20200904`
- `gEncoderFeedbackFaulted`：`0x2020091D`
- `gEncoderFeedbackMissing`：`0x2020091E`

## 未执行

- 未烧录本轮新镜像。
- 未执行软件复位。
- 未进行架空或落地硬件验证。

## 下一步

1. 用户明确允许后，以 Horco UID `da941ca0`、TI Pack、100 kHz、sector erase、ELF、`--no-reset` 烧录上述镜像。
2. 用户物理 RESET。
3. 先双轮架空运行 0.5~1 s，KEY2 停止且不复位。
4. 无复位只读最新 map，确认峰值 PI 修正、反馈缺失时长、左右累计与是否触发编码器 FAULT；通过后再落地。

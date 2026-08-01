# 交接：左右轮10ms影子测速镜像待烧录

日期：2026-08-01

## 用户目标

直接执行第二步：不先闭环、不改PWM，在现有REQ-002运行时每10 ms同步读取左右累计编码器计数，判断右轮实际速度差和左Capture链路质量。

## 实施范围

修改：

- `drivers/encoders.c`
- `drivers/encoders.h`
- `empty.c`
- `firmware_tests/test_firmware.py`

保留此前已批准的K230 UART1配置。未修改：

- `app/req002.c`
- `config/firmware_config.h`中的70%/62%和转弯参数
- 循迹PID、方向、KEY2、FAULT状态机
- MotorDriver输出计算

## 影子测速行为

- 左轮PB4/PB5 Capture中断连续软件正交计数。
- 右轮PB10/PB11 TIMG8硬件QEI连续计数。
- 每10 ms在同一个全局中断屏蔽临界区读取两侧累计值。
- 只计算/记录，不调整PWM。
- 记录最新窗口、运动期间累计、双方同时有计数时的累计、峰值和左侧非法跳变。

## 验证

- Python合同测试：11/11 PASS。
- SysConfig生成：PASS；仅有既有STOP/STANDBY retention info。
- MotorSelfTest编译/链接：PASS，未见编译或链接警告。
- 烧录、复位、12V、运动和RAM实测：NOT RUN。

## 镜像

- 文件：`MotorSelfTest/test1_motor_selftest.out`
- 大小：581944 bytes
- SHA-256：`5D733D13B0741B4547E8748F43289D22F74B0739661B1F6100EBA546959AF59A`

## RAM影子测速结构

符号：`gEncoderSpeedShadow`

```text
base = 0x20200714
size = 0x60 bytes
```

所有字段为4字节小端；`leftDelta/rightDelta/leftCount`为有符号32位：

| 偏移 | 字段 |
|---:|---|
| 0x00 | sampleCount |
| 0x04 | lastSampleMs |
| 0x08 | windowMs |
| 0x0C | initialized |
| 0x10 | leftDelta |
| 0x14 | rightDelta |
| 0x18 | leftAbsDelta |
| 0x1C | rightAbsDelta |
| 0x20 | leftCount |
| 0x24 | rightCount |
| 0x28 | leftInvalidTransitions |
| 0x2C | leftInvalidDelta |
| 0x30 | leftState |
| 0x34 | rightState |
| 0x38 | rightDown |
| 0x3C | motionSampleCount |
| 0x40 | bothMotionSampleCount |
| 0x44 | leftAbsSum |
| 0x48 | rightAbsSum |
| 0x4C | leftBothAbsSum |
| 0x50 | rightBothAbsSum |
| 0x54 | leftAbsPeak |
| 0x58 | rightAbsPeak |
| 0x5C | invalidDuringMotion |

停止后最后窗口可能为0，但累计和峰值字段会保留到复位。

若两只MG513X编码器的实际CPR一致，可主要比较：

```text
rightBothAbsSum / leftBothAbsSum
```

若右侧累计增加而左侧为0，左Capture链路未工作。若`invalidDuringMotion`快速增加，优先查高速漏边沿、噪声或相序。

## 建议L4测试流程

1. 12V断开，双轮架空，钢球取下，D36A断电，物理断能可触及。
2. 烧录指定镜像，禁止软件复位，由用户物理RESET。
3. 将LineFollower保持在可通过KEY2启动的A点/至少四黑位置。
4. 接通12V，按KEY2启动；运行约0.5～1 s后再次按KEY2中止。
5. 物理断开12V。
6. Generic Cortex-M、100 kHz、无复位attach，只读`0x20200714/0x60`后立即退出。
7. 禁止地面行驶；本测试只判断编码器速度比例和Capture质量。

## 本会话烧录尝试结果

- 用户已明确授权烧录、物理RESET、架空0.5～1秒测试以及停止后只读RAM。
- 烧录前重新核对SHA-256：`5D733D13B0741B4547E8748F43289D22F74B0739661B1F6100EBA546959AF59A`，完全匹配。
- 外部审批器在`pyocd list --no-config`启动前因当前任务上下文过长拒绝执行。
- 探针：NOT ACCESSED。
- 擦除/编程/校验：NOT RUN。
- 软件复位：NOT RUN。
- 电机动作：NOT RUN。
- 板上固件不得视为影子测速版本。
- 下一新任务只需读取本交接，按既定参数烧录；成功后让用户物理RESET，再执行架空短时运行和`0x20200714/0x60`只读。
# REQ-002 终点电气制动固件：烧录因长上下文审批阻塞

## 唯一后续任务

核对哈希后，使用 Horco CMSIS-DAP UID `da941ca0`、TI MSPM0G3507 Pack、SWD 100 kHz、sector erase、ELF、`--no-reset`，烧录：

```text
MotorSelfTest/test1_motor_selftest.out
```

必须匹配：

```text
SHA-256: 2B6F49CF5358FD3A917F9FAAB91A97EC8B8B91045B5B88758FF48FDD18DEC854
Size: 620928 bytes
```

本会话烧录审批因任务历史过长被系统拒绝；pyOCD未启动，目标板未写入该镜像。不得把本次失败报告为烧录成功。

## 工具

```text
pyOCD: C:\Users\Administrator\AppData\Local\Temp\codex-pyocd\bin\pyocd.exe
PYTHONPATH: C:\Users\Administrator\AppData\Local\Temp\codex-pyocd
Pack: C:\Users\Administrator\AppData\Local\Temp\TexasInstruments.MSPM0G1X0X_G3X0X_DFP.1.3.1.pack
UID: da941ca0
```

命令参数：

```text
load --no-config -u da941ca0 -t MSPM0G3507 -f 100k -M halt -e sector --pack <Pack> --format elf --no-reset <ELF>
```

烧录成功后让用户按一次物理 RESET；不要软件复位，不要第二次一致性烧录。

## 新固件行为

- 普通循迹和急右弯 80/0 状态机保留。
- 终点首次检测后进入 DRV8870 两阶段制动：
  1. 两路 IN1 强制高，等待 1 ms（当前 1 kHz PWM 的一个周期）；
  2. AIN2/BIN2 拉高，进入 DRV8870 `1/1` 电气制动；
  3. 使用现有 50 ms 终点确认窗口；
  4. 确认后释放为双低 `0/0` 滑行并进入 COMPLETE 软件锁。
- 制动时若循迹数据瞬时无效，不允许丢线恢复PWM覆盖制动；确认窗口结束后释放并报告故障。

## 验证

```text
python -m unittest firmware_tests.test_firmware: 11/11 PASS
CCS gmake MotorSelfTest: PASS
SysConfig generated files: Unchanged
git diff --check (relevant files): PASS
```

## 当前板上固件

最后一次成功烧录的是急右弯 80/0 版本：

```text
97F4DC2A1B324A9664AC344B1EBF56549AFE611FAD119839CDA8D3D4B8CD6477
```

终点电气制动版本 `2B6F49...EC854` 尚未烧录。

## 首次硬件验证门槛

首次制动验证必须双轮架空、低速、可立即物理断能，观察是否有反转脉冲、异常电流、撞击或驱动板发热。未获新授权前不执行运动验证。
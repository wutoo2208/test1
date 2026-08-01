# 交接：上一版循迹镜像烧录待执行

日期：2026-08-01

## 用户目标和物理状态

- 用户要求先烧录上一版循迹代码，再报告写入地址。
- 用户此前明确当前未接电池，因此按12V电机功率断开处理。
- 本次不重新构建，不让尚未生成的K230 UART配置进入镜像。

## 唯一允许烧录的镜像

- 文件：`MotorSelfTest/test1_motor_selftest.out`
- 大小：554584 bytes
- 生成时间：2026-08-01 11:11:48.192 +08:00
- SHA-256：`38401822A6FE35CBA1E603A558A44230E1A896CC4AA3DEA62B4F91D2EF34E4DE`
- 本会话重新计算哈希：完全匹配。

## ELF Flash布局

- `.intvecs`：0x00000000，长度0xC0
- `.text`：0x000000C0，长度0xDFF0
- `.rodata`：0x0000E0B0，长度0x0C88
- `.cinit`：0x0000ED38，长度0x30
- 有效Flash内容范围：0x00000000～0x0000ED67
- 预计sector/page编程覆盖可能对齐到0x0000EFFF；必须以实际pyOCD输出为证据。

RAM `.bss/.data` 不属于Flash烧录地址。

## 当前阻塞

- 外部审批器在 `pyocd list --no-config` 启动前因任务上下文过长拒绝执行。
- 探针：NOT ACCESSED
- 擦除：NOT RUN
- 编程：NOT RUN
- 校验：NOT RUN
- 软件复位：NOT RUN
- 板上固件不得视为已更新。

## 下一新任务唯一动作

读取本交接，重新核对上述哈希，枚举Horco CMSIS-DAP UID `da941ca0`，使用TI MSPM0G3507 Pack、100 kHz、sector erase、ELF格式和`--no-reset`烧录指定镜像。必须报告实际擦除/编程字节数、地址范围、校验结果和退出码；成功后由用户物理RESET。

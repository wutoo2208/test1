# REQ-002 终点制动测试后串口读取：长上下文审批阻塞

## 当前现场状态（用户陈述）

- 用户称终点电气制动版本已烧录并完成新测试。
- 测试完成后未复位。
- 用户明确允许COM8串口只读。
- 当前任务因历史过长，审批系统在命令启动前拒绝COM8访问；串口尚未打开，数据尚未读取。

## 唯一后续任务

使用 `COM8`、`115200 8N1`，DTR/RTS关闭，只发送以下只读诊断命令并读取响应：

```text
req002
track
enc
line
status
```

不发送 `stop`、电机测试、radio命令；不复位、不烧录、不写内存、不修改代码。

重点解析：

- REQ-002 state / blocked / last_left / last_right / speed trim / elapsed；
- TRACK position/error/shadow；
- 六路数字和模拟值；
- 编码器计数和缺失反馈；
- 是否进入 COMPLETE。

## 当前新源码确认存在的功能

急右弯状态机已经加入：

```text
触发：centeredError >= 0.15，右转方向连续10 ms
输出：左80%，右0%
最长：100 ms
退出：centeredError <= 0.05
```

终点停车状态机已经加入：

```text
终点首次检测 → IN1高准备1 ms → DRV8870双轮1/1制动
现有终点确认窗口共50 ms → 释放到双低0/0 → COMPLETE软件锁
```

待烧录构建的哈希为：

```text
2B6F49CF5358FD3A917F9FAAB91A97EC8B8B91045B5B88758FF48FDD18DEC854
```

用户称已经烧录并运行，但串口读取前不把板内版本视为工具验证事实。

## 注意

当前Req002Status没有记录 `sharp_turn_events`，因此若最后输出不是80/0，无法仅凭现有状态证明急弯此前从未短暂触发。只能确定最后一次应用输出以及最终状态。
# 交接：无线 SPI0 Pin Plan v1.2 冻结收尾

日期：2026-07-30
会话任务：将用户批准的无线 SPI0 Pin Plan v1.2 生成并同步为当前唯一冻结设计
任务状态：完成
授权等级：L1（仅文档）

## 已确认事实

- 用户已批准无线 SPI0 Pin Plan v1.2，但未授权修改 SysConfig、接线、上电、构建、烧录或硬件测试。
- 当前唯一 MCU/Timer/PinMux 资源基线为 `docs/pin-plan/mspm0g3507-pin-plan-frozen-v1.2.md`；当前唯一逐线接线矩阵为 `docs/pin-plan/mspm0g3507-adapter-harness-v1.2.md`。
- 无线 SPI0 使用 PB25/CSN、PB17/MOSI、PA12/SCK、PB19/MISO，另用 PB1/CE 和 PB16/IRQ；PB18 由用户确认连接 KEY3，标记为 `FORBIDDEN-FOR-RADIO`。
- TCRT OUT1/OUT2 使用 PA25/PA27，OUT3/OUT4/OUT5 使用 PA16/PA14/PB20；必须采用 v1.2 交叉线束，v1.0 连续 pin1-pin5 方案作废。
- MS42CG A/B/PWM/Z 使用 PA29/PA30/PA13/PB23；PWM 的 TIMG0 单输入双边沿 Capture 仍需未来 SysConfig 隔离验证。

## 本会话结果

- 创建/确认：`docs/pin-plan/README.md`
- 创建/确认：`docs/pin-plan/mspm0g3507-pin-plan-frozen-v1.2.md`
- 创建/确认：`docs/pin-plan/mspm0g3507-adapter-harness-v1.2.md`
- 更新历史状态：v1.0 Pin Plan、旧 harness、收敛候选、无线 v1.1/v1.2 候选。
- 同步更新：`docs/hardware-interface.md`、`docs/hardware/module-questions.md`、`docs/project-state.md`、`docs/decisions.md`。
- 新增决策 `DEC-013`、`DEC-014`，指定 v1.2 两个权威文件为每个模块的唯一最新版依据。
- 未修改：`.syscfg`、源码、CCS 工程、生成文件和构建产物。

## 验证与边界

- 证据等级：用户确认 + 资料复核 + SDK IOMUX 静态核验。
- 静态检查：v1.2 Owner 表共 35 个 MCU 引脚，无重复 Owner；无线 SCK 仅为 PA12；PB18 明确禁止用于无线；TCRT 与 MS42CG 迁移端点一致；主文档均指向 v1.2。
- `git diff --check -- docs` 通过；工作区无 `docs/` 之外的修改。
- NOT RUN：SysConfig、构建、链接、烧录、XDS110、串口、SPI、接线、上电、电机或其他硬件动作。

## 当前阻塞 / 待用户确认

1. 无线 pin1/pin2 的权威功能、供电和地定义。
2. 无线 IRQ 输出结构、无效电平和上拉需求；SPI mode、频率、对端与包协议。
3. TCRT 输出电平、结构、有效极性、物理左右顺序和交叉线束连续性。
4. MS42CG PWM 周期/占空比/最高边沿频率，以及 TIMG0 捕获方案的 SysConfig 可生成性。

## 下一步唯一动作

- 只有用户另行授权后，才能领取上述某一个阻塞项的资料核验或 SysConfig 隔离验证；不得自动进入接线、上电或固件实施。

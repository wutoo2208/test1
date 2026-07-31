# LineFollower_6CH 资料与 Pin Plan v1.5 候选交接

> **历史状态**：`SUPERSEDED`。用户随后批准 U12/I2C0 LineFollower + 原MPU接口/I2C1 OLED + MPU NOT FITTED；当前结论见 frozen v1.5 和 2026-07-31 最终交接，不得按本候选施工。

- 日期：2026-07-31
- 任务：以 HiWonder/AiBlock LineFollower_6CH V1.0 替换原五路 TCRT5000，完成 PDF→知识库并建立新 Pin Plan 候选。
- 协作边界：Claude Code 正在修改固件、SysConfig、Debug 生成物、`docs/firmware-v1.3.md` 和 v1.4 修订；本任务未编辑这些文件，也未构建/烧录/访问硬件。

## 已完成

1. 归档 4 份 PDF、2 张尺寸图到 `docs/source-pdf/`；文本、全部页面渲染和 contact sheet 位于 `docs/extracted/linefollower-6ch/`（均本地忽略）。
2. 创建：
   - `docs/reviewed/linefollower-6ch-pdf-review.md`
   - `docs/hardware/modules/linefollower-6ch-i2c.md`
3. 创建未冻结候选：
   - `docs/pin-plan/mspm0g3507-pin-plan-linefollower-6ch-v1.5-candidate.md`
   - `docs/pin-plan/mspm0g3507-adapter-harness-linefollower-6ch-v1.5-candidate.md`
4. 更新架构、主计划、接口、决策、项目状态、测试记录和模块索引；旧 TCRT 文档保留为历史证据。

## 关键事实

- 模块：5 V、资料 85 mA、4 线 I2C、7 位固定地址 0x5C。
- 原理图：5 V 经 LDO 到 VDD33；SDA/SCL 通过 R1/R2 上拉 VDD33，阻值未标。
- 数字状态：0x05，1 byte，bit0..5 对应通道 1..6。
- 模拟数据：0x06 起连续 12 bytes，六个 uint16 little-endian。
- 阈值寄存器有文档冲突：表格 18/20/... 与示例基址 0x18 不一致，当前禁止依赖。
- H10 pin1..8 原网络中没有完整硬件 I2C SCL/SDA 对；PA16 只有 SDA 能力，无配对 SCL。
- 候选共享 I2C1 PB2/PB3，与 MPU6050 地址不冲突，但上拉/总线负载未验证。

## 等待用户选择

- A：推荐，无切线。H10 pin9/10 供电，SDA/SCL 走 GY_SDA/GY_SCL Y 线。
- B：单一 H10 插头。隔离 H10 pin3/pin4 与 PA16/PA14，再桥接 PB3/PB2；需要拓展板 H10 正反面照片、切线点审批和断电连续性。

选择后才能生成 frozen v1.5；此前不修改 SysConfig、不接线、不切板、不释放旧 TCRT GPIO 给其他功能。

## NOT RUN

SysConfig、源码修改、构建、烧录、串口/I2C、接线、上电、传感器学习、200 Hz 调度和赛道测试均未运行。

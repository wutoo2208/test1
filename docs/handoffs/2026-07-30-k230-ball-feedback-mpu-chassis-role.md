# K230 球位置反馈与 MPU6050 车体职责变更交接

- 日期：2026-07-30
- 范围：仅方案文档；未修改 SysConfig、源码、Pin Plan、接线或硬件状态。
- 决策：MPU6050 不再安装于摆杆或测量杆角；K230 派生的钢球相对 O 点位置作为球位置外环唯一测量；D36A+步进电机+MS42CG 承担摆杆执行器内层；MPU6050 改为车体循迹扰动/角速度/加速度辅助候选。
- Pin Plan：frozen v1.2 端点不变，本轮不修改 v1.2/v1.3 Pin Plan。

## 已更新

- `docs/architecture.md`
- `docs/decisions.md`（新增 `DEC-015`）
- `docs/h-task-master-plan.md`
- `docs/hardware-interface.md`
- `docs/hardware/modules/k230-target-tracking.md`
- `docs/hardware/modules/mpu6000a.md`

## 必须关闭的阻塞

1. K230 完整帧格式、frame_id/捕获时间、帧率、端到端延迟、抖动和丢帧。
2. `x/y/w/h` 到沿杆轴有符号毫米位置的标定、O 点、方向和 1 cm 验收。
3. 球速估计、视觉异常值和超时策略。
4. MS42CG 零位、A/B/PWM/Z 使用、计数到杆角/右端高度的几何映射和丢步策略。
5. D36A/步进电机限位、回差、步频、加速度、失能和安全角度。
6. 车体 MPU6050 安装轴向、零偏、振动和加入循迹辅助前后的 AB 对照收益。

## NOT RUN

SysConfig、构建、烧录、串口、接线、上电、执行器与运动测试均未运行。Claude Code 的代码与生成文件未触碰。

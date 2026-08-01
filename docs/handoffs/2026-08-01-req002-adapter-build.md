# 交接：REQ-002 最小执行适配已构建，待新会话烧录验证

日期：2026-08-01  
会话任务：将已验证 `shadowCorrection` 接入冻结车辆前进极性，并保持默认 Debug 锁止  
任务状态：源码、测试、Debug/MotorSelfTest 构建完成；烧录未完成  
授权等级：L2 已完成；L3 被当前工具上下文审批阻塞

## 已确认事实
- `algorithm/line_tracking.c`、`algorithm/pid.c`、六路标定数组和 `empty.syscfg` 未修改。
- KEY2/PA21 是唯一正常启动入口；运行中再次按 KEY2 调用 `Req002_abort()` 停车。
- 车辆前进极性沿用地面实测冻结结果：左轮 BIN2/PB24 高、BIN1/TIMA0 C2 补占空比；右轮 AIN2/PB12 低、AIN1/TIMA0 C0 正常占空比。
- A 点宽标记为六路中至少四路黑；离开标记 50 ms 后进入 LAP_ACTIVE，再次四黑立即停车并确认 50 ms 后完成。
- 传感器无效、I2C/采样超时、信号不足、丢线或 20 s 总超时均调用 BoardSafety 停车。
- 左编码器仍不计数，REQ-002 不依赖编码器。

## 本轮实现
- `config/firmware_config.h`：默认 `MOTOR_SELFTEST_BUILD=0`；`REQ002_ACTUATION_BUILD` 仅随 MotorSelfTest 开放。初始架空候选参数：5 ms 控制、120 ms 启动脉冲、650‰基础脉冲密度、350‰转向范围。
- `drivers/motor_driver.c/.h`：新增连续 `MotorDriver_setVehicleForwardDuties(left,right)`，保留四输入归零停止。
- `app/req002.c/.h`：接入 `steeringCorrection`、KEY2 状态机、四黑 A 点、脉冲密度差速、故障停车、完成计时。
- `app/app.c`：传入 `lineTracking->shadowCorrection`；OLED 取消每 500 ms 整屏刷新，只在 READY/安全锁止/完成/故障状态刷新。
- `drivers/diag_console.c`：`stop` 同时中止 Req002；状态增加左右需求和控制序号。
- `firmware_tests/test_firmware.py`：更新隔离执行和安全停车合同。

## 验证
- `python -m unittest firmware_tests.test_firmware`：9/9 PASS。
- Debug 编译/链接：PASS，SHA-256 `70021A061E0DCB2ECB495D85D87D55853D80775F7C2DB9D6776CF463D6469648`。
- MotorSelfTest 编译/链接：PASS，SHA-256 `271DEFC2ADB90DCAF0A7D19C02941CF261991213764D6C1BABB1A0C539CA94D9`。
- SysConfig：NOT RUN / NOT MODIFIED。
- 烧录：FAILED BEFORE PROGRAMMING，pyOCD 仅输出 `Error: [__main__]`，无擦除/编程进度；不得认为新镜像已在板上。
- 只读枚举：Horco CMSIS-DAP、UID `da941ca0`、COM8 均存在；后续 attach 被当前任务上下文过长导致的审批系统拒绝，要求新会话继续。
- 当前物理状态：用户最后确认 12V 已断开；板上仍按旧 120 ms MotorSelfTest 镜像处理。

## 下一步唯一动作
- 新会话读取本交接，在 12V 断开条件下：先 `pyocd list`，再 Generic Cortex-M 无复位 attach；成功后退出并使用 TI Pack 烧录 `MotorSelfTest/test1_motor_selftest.out`。物理 RESET 后只做逻辑域/KEY2 无功率验证，未另行授权前不接通 12V。
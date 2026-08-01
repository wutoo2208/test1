# 交接：左编码器GPIO 1倍频与直线速度PI已构建，待断电逻辑域验证

日期：2026-08-01  
会话任务：将左轮PB4/PB5从TIMA1 Capture改为GPIO软件编码器，并在REQ-002直线段加入双轮归一化速度PI  
任务状态：源码、正式SysConfig生成、Debug/MotorSelfTest构建完成；未烧录或实测  
授权等级：L2完成；L3/L4未授权执行

## 已确认事实
- `empty.syscfg`中PB4/PB5无当前资源冲突：PB5为上升沿GPIO中断输入，PB4为方向输入；右轮PB10/PB11继续使用TIMG8 QEI。
- 正式生成宏为`DIAG_GPIO_LEFT_ENCODER_A/B_*`，GPIOB中断为`DIAG_GPIO_INT_IRQN`/`DIAG_GPIO_INT_IIDX`；应用入口为`GROUP1_IRQHandler`。
- 左轮使用接在PB5的软件A相上升沿1倍频，PB4软件B相判断方向；直线速度比较将左轮10ms计数乘4后与右轮硬件QEI计数比较。
- 先前TIMA1 Capture架空测试左计数为0；后发生杜邦线冒烟和并行烧录，旧结果不能证明GPIO方案或当前硬件状态。

## 本会话结果
- `drivers/encoders.c/.h`：移除TIMA1 Capture运行依赖；增加PB5 GPIO计数、PB4方向读取、GPIO中断入口和shadow快照API。
- `empty.c`：使用官方GROUP1/GPIOB分发方式调用左轮A相ISR。
- `app/req002.c`：复用`PidController`增加直线速度PI；软启动结束、转向绝对值不超过0.10、需求不低于250‰且两轮有新脉冲时才更新；缺脉冲、转弯、停机和故障时复位积分。
- `config/firmware_config.h`：初始候选为左计数×4、Kp=1.0、Ki=8.0、输出限幅±80‰、积分限幅±10。参数未台架标定。
- `firmware_tests/test_firmware.py`：增加GPIO 1倍频、GROUP1入口、归一化和PI安全门合同。
- 未手工编辑生成文件；`Debug/**`和`MotorSelfTest/**`仅由SysConfig和CCS gmake正式生成/构建。

## 验证与边界
- 隔离与正式SysConfig 1.26.2：PASS；仅3条既有SPI/QEI/PWM STOP/STANDBY retention info，无warning。
- `python -m unittest firmware_tests.test_firmware`：11/11 PASS。
- Debug编译/链接：PASS，未见warning；`Debug/test1.out`大小556004 bytes，SHA-256 `31C66A2FDE6826792DED87E98962B7DD1F39C6074BB58C253B069870CB9C8EC8`。
- MotorSelfTest编译/链接：PASS，未见warning；`MotorSelfTest/test1_motor_selftest.out`大小585572 bytes，SHA-256 `48B705FFE2837E840A403B411F3B4EBEA1747D38A60589C123354E4B74DF77C3`。
- Map确认`GROUP1_IRQHandler`、`Encoders_onLeftEncoderAInterrupt`、MotorSelfTest中的`Encoders_speedShadowSnapshot`和`Pid_step`已链接；TIMA1仅保留启动文件弱默认向量，不再有LEFT_CAPTURE配置或应用ISR。
- 证据等级：构建验证。
- NOT RUN：烧录、探针、串口、逻辑域上电、手动转轮、12V、架空运动、地面测试、速度PI调参。

## 当前阻塞 / 待用户确认
1. 杜邦线冒烟后的线束、供电、PB5/PB4电平及编码器模块健康状态尚未复核；复核前禁止通电运动测试。
2. PI参数仅为保守初始候选，不得写成已调好或已实现等速实测。

## 下一步唯一动作
- 获得L3只读/逻辑域授权且硬件冒烟原因已排除后，先不接12V，手动转动左轮并通过GPIO计数RAM或逻辑分析仪确认PB5有计数、PB4方向变化；通过后再单独申请L4架空低功率PI测试。


## PB5架空实测补充（2026-08-01）
- 新PB5计数镜像已烧录并物理RESET；LineFollower为`0x3F`，KEY2启动/中止有效。
- REQ-002实际运行约4075 ms，`controlSequence=816`，停止后为`SAFE_LOCKED`。
- 右轮QEI：`rightCount=17290`、`rightAbsSum=17298`、`rightAbsPeak=541`。
- 左轮PB5 GPIO 1x：`leftCount=-3`、`leftAbsSum=1`、`leftAbsPeak=1`；仅1个同时计数窗口。
- 用户确认左轮实物持续转动。PB4计数版本和PB5计数版本均几乎无边沿，因此阻塞已从MCU引脚/软件方案收敛为左编码器供电、共地、线束、通道定义或编码器模块本身问题。
- 不得继续调整Kp/Ki、归一化比例或进入地面测试；先在编码器端和MCU端分别测量A/B波形。

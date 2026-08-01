# REQ-003 静止摆球定位控制算法候选方案

> 日期：2026-08-01  
> 状态：`CANDIDATE DESIGN / DOCUMENT ONLY / NOT IMPLEMENTED / NOT TESTED`  
> 范围：只设计第三问；不修改、复用或重构第二问 `REQ-002` 的控制代码、参数和 KEY2 状态机。

## 1. 正式目标

第三问（REQ-003）的正式验收目标：

1. 小车保持静止；
2. 钢球从摆杆中心点 `O` 运动至 `+5 cm`；
3. 到达后折返，运动至 `-5 cm` 并稳定；
4. 全过程完成时间 `<= 5 s`；
5. 在 `+5 cm` 和 `-5 cm` 处最大误差绝对值 `<= 1 cm`。

本题启动按键按用户决定使用：

```text
KEY3 = PB18
```

KEY3 的实际有效电平和消抖参数必须从开发板原理图、现有生成配置或实物读取验证，不在本文猜测。

## 2. 与第二问的严格隔离

第三问采用独立应用状态机和独立模块，不修改第二问的循迹、电机方向、PWM、KEY2、A 点检测或停车逻辑。

建议未来代码边界：

```text
app/req003.c/.h                     第三问状态机
algorithm/ball_position_control.c/.h 钢球位置外环
algorithm/beam_angle_control.c/.h    摆杆角度/执行器内环
drivers/d36a_stepper.c/.h            STEP/DIR/EN和脉冲调度
drivers/ms42cg.c/.h                  唯一活动编码器
 drivers/key3_button.c/.h             KEY3独立消抖（不改KEY2模块）
```

可以只读复用：

```text
drivers/k230_link.c/.h               K230位置/速度协议解析
bsp/timebase.c/.h                     系统时间
OLED显示接口                         显示状态、位置和计时
```

模式互斥：

```text
REQ-002运行：REQ-003锁止，D36A禁止动作
REQ-003运行：车轮输出强制为0，REQ-002不运行
```

第三问不得同时启动底盘电机。

## 3. 编码器资源策略

用户已明确提醒：MSPM0G3507 同一时刻只按一个编码器采样链路设计。第三问直接避开资源冲突：

```text
第三问唯一编码器Owner = MS42CG
左右车轮编码器          = 不初始化、不采样、不分时复用
```

首版建议只连续使用 MS42CG 的 A/B 增量信号：

```text
A = PA29
B = PA30
```

MS42CG 的 PWM 和 Z 在首版中只保留，不与 A/B 同时建立另一套连续捕获链。原因：

- 摆杆工作范围很小，Z 每转一次，在小角度范围内通常无法提供工作零点；
- PWM绝对角度捕获需要额外Timer资源和实物时序验证；
- A/B连续计数更适合监测步进电机是否真实跟随命令；
- 可以在后续单独的标定模式中再评估PWM绝对角度，不影响首版闭环。

手册示例的1000线/4000计数每圈不能直接作为实物常数；必须实测确认。

## 4. 实物机构模型

照片显示执行链大致为：

```text
步进电机轴
   ↓
曲柄/摇臂
   ↓
连杆
   ↓
抬升或降低摆杆自由端
   ↓
摆杆绕另一端铰链小角度转动
```

必须区分：

```text
MS42CG测量量 = 电机轴角度
真正控制量   = 摆杆角度
```

两者由曲柄长度、连杆长度、连接点和装配零位共同决定，可能非线性并存在回差。因此不直接使用固定比例，建议建立标定表：

```text
MS42CG计数 → 摆杆自由端高度 → 摆杆角度
```

若摆杆铰链到执行点的有效长度为 `L`，自由端相对水平位置的高度变化为 `Δh`，则几何候选为：

```text
beam_angle = asin(Δh / L)
```

实际控制使用分段线性查表：

```text
encoder_count → beam_angle_deg
```

至少标定：负限位附近、负中间、水平零点、正中间、正限位附近。正反方向分别记录，以评估连杆回差。

用户给出的摆杆偏转范围为大约不超过 `±15°`。本文将它作为机械/软件绝对保护候选，而不是初次调试的工作角度。建议调试顺序：

```text
无球：±1° → ±3° → ±5°
有球：从±2°或更小开始
确认仍不足后才逐级放宽
绝对禁止超过经机械确认的±15°边界
```

## 5. 坐标和方向闭合

统一变量：

```text
x_ball       K230输出的钢球位置，单位mm
v_ball       K230输出的钢球速度，单位mm/s
x_ref        当前目标位置：+50 mm或-50 mm
theta_beam   实际摆杆角度，单位deg
theta_ref    外环给出的目标摆杆角度，单位deg
motor_count  MS42CG相对零位计数
```

K230协议已有：

```text
+5 cm = +500（0.1 mm单位）
-5 cm = -500（0.1 mm单位）
```

以下三个符号必须通过无球/低角度台架试验闭合，禁止猜测：

1. K230的正方向对应摆杆哪一端；
2. D36A DIR1哪一电平使摆杆自由端升高；
3. MS42CG计数增加对应电机和摆杆哪个方向。

在方向闭合之前，外环输出必须保持锁止。

## 6. 两层控制结构

### 6.1 钢球位置外环

外环只在收到新的、有效的K230帧时更新，不用200 Hz循环反复处理同一帧。

候选控制律：

```text
e_x = x_ref - x_ball

theta_candidate = direction_sign × (
    Kp_x × e_x
  - Kd_x × v_ball
  + Ki_x × integral(e_x)
)

theta_ref = clamp(theta_candidate, -theta_work_limit, +theta_work_limit)
```

首轮调试建议：

- 先用 `PD`，令 `Ki_x = 0`；
- 先调 `Kp_x` 使球能朝目标移动；
- 再增加 `Kd_x` 抑制到点时的速度和超调；
- 只有在 `-5 cm` 最终保持存在稳定静差时，再加入很小的积分；
- 积分只在接近最终目标、测量有效且输出未饱和时启用；
- 目标切换、丢球、预测帧、输出饱和时冻结或清除积分。

为避免目标从0瞬间跳到+50 mm导致过激，可在后续加入目标斜坡或S曲线；首版可以先用角度限幅和角度变化率限制控制冲击。

### 6.2 摆杆角度/步进执行器内环

内环根据MS42CG计数估算摆杆角度：

```text
theta_beam = lookup(motor_count - zero_count)
e_theta = theta_ref - theta_beam
```

步进速度候选：

```text
step_rate_target = clamp(Kp_theta × e_theta,
                         -step_rate_max,
                         +step_rate_max)
```

执行规则：

1. `step_rate_target`符号决定DIR1；
2. 绝对值决定ST1脉冲频率；
3. ST1使用独立硬件Timer产生上升沿，不占用MS42CG编码器Timer；
4. 步频必须经过加速度/减速度斜坡，防止瞬间跳频导致失步；
5. 角度误差进入死区后停止STEP脉冲，EN1保持策略由温升和机械保持试验决定；
6. 始终同时检查软件工作限位和绝对机械限位。

D36A通道1冻结端点：

```text
ST1  = PA26
DIR1 = PA24
EN1  = PB0
```

上述端点属于设计基线；实际实现仍需单独批准SysConfig修改与生成。

### 6.3 丢步/卡滞监测

不能把已发送STEP数量当成电机必然到位。维护：

```text
commanded_step_delta
encoder_count_delta
```

根据细分和编码器分辨率建立期望比例。如果连续一段时间出现：

```text
发送了STEP，但MS42CG计数不变化
或
命令位置与编码器位置差持续扩大
```

则判定为卡滞、失步、驱动未使能或机械顶死，立即进入FAULT，不再继续加大角度。

阈值必须通过无球低速测试确定。

## 7. KEY3状态机

```text
BOOT_SAFE
   ↓
WAIT_FEEDBACK
   ↓ K230有效 + MS42CG有效 + 摆杆零位有效
READY
   ↓ KEY3
LEVEL_AND_ARM
   ↓ 球接近O且速度较低
MOVE_TO_PLUS_5
   ↓ 到达+5 cm判据
TURNAROUND
   ↓ 立即切换目标
MOVE_TO_MINUS_5
   ↓ 到达并稳定
HOLD_MINUS_5 / COMPLETE
```

任意运行状态再次按KEY3：

```text
ABORT → 停止外环 → 摆杆受控回水平候选 → 停止STEP
```

### 7.1 启动门槛

KEY3事件只有以下全部满足才接受：

- 小车车轮输出为0；
- REQ-002未运行；
- K230有新的实测帧，CRC/SEQ/范围有效；
- K230未报告丢球；
- MS42CG反馈有效且未超时；
- 摆杆零点已经标定；
- 当前摆杆角度位于安全工作区；
- 钢球位于O点附近且速度较低；
- D36A无已锁存故障；
- 上一次任务故障已由明确复位流程清除。

### 7.2 +5 cm到达判据

不能只看一次位置穿过+5 cm。候选判据：

```text
abs(x_ball - 50 mm) <= plus_position_window
且 abs(v_ball) <= plus_velocity_window
持续 plus_confirm_time
```

内部窗口建议比题面的±10 mm更严格，例如先以±5～8 mm作为调试候选；最终数值由K230噪声和延迟决定。

满足后立即：

```text
x_ref = -50 mm
清除/重置位置环积分
进入TURNAROUND或MOVE_TO_MINUS_5
```

### 7.3 -5 cm稳定判据

```text
abs(x_ball + 50 mm) <= final_position_window
且 abs(v_ball) <= final_velocity_window
持续 final_confirm_time
```

稳定后进入 `HOLD_MINUS_5`，继续闭环保持在-5 cm，不立即将摆杆回水平；题面要求最终稳定在该位置。

总计时从接受KEY3启动开始，到首次满足最终稳定判据停止。软件目标必须留出余量，不能把5.000 s当控制目标。

## 8. K230数据使用

控制仅接受满足以下条件的数据：

- 帧头、固定长度、CRC正确；
- STATUS为实测；
- SEQ持续变化；
- 帧年龄小于经实测确定的门槛；
- POS、VEL、像素位置和置信度在合理范围；
- 位置跳变量和速度不违反机械可能范围。

首版建议不依赖预测帧完成到点判定。预测帧只允许短时保持或降低角度输出，不能用于宣布到达+5 cm或-5 cm。

若连续超过100 ms没有有效实测帧、SEQ卡住、STATUS丢球或置信度过低：

1. 不再沿用旧的钢球位置；
2. 清除/冻结位置环积分；
3. 若MS42CG仍有效，限制斜率将摆杆目标拉回水平零位；
4. 若MS42CG也无效，立即停止STEP并进入FAULT；
5. EN1保持还是释放必须经过无球机械安全与温升验证后冻结。

## 9. 运行时调度候选

具体Timer实例和频率必须在SysConfig阶段核对，本文只定义职责：

```text
UART RX ISR       只收K230字节并入队
主循环/协议服务   解析帧、校验CRC和SEQ
位置外环          每个新K230有效帧更新一次
角度内环          固定较高频率运行，候选500 Hz～1 kHz
STEP定时器        按目标步频产生独立上升沿
KEY3              轮询或GPIO事件 + 独立消抖
安全监控          每次主循环执行
```

不允许让STEP脉冲生成阻塞K230接收，也不允许用长延时循环发STEP。

## 10. 建议的首轮调试顺序

### 阶段A：无12 V/仅逻辑域

- 验证KEY3只触发REQ-003，不触发REQ-002；
- 验证K230 POS/VEL/STATUS/SEQ；
- 验证MS42CG唯一编码器计数、方向和零点；
- 左右轮编码器和车轮控制保持停用。

### 阶段B：无球、摆杆小角度

- D36A最低安全电流档候选，实际档位按电机额定相电流确认；
- 测试DIR1方向；
- 测试±1°、±3°、±5°；
- 建立电机计数—摆杆角度查表；
- 验证软件限位、STEP斜坡、卡滞检测和KEY3中止。

### 阶段C：有球、只保持O点

- 工作角度限制在很小范围；
- 只调位置环PD的方向、Kp和Kd；
- 验证K230丢球/超时后能安全回水平或停车。

### 阶段D：O到+5 cm

- 暂不自动折返；
- 调整到点速度和超调；
- 连续多次满足±1 cm后再开放折返。

### 阶段E：完整O→+5→-5

- 开放状态机自动折返；
- 调整两段时间预算；
- 在最终-5 cm加入必要的小积分；
- 记录每次时间、最大误差、超调、K230延迟和摆杆最大角度。

## 11. 需要调试闭合的参数

| 参数 | 作用 | 当前状态 |
|---|---|---|
| `direction_sign` | K230位置误差到摆杆倾角方向 | 必须实测 |
| `zero_count` | 水平摆杆时MS42CG零点 | 必须实测 |
| `count_to_beam_angle` | 连杆非线性映射 | 必须标定 |
| `theta_work_limit` | 正常工作角度 | 初始小角度，逐级放宽 |
| `theta_absolute_limit` | 绝对保护角度 | 用户候选约±15°，需机械确认 |
| `Kp_x/Kd_x/Ki_x` | 钢球位置外环 | 未调 |
| `Kp_theta` | 摆杆角度内环 | 未调 |
| `step_rate_max` | 最大STEP频率 | 电机/细分/负载实测 |
| `step_accel_limit` | 步频变化率 | 防失步，实测 |
| `plus_position_window` | +5 cm到达窗口 | 候选严于±10 mm |
| `final_position_window` | -5 cm稳定窗口 | 候选严于±10 mm |
| 速度窗口与确认时间 | 防止穿越目标误判到达 | K230噪声/延迟实测 |
| K230实测帧率/延迟 | 决定外环带宽 | 未验证 |
| D36A电流/细分 | 扭矩、分辨率、温升 | 未确认 |

## 12. 伪代码

```c
Req003_service(now_ms)
{
    stop_and_lock_wheel_motors();
    k230 = K230Link_snapshot(now_ms);
    ms42 = Ms42cg_snapshot(now_ms);   // 第三问唯一编码器
    key3_pressed = Key3_takePress();

    if (key3_pressed && req003_is_active()) {
        req003_abort();
        return;
    }

    validate_k230_and_ms42();
    enforce_beam_angle_and_step_mismatch_limits();

    switch (state) {
    case READY:
        if (key3_pressed && all_start_gates_valid()) {
            timer_start();
            target_mm = +50.0f;
            reset_ball_controller();
            state = MOVE_TO_PLUS_5;
        }
        break;

    case MOVE_TO_PLUS_5:
        update_ball_outer_loop_on_new_k230_frame();
        if (plus_5_reached_and_confirmed()) {
            target_mm = -50.0f;
            reset_or_freeze_integral();
            state = MOVE_TO_MINUS_5;
        }
        break;

    case MOVE_TO_MINUS_5:
        update_ball_outer_loop_on_new_k230_frame();
        if (minus_5_stable_and_confirmed()) {
            timer_stop();
            state = HOLD_MINUS_5;
        }
        break;

    case HOLD_MINUS_5:
        update_ball_outer_loop_on_new_k230_frame();
        break;

    case ABORT:
    case FAULT:
        command_beam_level_if_encoder_valid();
        otherwise_stop_step_output();
        break;
    }

    BeamAngleController_setTarget(theta_ref);
    D36aStepper_service();
}
```

## 13. 本文没有授权或证明的事项

- 未修改第二问或任何源码；
- 未修改 `.syscfg`；
- 未决定具体Timer实例、IRQ名称或生成宏；
- 未构建、未烧录；
- 未访问K230、MS42CG或D36A；
- 未确认D36A电流档、细分和步进电机相电流；
- 未确认KEY3有效电平；
- 未确认连杆几何、水平零点、方向和±15°机械安全范围；
- 未证明5秒和±1 cm指标能够达到。

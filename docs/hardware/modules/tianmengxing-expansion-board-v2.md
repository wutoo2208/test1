# 天猛星拓展板 V2.0 - 硬件来源事实卡

> **状态**：EPRO 静态复核 + 实物正面照片复核；非实物接线、非电气验收。  
> **详细来源**：[EPRO 来源复核事实](../../reviewed/tianmengxing-expansion-board-v2-source-facts.md)、[实物正面照片复核](../../reviewed/tianmengxing-expansion-board-v2-photo-review.md)。  
> **原始设计归档**：`docs/source-epro/tianmengxing-expansion-board-v2.epro`。

## 已确认的板级功能形态

| 功能区 | EPRO 中的来源事实 | 来源定位 |
|---|---|---|
| 主控适配 | PCB 存在名为 `MSPM0G3507天猛星拓展板` 的连接器组件，具有 80 个带网络名的焊盘。 | `PCB`：该组件的 `PAD_NET` |
| 原始电源输入 | `电源接口` 经 `SW3` 形成 `VBAT_IN`；PCB 丝印写 `7-24V输入，最好是12V`。 | `PCB`：组件网络与 `STRING` |
| 电源分配 | 设有 `5V稳压`、`舵机稳压`、3.3 V/5 V/GND/12 V/舵机电源接口。 | `PCB`：各组件 `PAD_NET` |
| 电机接口 | PCB 和原理图标注两组 `TB6612模块` 接口。 | `SCH-1`；`PCB` |
| 传感器接口 | 有八路灰度 H10、MPU6050、JY 系列 IMU、超声波和 OLED 接口。 | `PCB`：H10、`MPU6050模块`、U1、U12、U8 |
| 交互/通信 | 有按键、LED、蜂鸣器和 USART0/1/2 接口。 | `PCB`：对应组件 `PAD_NET` |

## 关键网络 - 只读矩阵

完整焊盘—网络矩阵见：[mspm0-expansion-net-matrix.md](../../extracted/tianmengxing-expansion-board-v2-epro/mspm0-expansion-net-matrix.md)。其中：

- `H10`：`OUT_1`-`OUT_8`、`5V`、`GND`；
- `USART0`：`PA11`、`PA10`、`GND`、`5V`；
- `USART1`：`PA9`、`PA8`、`GND`、`5V`；
- `USART2`：`PB16`、`PB15`、`GND`、`5V`；
- `MPU6050模块`：`5V`、`GND`、`GY_SCL`、`GY_SDA`；
- `OLED`：`GND`、`5V`、`PA1`、`PA0`；
- `H13`：`GND`、`BEEP`、`3.3V`。

## 禁止外推

1. `5V`、`3.3V`、`PAx`、`PBx` 是 EPRO 网络标签，不能单独证明电压容限、信号方向、外设复用、上拉或当前 SysConfig 配置。
2. `TB6612模块` 接口不是双路 DRV8870 的兼容性证明；本项目当前电机驱动须按实际 DRV8870 模块资料复核。
3. 八路灰度 H10 不能直接认定适合五路 TCRT5000；先关闭五路板的供电、电平与极性问题。
4. 输入丝印范围和稳压模块网络不能代替连续电流、峰值、热、保护、纹波和带载台架证据。

## 进入 Pin Plan 前仍需具备

- 实物扩展板**背面**、排针方向和丝印与本 EPRO PCB 的一致性（正面装配照片已归档，但尚不充分）；
- 所有外设实际型号、连接器线序、供电与输出电平；
- 天猛星开发板原理图/引脚图与扩展板 80 针连接器的一致性核对；
- TB6612 接口与现有 DRV8870 方案的明确取舍；
- 五路 TCRT5000 的专用电气资料或获授权后的静态测量。
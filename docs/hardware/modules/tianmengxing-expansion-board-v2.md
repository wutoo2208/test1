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

## 当前 frozen v1.5 使用

- `U12`：LineFollower_6CH，pin1=5V、pin2=PA31/I2C0 SCL、pin3=PA28/I2C0 SDA、pin4=GND。
- 原 `MPU6050模块` 接口：OLED，pad1=5V、pad2=GND、pad3=PB2/I2C1 SCL、pad4=PB3/I2C1 SDA。
- MPU6050：NOT FITTED；旧 `OLED` U8接口：DNC。
- 两个模块都必须使用按信号重排/交叉的线束，不能按插头位置直通。

## 禁止外推

1. `5V`、`3.3V`、`PAx`、`PBx` 是 EPRO 网络标签，不能单独证明电压容限、信号方向、外设复用、上拉或当前 SysConfig 配置。
2. `TB6612模块` 接口不是双路 DRV8870 的兼容性证明；本项目当前电机驱动须按实际 DRV8870 模块资料复核。
3. H10 pin1～8 是普通 OUT 网络，不是原生 I²C 插座。旧H10/GY共享候选已否决；frozen v1.5 使用 U12 的 `5V/SCL/SDA/GND` 网络连接 LineFollower，不切H10。
4. 输入丝印范围和稳压模块网络不能代替连续电流、峰值、热、保护、纹波和带载台架证据。

## 进入 Pin Plan 前仍需具备

- 实物扩展板**背面**、排针方向和丝印与本 EPRO PCB 的一致性（正面装配照片已归档，但尚不充分）；
- 所有外设实际型号、连接器线序、供电与输出电平；
- 天猛星开发板原理图/引脚图与扩展板 80 针连接器的一致性核对；
- TB6612 接口与现有 DRV8870 方案的明确取舍；
- LineFollower_6CH 实物版本与插头观察面、U12 pin1观察面、交叉线束、5 V负载、I²C0上拉/速率、通道方向及断电连续性。
- OLED `GND/VDD/SCK/SDA` 与原MPU/GY `5V/GND/SCL/SDA` 的重排线束、GY pad1观察面、I²C1上拉电压和实际地址。
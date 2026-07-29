# 硬件接口与接线登记

> 初始化日期：2026-07-28  
> 本文档区分“板级已复核事实”“候选接口”和“最终接线”。当前没有已确认的外接模块接线。

## 1. 安全声明

- 原始 PDF 是开发板硬件事实源；OCR/提取文本仅供检索。
- 标为 `待确认` 或 `候选` 的内容不得用于接线、上电或修改 `.syscfg`。
- 外接模块必须先确认精确型号、供电、逻辑电平、最大电流、协议和实物版本。
- 电机/执行器供电不得从开发板 3V3 路径直接推定；独立供电时仍需确认共地和回流路径。
- 本轮未连接、未上电、未测量任何硬件。

## 2. 硬件事实来源

| 层级 | 路径 | 用途 |
|---|---|---|
| 原始资料 | `docs/source-pdf/tianmengxing-mspm0g3507-pinout.pdf` | 引脚分配图，1 页 |
| 原始资料 | `docs/source-pdf/tianmengxing-mspm0g3507-schematic.pdf` | 原理图，3 页 |
| 已复核摘要 | `docs/reviewed/tianmengxing-mspm0g3507-source-facts.md` | 带原始页码的高价值事实 |
| 项目索引 | `docs/hardware/README.md` | 资料用途和使用规则 |
| 工作记忆 | `docs/hardware/board-overview.md` | 板载资源概览 |
| 工作记忆 | `docs/hardware/board-pinout.md` | 引脚规则与接口建议 |
| 工作记忆 | `docs/hardware/board-power-debug.md` | 电源、VREF、SWD、BSL、UART0 |
| 工作记忆 | `docs/hardware/resource-conflicts.md` | 板载资源冲突 |
| 工作清单 | `docs/hardware/bringup-checklist.md` | 后续接线和上电前检查 |

## 3. 已确认的开发板资源

以下是资料层面的板级事实，不表示外设已接线或已在实物上验证。

| ID | 状态 | 资源 | 已复核事实 | 证据 |
|---|---|---|---|---|
| `HW-001` | `[已确认|资料复核]` | 板载 LED | `PB22` 驱动 LED，高电平点亮。 | 原理图 p1；`docs/reviewed/tianmengxing-mspm0g3507-source-facts.md` |
| `HW-002` | `[已确认|资料复核]` | 板载按键 | `PB21` 接地，使用时应按低电平有效理解。 | 原理图 p1；同上 |
| `HW-003` | `[已确认|资料复核]` | SWD | `PA19/SWDIO`、`PA20/SWCLK` 引至调试接口。 | 原理图 p3；同上 |
| `HW-004` | `[已确认|资料复核]` | BSL | `PA18/BSL` 有 47 kΩ 下拉，BSL 按键接 3V3。 | 原理图 p1；同上 |
| `HW-005` | `[已确认|资料复核]` | 时钟相关 | `PA2` 为 ROSC 网络；`PA3/PA4`、`PA5/PA6` 与板载晶体网络相关。 | 原理图 p1；`docs/hardware/board-overview.md` |
| `HW-006` | `[已确认|资料复核]` | 板载串口路径 | `PA10/U0TX`、`PA11/U0RX` 接 CH340E 调试/下载网络。 | 原理图 p3；已复核摘要 |
| `HW-007` | `[已确认|资料复核]` | SPI Flash/LCD | `PB6–PB9` 接板载 SPI Flash；`PB8/PB9` 同时出现在 LCD 接口。 | 原理图 p3；已复核摘要 |
| `HW-008` | `[已确认|资料复核]` | ADC 参考 | `PA23/VREF+` 有参考电压选择网络；`PA21/PA23` 未确认前不作为普通 GPIO。 | 原理图 p2；`docs/hardware/board-power-debug.md` |
| `HW-009` | `[已确认|资料复核]` | 电源路径 | USB-C 5V 经 F1 500 mA，EXT_3V3 经 F2 500 mA。 | 原理图 p2；已复核摘要 |

## 4. 保留和冲突资源

| 资源 | 初始规则 | 原因 |
|---|---|---|
| `PA19/PA20` | 保留 | SWD 调试/下载 |
| `PA18` | 避免普通功能 | BSL 启动网络 |
| `PA2–PA6` | 未复核具体时钟方案前保留 | ROSC/晶体网络 |
| `PA21/PA23` | 未确认 VREF 配置前禁用 | ADC 参考网络 |
| `PA10/PA11` | 优先保留给板载 UART0 | CH340E 路径；改作其他用途会产生冲突 |
| `PB6–PB9` | 分配前审查 | 板载 SPI Flash；PB8/PB9 还与 LCD 共享 |
| `PB21/PB22` | 识别为板载按键/LED | 避免与用户功能冲突 |

## 5. 外部模块 BOM

当前已收到部分模块资料，但尚未完成实物丝印、供电电平和最终接线核对；以下条目均不是已确认物理 BOM。

“灰度模块、霍尔编码器、OLED、MPU6050、蜂鸣器、电机驱动”等名称只表示现有资料讨论过的模块类别，不能视为实际 BOM。

| 模块 ID | 精确型号/版本 | 数量 | 供电 | 逻辑电平 | 最大电流 | datasheet | 状态 |
|---|---|---:|---|---|---:|---|---|
| 待登记 | 待确认 | — | 待确认 | 待确认 | 待确认 | 待提供 | `[待确认|无]` |

## 6. 候选接口池

| ID | 状态 | 候选方向 | 依据 | 实施前门槛 |
|---|---|---|---|---|
| `IF-001` | `[候选|资料复核]` | `PA0/I2C0-SDA`、`PA1/I2C0-SCL` 可作为 I2C 方向候选。 | `docs/hardware/board-pinout.md` 和原理图/引脚图建议 | 确认模块型号、电压、地址、速率、上拉、实际复用及 SysConfig 资源 |
| `IF-002` | `[候选|资料复核]` | `PA10/U0TX`、`PA11/U0RX` 可保留为板载 UART0 日志方向。 | `HW-006` | 确认日志协议、波特率、引脚占用和 CH340E 使用方式 |

候选项不是接线指令，也不是当前 `.syscfg` 的配置事实。

## 7. 已确认接线矩阵

**当前没有任何已确认的外接模块接线。**

| Interface ID | 模块及精确型号 | 模块引脚 | 供电/电平 | 协议参数 | MCU 实例/引脚 | 状态 | DEC | TEST |
|---|---|---|---|---|---|---|---|---|
| 暂无 | — | — | — | — | — | `[待确认|无]` | — | — |

## 8. 电源与逻辑电平预算

| 项目 | 状态 | 当前记录 |
|---|---|---|
| 开发板输入/保护路径 | `[已确认|资料复核]` | 见 `HW-009`；保护器件额定信息不能直接等同于可用外设预算。 |
| 外部模块总电流 | `[待确认|无]` | 已归档 MP1584EN 参数图：宣称最大输出 3 A，但未确认实物、设定值、带载能力或热；仍缺 BOM、datasheet 与测量。 |
| 电机/执行器电源 | `[待确认|无]` | 不从开发板 3V3 直接推定。 |
| 共地与回流路径 | `[待确认|无]` | 需结合实际电源、驱动和接线确认。 |
| 逻辑电平转换 | `[待确认|无]` | 需逐模块核对 VIH/VIL/VOH/VOL。 |
| 去耦、上拉和保护 | `[待确认|无]` | 需结合模块板载电路和总线参数确认。 |

## 9. 接线确认门槛

接口进入“已确认接线矩阵”前必须同时满足：

1. 模块精确型号、实物版本和 datasheet 已登记。
2. 供电电压、逻辑电平、最大电流和方向已复核。
3. 协议实例、速率/模式、上拉或极性等参数已确定。
4. MCU 引脚已通过原理图、引脚图、资源冲突和实际排针位置复核。
5. 负责人通过 `DEC-*` 接受该分配。
6. 后续修改 `.syscfg` 或接线前已获得用户明确授权。

## 10. 新增接口模板

```text
Interface ID: IF-NNN
Module exact model/revision:
Module pin and direction:
Supply and logic level:
Maximum/current budget:
Protocol and parameters:
MCU peripheral instance and pin:
Physical header position:
Conclusion status:
Datasheet/source/page:
Wiring photo/diagram:
Related REQ/DEC/TASK/TEST:
Supersedes:
Reviewed by:
```

## 9. 模块资料索引（未确认接线）

| 模块 | 已提取资料 | 当前关键事实 | 阻塞项 |
|---|---|---|---|
| nRF24L01+ | `docs/hardware/modules/nrf24l01p.md` | 3.3 V SPI 无线 IC/模块候选，最高 2 Mbps 空口速率。 | 实物模块排针、用途、图传能力。 |
| 八路灰度循迹 | `docs/hardware/modules/line-sensor-8ch.md` | 5 V 供电、AD0-AD2 选通、OUT 数字量；教程候选 PA14-PA17。 | OUT 电平、发光颜色、实测高度。 |
| MPU6000A/MPU6050 | `docs/hardware/modules/mpu6000a.md` | I2C 地址候选 0x68/0x69、INT、采样率/量程寄存器。 | 实物型号、上拉电压、安装方向。 |
| DRV8870 双路 | `docs/hardware/modules/drv8870-dual.md` | 双电机控制、四路编码器输入、VIN/电流/接口资料。 | 实物板版本、电机参数、电池与电平。 |
| 天猛星拓展板 V2.0 | `docs/hardware/modules/tianmengxing-expansion-board-v2.md` | EPRO 静态资料与实物正面装配照片均已归档；可见双稳压、TB6612、OLED、按键等区域。 | 背面照片、板号、排针线序、电平与实际供电路径。 |
| MP1584EN 可调降压 | `docs/hardware/modules/mp1584en-adjustable-step-down.md` | 用户称 MP1584EN；参数图称 4.5–28 V 输入、0.8–20 V 可调输出、最大 3 A。 | 实物板型、输出设定/测量、负载、保护、热与接线。 |

最终接线仍应只登记在本文件“已确认接线矩阵”中；在用户回答 `docs/hardware/module-questions.md` 的问题前，不得写入 `.syscfg` 或接线结论。
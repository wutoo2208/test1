# TCRT5000 五路模块 - 实物照片事实卡

> **状态**：实物外观已确认；电气接口待确认；非最终接线。  
> **详细证据**：[tcrt5000-five-channel-photo-review.md](../../reviewed/tcrt5000-five-channel-photo-review.md)。

## 已确认

| 项目 | 实物照片事实 | 证据 |
|---|---|---|
| 模块形态 | 一块集成 PCB 上有五组相同的反射式传感器封装。 | 用户陈述；`tcrt5000-five-channel-user-photo-02.jpg` |
| 接口丝印 | `GND`、`5V`、`OUT1`、`OUT2`、`OUT3`、`OUT4`、`OUT5`。 | 两张用户照片 |
| 可见电源标记 | PCB 可见 `POWER` 丝印。 | `tcrt5000-five-channel-user-photo-01.jpg` |

## 仍待确认

- `5V` 标签代表的实际供电范围，以及五路输出的高/低电平幅度；不能据此假定可直连 3.3 V MSPM0 GPIO。
- `OUT1`-`OUT5` 的有效极性、驱动方式、上拉、通道位置映射和最大输出电流。
- 板载 IC 的精确型号及完整原理图。
- 正式赛道、安装高度和环境光下的可用性。

## 约束

本卡中的实物事实来自用户照片，**没有原 PDF 页码**；它们不覆盖 [单通道 PDF 复核记录](tcrt5000-single-channel-source-facts.md) 中的“单通道资料”范围说明。
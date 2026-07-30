# MSPM0G3507 无线 SPI0 Pin Plan v1.1 候选

> **状态**：`SUPERSEDED / REJECTED-PIN-CONFLICT / NOT WIRED`。
> **日期**：2026-07-30。
> **基线**：只对 `mspm0g3507-pin-plan-frozen-v1.0.md` 增加无线 SPI0 所需的有限变更；v1.0 的其他冻结资源保持不变。
> **否决原因**：2026-07-30 用户确认 PB18 连接 KEY3、不可作为无线 SCLK；本候选不得继续使用，后继方案见 v1.2。
> **不包含**：`.syscfg` 修改、构建、烧录、接线、上电或通信验证。

## 1. 变更目标

在不占用 v1.0 的 K230、CH340、Flash、SWD、时钟、D36A、MS42CG、DRV 和五路 TCRT 的其余资源前提下，为用户所述改装无线模块建立 `SPI0 + CE + IRQ` 候选。

无线模块已知针脚仅来自用户此前陈述：

```text
pin3 = CSN
pin4 = CE
pin5 = MOSI
pin6 = SCK
pin7 = IRQ（低有效）
pin8 = MISO
pin1 / pin2 = UNKNOWN / DO-NOT-CONNECT
```

因此，本计划**不设计任何无线模块电源线**，也不推断 pin1/pin2 为 VCC/GND。

## 2. 与 v1.0 的资源变化

| 资源 | v1.0 | v1.1 候选 | 原因 |
|---|---|---|---|
| H10 pin1 / `OUT_1` / PB19 | TCRT OUT1 | SPI0 `POCI/MISO` | 释放 PB19 给 SPI0。 |
| H10 pin2 / `OUT_2` / PB17 | TCRT OUT2 | SPI0 `PICO/MOSI` | 释放 PB17 给 SPI0。 |
| H10 pin6 / `OUT_6` / PB25 | DNC | SPI0 `CS0/CSN` | PB25 是本地 SDK 静态核验到的 SPI0 CS0 复用点。 |
| H10 pin7 / `OUT_7` / PA25 | DNC | TCRT OUT1 | 补回迁移的第一路循迹输出。 |
| H10 pin8 / `OUT_8` / PA27 | DNC | TCRT OUT2 | 补回迁移的第二路循迹输出。 |
| PB18 | 未分配 | SPI0 `SCLK` | 必须从开发板 U22 原始 PB18 排针飞线，扩展板现有 H1-H4/H10 未确认引出。 |
| PB1 | 未分配 | 无线 `CE` GPIO 输出 | 使用 H1 pin6 取线。 |
| PB16 | UART2 future-reserved / DNC | 无线 `IRQ` GPIO 输入 | 用户明确同意释放 UART2 的 PB16；使用 H3 pin7 取线。 |
| PB15 | UART2 future-reserved / DNC | 保持 DNC | UART2 已不能视为完整预留接口；PB15 不自动再分配。 |

## 3. 无线模块候选线束

| 无线模块针脚/角色 | MCU 引脚 | MCU 外设/模式 | 拓展板/开发板取线 | 上电安全候选 | 状态 |
|---|---|---|---|---|---|
| pin3 `CSN` | `PB25` | `SPI0.CS0` | H10 pin6 / `OUT_6` | 未选中为高；具体 SPI 模式/极性待模块资料确认 | CONDITIONAL |
| pin4 `CE` | `PB1` | GPIO 输出 | H1 pin6 / `PB1` | 复位及异常时强制低 | CONDITIONAL |
| pin5 `MOSI` | `PB17` | `SPI0.PICO` | H10 pin2 / `OUT_2` | 不在初始化前输出有效事务 | CONDITIONAL |
| pin6 `SCK` | `PB18` | `SPI0.SCLK` | 开发板 U22 原始 `PB18` 排针飞线 | SPI mode/空闲电平待确认；需核对 U22 观察面和实际可达性 | CONDITIONAL |
| pin7 `IRQ`（低有效） | `PB16` | GPIO 输入 / IRQ 候选 | H3 pin7 / `PB16` | 上拉策略取决于模块输出结构；未确认前不指定内部/外部上拉 | CONDITIONAL |
| pin8 `MISO` | `PB19` | `SPI0.POCI` | H10 pin1 / `OUT_1` | 输入；未确认模块未选中三态前不得提升 SPI 频率 | CONDITIONAL |
| pin1 / pin2 | — | DNC | 不接 | 不推断 VCC/GND 或其他功能 | BLOCKED |

本地 MSPM0 SDK `2.11.00.07` 静态 IOMUX 证据：

```text
PB17 = SPI0.PICO (mode 3)
PB18 = SPI0.SCLK (mode 3)
PB19 = SPI0.POCI (mode 3)
PB25 = SPI0.CS0  (mode 3)
```

## 4. TCRT 五路改线候选

为释放 PB17/PB19，五路一体 TCRT5000 模块的输出线束改为：

| TCRT 模块输出标签 | H10 针脚 | 拓展板网络 | MCU 引脚 |
|---|---:|---|---|
| `OUT1` | 7 | `OUT_7` | `PA25` |
| `OUT2` | 8 | `OUT_8` | `PA27` |
| `OUT3` | 3 | `OUT_3` | `PA16` |
| `OUT4` | 4 | `OUT_4` | `PA14` |
| `OUT5` | 5 | `OUT_5` | `PB20` |
| `5V` / `GND` | 9 / 10 | `5V` / `GND` | 供电/地仍依赖独立电平与电源验证 |

这不是“一根 7 针直通排线”的方案。必须以独立杜邦线或确认过观察面的交叉排线制作，且最终软件的传感器左右顺序必须以实际物理位置为准，不能按 H10 针号推断。

## 5. 必须先关闭的阻塞项

1. 无线模块 pin1/pin2 的精确身份、供电范围、地、峰值电流、去耦和实际排针观察面。
2. 无线模块 pin7 IRQ 的输出类型（推挽/开漏）、无效电平和上拉需求；PB16 才能最终启用中断。
3. U22 `PB18` 的实物针号、观察面、拓展板装配后可达性及飞线机械固定。
4. H10 改线后 TCRT OUT1–OUT5 的电平、极性、物理左右顺序和抗干扰表现。
5. SPI 模式、SCLK 频率、CSN/CE 初始偏置、MISO 未选中三态、对端模块和数据包协议。
6. 无线链路只可用于开发期低速遥测/参数导出候选，不能代替 H 题要求的视频图传，也不得用于比赛过程人工遥控。

## 6. 进入下一阶段前的静态/台架门槛

- 用户审阅并明确接受本 v1.1 候选后，才可合并为新的冻结 Pin Plan；
- 之后仍需用户单独授权 `.syscfg` 修改（L1）与构建（L2）；
- 第一次无线静态上电/串口或 SPI 台架访问至少需要 L3，并先关闭电源和实际排针问题；
- 在所有信号验证前，CE 保持低、CSN 不选中、SCLK 不产生事务；电机与 D36A 保持断能。

## 7. 本轮未执行

- 未修改 `empty.syscfg`、源码、CCS 工程或生成文件。
- 未接线、未制作 H10 交叉线束、未从 U22 引出 PB18。
- 未连接无线模块 pin1/pin2，未给无线模块供电。
- 未构建、烧录、探针、串口、SPI、上电或硬件测试。

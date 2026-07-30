# MSPM0G3507 无线 SPI0 Pin Plan v1.2 候选（历史审批稿）

> **状态**：`APPROVED / MATERIALIZED AS FROZEN v1.2 / HISTORICAL CANDIDATE / NOT WIRED`。
> **日期**：2026-07-30。
> **基线**：`mspm0g3507-pin-plan-frozen-v1.0.md`；v1.1 因 `PB18=KEY3` 冲突被否决。
> **审批结果**：用户已于 2026-07-30 批准本候选；当前权威文件为 `mspm0g3507-pin-plan-frozen-v1.2.md` 与 `mspm0g3507-adapter-harness-v1.2.md`。本历史稿不授权 `.syscfg`、接线、上电或构建。

## 1. 冲突修正

用户现场确认：

```text
PB18 = KEY3，无法用于 SPI0 SCLK
```

本地 MSPM0 SDK 显示 SPI0 SCLK 还可复用到 `PA12`、`PA11`、`PA6`：

- `PA11` 已由板载 CH340/UART0 占用，且存在板载电气路径，不选；
- `PA6` 属于板载高频晶体/时钟保留资源，禁止使用；
- `PA12` 原为 MS42CG A 相，允许通过重新安排 MS42CG 捕获资源释放。

因此 v1.2 选择：

```text
SPI0 SCLK = PA12
PB18       = KEY3 / FORBIDDEN-FOR-RADIO
```

## 2. v1.2 唯一变更表

| 资源 | v1.0 / v1.1 | v1.2 候选 | 物理路径 / 原因 |
|---|---|---|---|
| `PB18` | v1.1 SPI0 SCLK | KEY3，禁止无线使用 | 用户现场确认。 |
| `PA12` | MS42CG A / TIMG0 CCP0 | SPI0 SCLK | H3 pin15 / `AIN4`；本地 SDK `SPI0.SCLK mode 3`。 |
| `PA29` | 未分配 | MS42CG A / TIMG6 CCP0 Capture | H4 pin17 / `PA29`；本地 SDK `TIMG6.CCP0 mode 5`。 |
| `PA30` | 未分配 | MS42CG B / TIMG6 CCP1 Capture | H2 pin7 / `PA30`；本地 SDK `TIMG6.CCP1 mode 5`。 |
| `PA13` | MS42CG B / TIMG0 CCP1 | MS42CG PWM / TIMG0 CCP1 单输入捕获 | H4 pin15 / `BIN3`；用双边沿时间戳计算周期和高电平时间，具体配置待 SysConfig 验证。 |
| `PB26` | MS42CG PWM / TIMG6 CCP0 combined capture | 释放，保持 DNC | H4 pin16 / `AIN3`；不自动分配给其他功能。 |
| `PB16` | UART2 future-reserved | 无线 IRQ GPIO 输入候选 | H3 pin7；用户允许释放 UART2。 |
| `PB15` | UART2 future-reserved | 保持 DNC | UART2 不再作为完整预留接口。 |

v1.0 的其他资源不变。

## 3. 无线 SPI0 完整候选

| 无线模块针脚/角色 | MCU 引脚 | 外设/模式 | 物理取线 | 安全默认候选 | 状态 |
|---|---|---|---|---|---|
| pin3 `CSN` | `PB25` | `SPI0.CS0` | H10 pin6 / `OUT_6` | 未选中高；SPI mode 待确认 | CONDITIONAL |
| pin4 `CE` | `PB1` | GPIO 输出 | H1 pin6 / `PB1` | 复位、异常和初始化前保持低 | CONDITIONAL |
| pin5 `MOSI` | `PB17` | `SPI0.PICO` | H10 pin2 / `OUT_2` | 初始化完成前不产生有效事务 | CONDITIONAL |
| pin6 `SCK` | `PA12` | `SPI0.SCLK` | H3 pin15 / `AIN4` | 空闲极性与最高频率待确认 | CONDITIONAL |
| pin7 `IRQ`（低有效） | `PB16` | GPIO 输入 / IRQ 候选 | H3 pin7 / `PB16` | 上拉取决于模块推挽/开漏结构 | CONDITIONAL |
| pin8 `MISO` | `PB19` | `SPI0.POCI` | H10 pin1 / `OUT_1` | 未选中三态待确认 | CONDITIONAL |
| pin1 / pin2 | — | DNC | 不接 | 不推断 VCC/GND | BLOCKED |

SPI0 IOMUX 静态证据：

```text
PB17 = SPI0.PICO (mode 3)
PA12 = SPI0.SCLK (mode 3)
PB19 = SPI0.POCI (mode 3)
PB25 = SPI0.CS0  (mode 3)
```

## 4. TCRT 五路线束候选

| TCRT 模块输出 | H10 针脚 | 拓展板网络 | MCU 引脚 |
|---|---:|---|---|
| `OUT1` | 7 | `OUT_7` | `PA25` |
| `OUT2` | 8 | `OUT_8` | `PA27` |
| `OUT3` | 3 | `OUT_3` | `PA16` |
| `OUT4` | 4 | `OUT_4` | `PA14` |
| `OUT5` | 5 | `OUT_5` | `PB20` |
| `5V/GND` | 9/10 | `5V/GND` | 供电、电平和回流仍待验证 |

H10 pin1、pin2、pin6 分别转为无线 MISO、MOSI、CSN。TCRT 必须使用独立跳线或确认过观察面的交叉线束，不能使用原连续直通次序。

## 5. MS42CG 新反馈候选

| MS42CG 信号 | MCU 引脚 | 定时器/模式 | 物理路径 | 说明 |
|---|---|---|---|---|
| `A` | `PA29` | TIMG6 CCP0 Capture | H4 pin17 | 与 B 组成同一 TIMG6 双通道软件正交。 |
| `B` | `PA30` | TIMG6 CCP1 Capture | H2 pin7 | 与 A 同实例；最高边沿频率仍待实测。 |
| `PWM` | `PA13` | TIMG0 CCP1 单输入捕获候选 | H4 pin15 / `BIN3` | 候选采用单通道双边沿时间戳计算占空比；必须由 SysConfig 隔离验证确认。 |
| `Z` | `PB23` | GPIO IRQ | H3 pin16 | v1.0 不变。 |

定时器 Owner 候选变化：

```text
TIMG6：MS42CG A/B 双 Capture（PA29/PA30）
TIMG0：MS42CG PWM 单输入双边沿 Capture（PA13）
```

这避免占用右轮编码器的 `TIMG8`，也不改变左轮编码器 `TIMA1`、D36A `TIMG7` 或电机 PWM `TIMA0`。

## 6. v1.2 MCU 增量 Owner 表

| MCU 引脚 | v1.2 Owner | 状态 |
|---|---|---|
| `PB25` | 无线 SPI0 CSN | CANDIDATE |
| `PB1` | 无线 CE | CANDIDATE |
| `PB17` | 无线 SPI0 MOSI | CANDIDATE |
| `PA12` | 无线 SPI0 SCLK | CANDIDATE |
| `PB16` | 无线 IRQ | CANDIDATE |
| `PB19` | 无线 SPI0 MISO | CANDIDATE |
| `PA25` | TCRT OUT1 | CANDIDATE |
| `PA27` | TCRT OUT2 | CANDIDATE |
| `PA29` | MS42CG A | CANDIDATE |
| `PA30` | MS42CG B | CANDIDATE |
| `PA13` | MS42CG PWM | CANDIDATE |
| `PB26` | DNC / released | UNASSIGNED |
| `PB18` | KEY3 | FORBIDDEN-FOR-RADIO |

## 7. 审批前必须理解的代价

1. MS42CG A/B/PWM 的物理线束和定时器 Owner 全部变化，不能沿用 v1.0 线束。
2. MS42CG PWM 从 combined capture 改为单通道双边沿时间戳候选；必须在后续 SysConfig 隔离验证中证明可配置、无实例冲突，并在台架上验证角度占空比。
3. TCRT OUT1/OUT2 仍需要交叉改线到 H10 pin7/pin8。
4. 无线 pin1/pin2、电源和地仍未知；即使 Pin Plan 获批，也不得给模块上电。
5. 本候选只定义资源，不定义 SPI mode、SCLK 频率、数据包协议、IRQ 上拉或正式比赛使用策略。

## 8. 待审批结论

用户审批时应明确回复：

```text
批准无线 SPI0 Pin Plan v1.2 候选，允许后续生成冻结版；不授权修改 SysConfig、接线或上电。
```

若批准，下一任务仅生成新的冻结 Pin Plan 与配套线束文档；不会修改 `.syscfg`。

## 9. 本轮未执行

- 未修改 `empty.syscfg`、源码、CCS 工程、生成文件或当前 v1.0 冻结文件。
- 未运行 SysConfig、构建、烧录、探针或串口。
- 未接线、未给无线模块供电、未输出 SPI、未访问 IRQ。
- 未验证 MS42CG A/B/PWM 的新定时器配置或信号性能。

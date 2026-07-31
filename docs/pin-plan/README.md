# Pin Plan 版本索引

## 当前唯一最新版

| 类型 | 当前权威文件 | 状态 |
|---|---|---|
| 基础 MCU/Timer/PinMux 资源 | `mspm0g3507-pin-plan-frozen-v1.2.md` | FROZEN-DESIGN；未被后续修订覆盖的条目继续有效 |
| START_BUTTON / 蜂鸣器修订 | `mspm0g3507-pin-plan-frozen-v1.3.md` | APPROVED AMENDMENT |
| KEY2 实物映射修订 | `mspm0g3507-pin-plan-frozen-v1.4.md` | USER-VERIFIED AMENDMENT |
| LineFollower/OLED/MPU/U12/I2C 重构 | `mspm0g3507-pin-plan-frozen-v1.5.md` | FROZEN-DESIGN / USER-APPROVED / NOT SYSCONFIG-APPLIED |
| 当前模块逐线线束 | `mspm0g3507-adapter-harness-v1.5.md` | FROZEN-DESIGN / USER-APPROVED / NOT WIRED |

解释规则：按 v1.2 基础表读取，再依次应用 v1.3、v1.4、v1.5 修订；发生冲突时高版本优先。所有新接线只使用 v1.5 harness。

## v1.5 当前关键端点

```text
LineFollower_6CH：U12 → PA28 SDA / PA31 SCL / I2C0
OLED：原MPU接口 → PB3 SDA / PB2 SCL / I2C1；实物 `VDD→5V`、`SCK→SCL`，必须重排线束
MPU6050：NOT FITTED
旧U8 OLED接口：DNC
旧五路TCRT/H10巡线线束：退出
```

## 历史/候选文件

| 文件 | 状态 |
|---|---|
| `mspm0g3507-pin-plan-frozen-v1.0.md` | SUPERSEDED BY v1.2 |
| `mspm0g3507-adapter-harness.md` | SUPERSEDED |
| `mspm0g3507-adapter-harness-v1.2.md` | SUPERSEDED BY v1.5 HARNESS；仅追溯无线/旧模块 |
| `mspm0g3507-pin-plan-candidate.md` | HISTORICAL / SUPERSEDED |
| `mspm0g3507-pin-plan-wireless-spi0-v1.1-candidate.md` | REJECTED：PB18=KEY3 |
| `mspm0g3507-pin-plan-wireless-spi0-v1.2-candidate.md` | MATERIALIZED AS v1.2 |
| `mspm0g3507-pin-plan-linefollower-6ch-v1.5-candidate.md` | MATERIALIZED AS FROZEN v1.5；旧H10/GY共享方案已否决 |
| `mspm0g3507-adapter-harness-linefollower-6ch-v1.5-candidate.md` | SUPERSEDED BY `mspm0g3507-adapter-harness-v1.5.md` |

冻结只确认设计，不自动授权修改 `.syscfg`、源码、接线、上电、构建、烧录或硬件测试。

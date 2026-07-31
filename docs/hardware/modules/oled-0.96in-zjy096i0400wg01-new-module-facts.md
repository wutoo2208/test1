# 0.96 英寸新款 OLED - ZJY096I0400WG01 来源事实卡

> **状态**：`FROZEN-DESIGN / USER-APPROVED / NOT WIRED`；实物照片和模块数据手册已复核，电气与通信仍未实测。
> **主复核记录**：[ZJY096I0400WG01 新款 OLED 模块复核](../../reviewed/zjy096i0400wg01-new-oled-module-review.md)。

## 当前可用的模块级事实

| 项目 | 结论 | 来源 |
|---|---|---|
| 模块资料型号 | `ZJY096I0400WG01`，Revision A。 | PDF p1-p2 |
| 实物载板丝印 | `GM009605V4.3`；与 PDF 是否逐修订一致尚未证明。 | 用户照片 |
| 显示规格 | 白色、128×64；PCB 27.3×27.8×2.65 mm。 | PDF p5 |
| 资料接口 | 1=GND、2=VCC、3=SCL、4=SDA；I²C。 | PDF p6-p8 |
| 实物接口 | 1=GND、2=VDD、3=SCK、4=SDA。 | 用户 photo-02、photo-03 |
| 当前设计别名 | `VDD` 按模块正电源使用；`SCK` 按本 I²C 模块的 `SCL` 使用。 | PDF I²C 定义 + 用户批准 v1.5 |
| I²C 地址 | 资料默认 7 位地址 0x3C；地址电阻切换后为 0x3D。 | PDF p7-p8 |
| 电源与逻辑 | 模块资料的显示供电为 3–5 V；SCL/SDA 数据域为 1.65–3.3 V。 | PDF p9 |
| 资料原理图 | 输入 VCC_IN 经标记 `662K` 的 U2 形成内部 `VCC`；SCL/SDA 各有 4.7 kΩ 上拉。 | PDF p8 |

## frozen v1.5 接线设计

```text
OLED GND → 原MPU6050/GY接口 pad2 / GND
OLED VDD → 原MPU6050/GY接口 pad1 / 5V
OLED SCK → 原MPU6050/GY接口 pad3 / PB2 / I2C1 SCL
OLED SDA → 原MPU6050/GY接口 pad4 / PB3 / I2C1 SDA
```

- 旧 U8 OLED 接口退出并保持 DNC。
- OLED 针序为 `GND/VDD/SCK/SDA`，GY 接口顺序为 `5V/GND/SCL/SDA`；**禁止使用完全直通四芯线**，必须按信号重排。
- `SCK` 在本模块设计中是 I²C 时钟别名，不是改用 SPI 的依据。

## 接线与上电前规则

- frozen v1.5 只确认设计端点，不等于已制作线束、已修改 SysConfig 或已上电。
- 即使模块从 5 V 供电，PB2/PB3 上的 I²C 数据域也不得超过 3.3 V。
- 第一次上电前确认 GY pad1 观察面并完成断电连续性；第一次上电后立即测 SCK/SDA 空闲电压，异常接近 5 V、长期 0 V或发热时断电。
- 实际 7 位地址必须扫描确认是 `0x3C` 还是 `0x3D`；不得只按默认值写死。

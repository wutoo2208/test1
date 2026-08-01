# 天猛星 MSPM0G3507 开发板概览

> 来源：引脚图第 1 页；原理图第 1 页（core）、第 2 页（power）、第 3 页（extension）。
> 状态：已根据 PDF 页面图像复核；实际接线前仍须确认模块电压与外设复用。

## 板载资源

| 资源 | MCU/网络 | 行为或说明 | 来源 |
|---|---|---|---|
| 用户 LED | `PB22` | `PB22 -> 2 kΩ -> LED -> GND`；GPIO 输出高电平时 LED 点亮。 | 原理图 p1 |
| 功能按键 | `PB21` | 按键将 `PB21` 接到 GND；应用中应使用上拉输入并按低电平判断。 | 原理图 p1 |
| Reset 按键 | `NRST` | `NRST` 由 47 kΩ 上拉至 3V3，按键接 GND。 | 原理图 p1 |
| BSL 按键 | `PA18/BSL` | `PA18/BSL` 由 47 kΩ 下拉至 GND，BSL 按键接至 3V3。 | 原理图 p1 |
| 高频时钟 | `PA5/HFXIN`、`PA6/HFXOUT` | 板载 40 MHz 晶体。 | 原理图 p1 |
| 低频时钟 | `PA3/LFXIN`、`PA4/LFXOUT` | 板载 32.768 kHz 晶体。 | 原理图 p1 |
| ROSC | `PA2/ROSC` | 板载 100 kΩ 精密电阻网络；不要作为普通 GPIO 使用。 | 原理图 p1 |
| 调试接口 | `PA19/SWDIO`、`PA20/SWCLK` | 在调试/下载接口 U20 上引出；同时带 UART0 与电源。 | 原理图 p3 |
| 板载串口（原理图资料） | `PA10/U0TX`、`PA11/U0RX` | 原理图记录为经 CH340E 到 USB 的板级网络；**当前项目未接 UART0 外设/线束，未验证为可用 USB/COM 链路。** | 原理图 p3；用户现场陈述 |
| 板载 SPI Flash | `PB6`、`PB7`、`PB8`、`PB9` | 分别接 SPI1 CS0/POCI/PICO/SCK 与 U24 板载 Flash。 | 原理图 p3 |
| SPI-LCD 接口 | `PB8`、`PB9`、`PB10`、`PB11`、`PB14`、`PB26` | 接口 H8：SDA、SCL、RES、DC、CS、BLK；PB8/PB9 与板载 Flash 共用。 | 原理图 p3 |

## 给电赛工程的初步建议

- MPU6050 与 I2C OLED 优先考虑 `PA0/I2C0-SDA`、`PA1/I2C0-SCL`；原理图显示 PA0/PA1 配有 4.7 kΩ 开漏上拉网络，默认通过 0 Ω 选择到 3V3。接外设时仍应保证 I2C 总线最终上拉到 3.3 V。来源：原理图 p1、引脚图 p1。
- 当前项目**未启用** `PA10/U0TX`、`PA11/U0RX` 的 UART0 日志、遥测或参数调节；不得因原理图存在 CH340E 路由而假定无需 USB-TTL、已存在 COM 口或可直接接 DAP UART。未来启用或复用前需单独核对实际板型、连线、Pin Plan 与 SysConfig。
- 用户 LED 和按键可先用于最小点灯、状态告警和人工模式切换：`PB22`、`PB21`。
- 电机 PWM、编码器、八路灰度的具体 GPIO 暂不在此文档固定；应先在 SysConfig 中检查定时器、ADC、DMA 和所选引脚是否冲突。

# 天猛星 MSPM0G3507 PDF 事实提取记录

## 来源文件

| 文件 | 已处理页面 | 提取物 |
|---|---:|---|
| `tianmengxing-mspm0g3507-pinout.pdf` | 1/1 | Markdown + `images/page-001.png` |
| `tianmengxing-mspm0g3507-schematic.pdf` | 3/3 | Markdown + `images/page-001.png` 到 `page-003.png` |

## 已核对的高价值事实

- `PB22` 驱动用户 LED；高电平点亮。原理图 p1。
- `PB21` 是接地按键。原理图 p1。
- `PA18/BSL` 47 kΩ 下拉，BSL 按键接 3V3。原理图 p1。
- `PA19/SWDIO`、`PA20/SWCLK` 由调试接口引出。原理图 p3。
- `PA10/U0TX`、`PA11/U0RX` 接板载 CH340E 串口调试/下载网络。原理图 p3。
- `PB6`、`PB7`、`PB8`、`PB9` 接板载 SPI Flash；PB8/PB9 同时出现在 LCD 接口。原理图 p3。
- USB-C 5V 经 F1 500mA，EXT_3V3 经 F2 500mA。原理图 p2。
- PA23/VREF+ 有三种参考电压来源选择，三者存其一。原理图 p2。

## 待确认项

- U21/U22 每个物理排针序号与 GPIO 的最终接线，应在操作时对照引脚图 p1；不只依赖 Markdown/OCR。
- 灰度模块、霍尔编码器、OLED、MPU6050、蜂鸣器的实际供电和逻辑电平尚未由其自身模块说明书确认。
- SPI-LCD H8 的 0Ω 电阻供电选择必须在接屏前确认。
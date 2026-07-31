# 天猛星 MSPM0G3507 硬件资料索引

本目录由以下原始 PDF 提取并经页面图像复核后整理：

- `docs/source-pdf/tianmengxing-mspm0g3507-pinout.pdf`（引脚分配图，1 页）
- `docs/source-pdf/tianmengxing-mspm0g3507-schematic.pdf`（原理图，3 页：core / power / extension）

## 文档

- [board-overview.md](board-overview.md)：板载资源、时钟、按键、LED、调试和串口概览。
- [board-pinout.md](board-pinout.md)：引脚使用规则、保留资源、接口与外设建议。
- [board-power-debug.md](board-power-debug.md)：USB 电源、3V3、参考电压、SWD、BSL 与 UART0。
- [resource-conflicts.md](resource-conflicts.md)：电赛工程开始前应保留或避免冲突的资源。
- [bringup-checklist.md](bringup-checklist.md)：接外设、建 SysConfig、上电和下载前检查清单。

## 使用规则

1. 原始 PDF 是唯一硬件事实来源；本目录是方便 Claude Code/Codex 检索的工作记忆。
2. 表中标为“待确认”的内容不得直接用于接线或修改 `.syscfg`。
3. 任何 GPIO 最终配置必须同时通过原理图、引脚图、MSPM0 SDK/SysConfig 和实际接线复核。
4. 不要直接修改由 SysConfig 生成的 `ti_msp_dl_config.c/.h` 文件。
## 模块资料（已整理；设计冻结不等于已接线）

- [modules/nrf24l01p.md](modules/nrf24l01p.md)
- [modules/linefollower-6ch-i2c.md](modules/linefollower-6ch-i2c.md)：当前主循迹，frozen v1.5 经 U12 独占 I2C0，地址 0x5C。
- [modules/oled-0.96in-zjy096i0400wg01-new-module-facts.md](modules/oled-0.96in-zjy096i0400wg01-new-module-facts.md)：当前 OLED，实物 `GND/VDD/SCK/SDA`，frozen v1.5 经原MPU/GY接口独占I2C1。
- [modules/line-sensor-8ch.md](modules/line-sensor-8ch.md)：历史八路模块资料，不再作为当前主循迹。
- [modules/mpu6000a.md](modules/mpu6000a.md)：`NOT FITTED / HISTORICAL OPTION`，当前接口已转给OLED。
- [modules/drv8870-dual.md](modules/drv8870-dual.md)
- [modules/tianmengxing-expansion-board-v2.md](modules/tianmengxing-expansion-board-v2.md)
- [modules/mp1584en-adjustable-step-down.md](modules/mp1584en-adjustable-step-down.md)
- [modules/k230-target-tracking.md](modules/k230-target-tracking.md)
- [modules/ms42cg-encoder.md](modules/ms42cg-encoder.md)
- [modules/d36a-dual-stepper-driver.md](modules/d36a-dual-stepper-driver.md)
- [module-questions.md](module-questions.md)
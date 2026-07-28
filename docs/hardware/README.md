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
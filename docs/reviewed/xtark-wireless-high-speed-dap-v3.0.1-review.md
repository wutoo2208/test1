# 塔克无线高速 DAP 调试下载器 V3.0.1 手册复核

## 1. 来源与处理范围

| 项目 | 内容 |
|---|---|
| 原始文件 | `docs/source-pdf/xtark-wireless-high-speed-dap-v3.0.1.pdf` |
| 文件 SHA-256 | `451cdd099a6f5f4c4824b39e6c775a009af025e6093f06b53ba48424e22f969a` |
| PDF 页数 | 14 页 |
| PDF 内部标题版本 | V3.0 |
| 文件名版本 | V3.0.1 |
| PDF 创建时间元数据 | 2026-06-01 10:13:49 +08:00 |
| 自动提取 | `docs/extracted/xtark-wireless-high-speed-dap-v3.0.1/xtark-wireless-high-speed-dap-v3.0.1.md` |
| 关键页面图像 | `docs/extracted/xtark-wireless-high-speed-dap-v3.0.1/images/` |

本文只登记手册和权威软件文档能够支持的事实。厂商性能描述不等于本项目实测；“理论支持 Cortex-M”也不等于已在 MSPM0G3507、当前 CCS、当前固件和无线链路上验收。

## 2. 产品定位与厂家声明

- 产品基于 CMSIS-DAP，普通款模块可在有线、无线主机和无线从机三种模式间切换；一套无线调试链路包含电脑侧发射机与目标板侧接收机。手册 p3、p5、p7-p8。
- 支持 SWD、JTAG、硬件/软件复位和虚拟串口；厂商列出的上位机路径为 MDK、IAR 和 OpenOCD。手册 p3-p4。
- 无线接收端需要由目标板供电，可使用 3.3 V 或 5 V，手册推荐 5 V。手册 p8。
- 厂商宣称第三代在无线下载速度、距离和稳定性上升级：无线下载速度可达约 60 KB/s；2 M 空中速率下实测距离可达 100 m 以上。该数据仅为厂商声明，项目未实测。手册 p3。
- 单个普通款可作为有线 CMSIS-DAP 使用；可选 USB 迷你款作为电脑侧发射主机，需配合普通款接收机。手册 p3、p8。

## 3. 普通款接口丝印

手册 p5 给出的 2x6 接口丝印如下。下表只记录标签与排布，不根据标签猜测未说明的内部电气结构。

| 左列，自上而下 | 右列，自上而下 |
|---|---|
| 5V | TX |
| 3V3 | RX |
| GND | RST |
| CLK | TDI |
| DIO | TDO |
| DIR | GND |

普通款外形尺寸图标注约 43 mm x 26 mm x 14 mm。手册 p5。

## 4. 模式、指示灯与配对

### 4.1 模式指示

| STA 指示灯 | 模式 |
|---|---|
| 红色 | 有线模式 / USB |
| 蓝色 | 无线发射端 / Host |
| 绿色 | 无线接收端 / Slave |

普通款出厂通常默认为有线模式。手册 p5。

### 4.2 KA 按键切换模式

1. 长按 KA，直到指示灯变为黄色。
2. 短按 KA，在模式间切换。
3. 长按 KA 保存；对应模式颜色快闪。
4. 重新上电后生效。

来源：手册 p6。

### 4.3 KB 按键自动配对

1. 第一台模块按住 KB 上电，约 2 秒后松开，等待紫色慢闪。
2. 第二台模块执行相同步骤。
3. 配对后两端分别变为蓝色主机和绿色从机。
4. 两端重新上电后生效。

来源：手册 p6。

## 5. U 盘配置文件

调试器连接电脑后会出现 U 盘，手册称其中有 `CONF.TXT` 和 `README.TXT`；修改并保存配置后需要重新上电。手册 p6。

| 参数 | 手册允许值/格式 | 含义 |
|---|---|---|
| `mode` | `usb`、`master`、`slave` | 有线、无线主机、无线从机 |
| `addr` | 8 个十六进制字符，例如 `688fa5e5` | 主从同步地址，必须完全一致 |
| `rate` | `2M`、`1M`、`500K`、`125K` | 空中速率；速率越低，手册称距离越远 |
| `esp_down` | `enable`、`disable` | 乐鑫芯片下载功能 |

### 手册内部歧义

- p6 的配置示例和参数表使用 `mode=master`；同页正文末尾出现 `mester`。这应视为手册拼写冲突，不能直接推断固件接受 `mester`。
- 实施时必须以设备实际 U 盘内的 `CONF.TXT`、`README.TXT` 和写入后的模式指示灯为准，优先使用示例和参数表中的 `master`，但在上电前仍需复核实机文件。

## 6. SWD、JTAG 与虚拟串口接线事实

### 6.1 SWD

手册 p9 的 SWD 图示为：

| 调试器 | 目标板 |
|---|---|
| `SWCLK/TCK` | `SWCLK` |
| `SWDIO/TMS` | `SWDIO` |
| `GND` | `GND` |
| `3.3V` | `3.3V` |
| `RX` | 目标板 `TX`，可选 |
| `TX` | 目标板 `RX`，可选 |

手册文字称 SWD 只需两根信号线，串口按需连接。无线模式的接收机还需要目标板提供工作电源。手册 p8-p9。

### 6.2 JTAG

手册 p9 给出 `TCK/TMS/TDI/TDO/GND/3.3V`，虚拟串口仍为 RX/TX 交叉连接。本项目 MSPM0G3507 当前基线使用 SWD，不需要自动切换到 JTAG。

### 6.3 复位

- 产品接口含 `RST`，手册同时宣称支持硬件和软件复位。手册 p3、p5。
- p9 的通用 SWD 图没有画出 RST 连接；因此“RST 是否必须连接、无线复位行为、复位极性和目标板侧具体连接”仍需实机和厂商说明关闭。

## 7. 软件支持范围

### 7.1 手册明确范围

- 手册写明 MDK 版本需 5.29 以上，IAR 版本需 8.32 以上。p5。
- p11-p13 演示在 MDK 中选择 `CMSIS-DAP Debugger`、检测 SWD 目标、配置复位与烧写算法。
- 手册称 Windows、Linux 和 macOS 可通过 OpenOCD 使用，Windows 10 及以上通常无需额外 CMSIS-DAP HID 驱动。p3-p4、p14。
- 手册没有提供 CCS/CCS Theia 的配置步骤，也没有声明已在 MSPM0G3507 上验证。

### 7.2 对当前 MSPM0/CCS 工程的外部复核（2026-07-30）

- TI MSPM0 SDK 2.11.00.07 的 CCS 指南列出的“已成功测试”探针是 TI XDS 系列和 SEGGER J-Link；未列 CMSIS-DAP。
- 当前工程 `targetConfigs/MSPM0G3507.ccxml` 明确绑定 `Texas Instruments XDS110 USB Debug Probe`，并使用 TI 的 XDS 驱动与 MSPM0 GEL/DSSM 流程。
- OpenOCD 官方文档支持 CMSIS-DAP v1 HID、v2 USB bulk 和 TCP 后端；当前 OpenOCD 文档也已包含 `mspm0` Flash Driver，可自动识别 MSPM0 版本并处理 MAIN/NONMAIN 区域。
- 上述事实只能证明“CMSIS-DAP + OpenOCD 是可研究的候选路径”，不能证明塔克无线桥、特定 OpenOCD 构建、MSPM0G3507 目标配置、复位、烧录和断点在本项目上已经通过。

外部来源：

- TI MSPM0 SDK CCS IDE Guide：<https://software-dl.ti.com/msp430/esd/MSPM0-SDK/latest/docs/english/tools/ccs_ide_guide/doc_guide/doc_guide-srcs/ccs_ide_guide.html>
- OpenOCD Debug Adapter Configuration：<https://openocd.org/doc/html/Debug-Adapter-Configuration.html>
- OpenOCD Flash Commands - `mspm0`：<https://www.openocd.org/doc/html/Flash-Commands.html>

## 8. 对本项目的证据结论

| 结论 | 状态 | 证据 |
|---|---|---|
| 可作为普通有线 CMSIS-DAP | `[厂家声明|未实测]` | 手册 p3-p5 |
| 可形成主机-从机无线 SWD 桥 | `[厂家声明|未实测]` | 手册 p3、p6-p9 |
| 支持虚拟串口透传 | `[厂家声明|未实测]` | 手册 p3-p4、p7-p9 |
| 可直接替换当前 CCS `.ccxml` 中的 XDS110 | `[不成立/未有支持证据]` | 手册未提 CCS；TI 已测试列表未列 CMSIS-DAP；当前 `.ccxml` 为 XDS110 专用 |
| 可通过 OpenOCD 调试 MSPM0G3507 | `[候选|软件文档支持，硬件未实测]` | OpenOCD 有 CMSIS-DAP 驱动和 MSPM0 Flash Driver |
| 无线下载 60 KB/s、距离 100 m | `[厂家声明|未实测]` | 手册 p3 |

## 9. 未执行

- 未连接或识别实物塔克 DAP；
- 未修改 `targetConfigs/MSPM0G3507.ccxml`；
- 未运行 OpenOCD、CCS 调试、构建、烧录、复位、串口或无线测试；
- 未接线、未上电；
- 未修改 `docs/` 之外的任何工程文件。

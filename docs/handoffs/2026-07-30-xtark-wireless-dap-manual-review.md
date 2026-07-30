# 交接：塔克无线高速 DAP 手册复核

日期：2026-07-30
会话任务：复核塔克无线高速 DAP V3.0.1 手册，并评估其替换 XDS110 的可行性
任务状态：完成
授权等级：L1（仅 docs 文档）

## 已确认事实

- 手册内部版本为 V3.0，PDF 文件名为 V3.0.1，共 14 页。
- 普通款支持有线、无线 Host、无线 Slave 三种模式，支持 CMSIS-DAP、SWD/JTAG、虚拟串口和 U 盘配置。
- 手册明确给出 MDK、IAR 和 OpenOCD 路径，没有 CCS/CCS Theia 配置说明。
- 当前工程 `targetConfigs/MSPM0G3507.ccxml` 明确绑定 XDS110；TI MSPM0 SDK 当前 CCS 指南的已测试探针列表未列通用 CMSIS-DAP。
- OpenOCD 官方文档具备 CMSIS-DAP 后端和 MSPM0 Flash Driver，因此塔克 DAP 可作为 OpenOCD 隔离验证候选，但尚不能作为 CCS 中 XDS110 的直接替代品。

## 本会话结果

- 本地保存：`docs/source-pdf/xtark-wireless-high-speed-dap-v3.0.1.pdf`
- 自动提取：`docs/extracted/xtark-wireless-high-speed-dap-v3.0.1/`
- 资料复核：`docs/reviewed/xtark-wireless-high-speed-dap-v3.0.1-review.md`
- 项目建议：`docs/hardware/modules/xtark-wireless-high-speed-dap.md`
- 未更新共享主文档和索引，避免影响正在工作的 Claude Code。
- 未修改 `docs/` 之外的工程内容；工作区中原有的 `empty.c`、`empty.syscfg` 和 `Debug/**` 修改属于本任务开始前已存在的 Claude Code 工作。

## 验证与边界

- 已提取全部 14 页文本，并以 180 DPI 渲染复核 p3、p5、p6、p8、p9、p11-p14。
- 已核对 PDF SHA-256：`451cdd099a6f5f4c4824b39e6c775a009af025e6093f06b53ba48424e22f969a`。
- 证据等级：厂家手册资料复核 + 项目静态核验 + 官方软件文档复核。
- NOT RUN：CCS/OpenOCD、构建、烧录、复位、串口、无线、接线、上电和探针切换。

## 当前阻塞 / 待用户确认

1. 实物是普通款一对，还是迷你主机加普通接收机。
2. 实物固件版本、CMSIS-DAP v1/v2、VID/PID、虚拟串口和 U 盘配置文件。
3. 接收机供电针的输入/输出性质、RST 接法以及目标板双供电风险。
4. OpenOCD 版本是否包含 MSPM0 Flash Driver，以及无线链路在 MSPM0G3507 上的烧录/断点/恢复实测。

## 下一步唯一动作

- 用户另行授权后，先做“仅枚举设备与备份 U 盘配置”的 L0/L1 识别任务；不要直接修改 `.ccxml` 或拔掉 XDS110 基线。

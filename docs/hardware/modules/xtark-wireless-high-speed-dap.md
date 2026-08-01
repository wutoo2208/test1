# 塔克无线高速 DAP：MSPM0G3507 项目使用建议

> **状态**：`CANDIDATE DEBUG PROBE / MANUAL-REVIEWED / NOT CONNECTED / NOT TESTED`。
>
> **当前结论**：适合列为移动小车阶段的无线调试候选，但目前不能直接取代 CCS 中已经验证和配置的 XDS110。优先保持 XDS110 作为基线与恢复探针，另行以 OpenOCD/CMSIS-DAP 路径做隔离验证。

## 1. 为什么有价值

对于会移动的小车，使用有线 SWD 会限制车体运动，并存在拉扯线缆、瞬时断线或地线回路风险。该设备把 CMSIS-DAP、虚拟串口和无线桥组合在一起，理论上可用于：

- 车轮架空或低风险运动阶段的无线烧录和断点调试；
- 不方便接 USB 线时的日志串口；
- 多台小车之间通过修改地址切换调试目标；
- 单个普通款切到有线模式作为备用 CMSIS-DAP。

这些均为候选用途，尚未在本项目实测。

## 2. 不应立即替换 XDS110 的原因

1. 当前 `targetConfigs/MSPM0G3507.ccxml` 绑定 XDS110，包含 TI XDS 驱动、1 MHz SWD 时钟和 MSPM0 专用调试流程。
2. TI MSPM0 SDK 当前 CCS 指南明确列出的已测试探针为 TI XDS 系列和 SEGGER J-Link，没有列出通用 CMSIS-DAP。
3. 塔克手册只给出 MDK、IAR 和 OpenOCD 路径，没有 CCS/CCS Theia 配置步骤。
4. MSPM0 的 DSSM Mass Erase、Factory Reset、Wait for Debug、Set Reset Mode 等恢复能力在 CCS 文档中依赖 TI 支持包、GEL 和 XDS110 复位线；不能假设第三方 DAP 完全等价。
5. 无线链路引入延迟、丢包、配对状态和供电状态，断点/单步/复位稳定性必须逐项验证。

因此当前策略应是：

```text
XDS110 = 已知可用的主基线与救援探针
塔克无线 DAP = 隔离验证中的候选移动调试链路
```

## 3. 本项目候选连接

### 3.1 最小 SWD 连接

| 塔克 DAP 接收机 | 天猛星 MSPM0G3507 | 项目规则 |
|---|---|---|
| `SWCLK/TCK` | `PA20/SWCLK` | 调试专用，禁止与其他模块共用 |
| `SWDIO/TMS` | `PA19/SWDIO` | 调试专用，禁止与其他模块共用 |
| `GND` | 开发板逻辑 GND | 必须共参考；功率回流不得经过细调试线 |
| `3.3V` | 目标电压参考/供电候选 | 连接性质需实机确认，禁止输出对输出 |
| `RST` | `NRST` 候选 | 手册 SWD 图未明确，需实机确认后再接 |

无线接收机需要目标板供电。手册允许 3.3 V 或 5 V并推荐 5 V，但本项目不得直接照搬：必须先确认普通款硬件版本、电源针是输入还是输出、目标板当前 USB/电池供电路径以及是否会反灌。

### 3.2 虚拟串口连接

手册要求交叉连接：DAP RX 接目标 TX，DAP TX 接目标 RX。

> **当前状态修正（用户确认）**：PA10/PA11 当前未接任何 UART0 外设或线束；没有已验证的 CH340/COM 链路。因此本文件中任何 DAP UART 接线、PA10 日志监听或 PA11 双驱动检查，均为未来候选步骤，当前不得执行或据此修改固件。
历史原理图资料中的 UART0 路由为（**非当前已验证接线**）：

```text
PA10 = UART0 TX / 板载 CH340 RX 路径
PA11 = UART0 RX / 板载 CH340 TX 路径
```

因此建议分两阶段：

1. 首次只接 `DAP RX <- PA10` 和 GND，用于只读日志；
2. 在确认板载 CH340 TX 不会同时驱动 PA11、不会发生输出争用后，才考虑 `DAP TX -> PA11`。

不能把 DAP 虚拟串口与无线 SPI0/nRF24L01 混为同一通道。DAP 的虚拟串口属于调试器无线桥，项目 nRF24L01 属于应用层无线模块，两者硬件和协议独立。

## 4. 模式与配置摘要

| 项目 | 配置 |
|---|---|
| 有线模式 | STA 红色；`mode=usb` |
| 电脑侧无线主机 | STA 蓝色；`mode=master` |
| 目标侧无线从机 | STA 绿色；`mode=slave` |
| 配对地址 | `addr`，主从必须相同，8 个十六进制字符 |
| 空中速率 | `2M`、`1M`、`500K`、`125K` |
| 生效方式 | 保存后重新上电 |

手册正文存在 `mester` 拼写，但配置示例和表格使用 `master`；实机以 U 盘内 `README.TXT` 和模式灯为准。

## 5. 推荐的隔离验证顺序

### 阶段 A：只识别，不接目标板

1. 确认购买的是普通款一对，还是“迷你主机 + 普通接收机”。
2. 电脑侧插入后记录：CMSIS-DAP HID 名称、VID/PID、固件版本、虚拟串口号和 U 盘内容。
3. 读取并备份两端 `CONF.TXT`、`README.TXT`。
4. 配对并确认主机蓝灯、从机绿灯；地址、速率一致。

### 阶段 B：有线模式建立基线

1. 继续断开 DRV、D36A 和电机功率；只保留逻辑板。
2. 先只连接 SWDIO、SWCLK、GND 和经确认的目标参考电压；RST 暂不接。
3. 使用 OpenOCD `cmsis-dap` 后端，以低速 SWD 起步，建议初始候选为 100 kHz 到 1 MHz，不直接使用手册截图的 10 MHz。
4. 只执行识别、halt、寄存器读取；之后才允许烧录已知安全的 LED/空工程。
5. 验证下载后读取校验、复位运行、断点、单步、重新连接和断电恢复。

### 阶段 C：无线模式

在阶段 B 全部通过后：

1. 将电脑侧设为 Host、目标侧设为 Slave；
2. 先在静止、近距离、无电机状态下重复识别、halt、烧录、校验和复位；
3. 再逐步增加距离，记录下载时间、重连次数和失败行为；
4. 最后才在架空轮子或受限运动阶段使用，调试线仍需提供应力释放，接收机必须固定。

### 阶段 D：虚拟串口

1. 先只接 PA10 到 DAP RX；
2. 验证 115200 8N1 日志；
3. 再审查 PA11 双驱动风险；
4. 不将串口成功等同于 SWD 烧录和断点稳定。

## 6. OpenOCD 候选方向

OpenOCD 官方文档支持：

```text
adapter driver cmsis-dap
cmsis-dap backend auto
transport select swd
```

当前 OpenOCD 文档含 MSPM0 Flash Driver，候选配置需要 MSPM0 target/board 脚本和足够新的 OpenOCD 构建。具体命令必须在未来 L2 授权下根据本机 OpenOCD 版本和实际 USB 枚举生成，当前不写入工程、不运行。

## 7. 失败时必须回退 XDS110

出现以下任一情况，停止无线 DAP 并回到 XDS110：

- 无法稳定识别 DAP/Access Port；
- 烧录后校验失败；
- reset/halt 状态不可控；
- 无线断开后目标保持异常 halt/reset；
- 无法执行 MSPM0 Mass Erase/Factory Reset 等恢复动作；
- 连接导致目标 3.3 V/5 V 异常、反灌或发热；
- 电机启动后无线调试链路频繁断开。

XDS110 在比赛前不应淘汰，应保留为：

- 已知可用的 CCS 探针；
- 救援和恢复探针；
- 对比无线 DAP 行为的基准设备。

## 8. 当前阻塞

1. 实物是普通款、迷你主机还是其他硬件修订版；
2. 两端实际固件版本、CMSIS-DAP v1/v2、VID/PID 和虚拟串口枚举；
3. 接收机 3.3 V/5 V 针在各模式下的输入/输出性质和保护；
4. RST 到 MSPM0 NRST 的推荐接法与无线复位行为；
5. 当前本机 OpenOCD 是否足够新并包含 `mspm0` Flash Driver；
6. 塔克无线桥在 MSPM0G3507 上的识别、烧录、校验、断点和恢复实测；
7. DAP UART TX 与板载 CH340 TX 同时连接 PA11 时的争用处理。

## 9. 授权边界

当前只完成资料整理。未授权并且未执行：

- 修改 `targetConfigs/MSPM0G3507.ccxml`；
- 修改 `.syscfg`、源码、Debug 生成物或构建设置；
- 安装/运行 OpenOCD；
- 切换实际调试探针；
- 接线、供电、烧录、串口或无线测试。

详细手册复核见 `docs/reviewed/xtark-wireless-high-speed-dap-v3.0.1-review.md`。

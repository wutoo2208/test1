# 宝嵌 USB 无线串口 V2.0 + NF-02-PA 协议复核

## 1. 来源

| 资料 | 归档位置 | SHA-256 |
|---|---|---|
| NF-02-PA 2.4G 产品规格书 V1.0，2 页 | `docs/source-pdf/nf-02-pa-v1.0.pdf` | `8ffeaa7d6ce669ec29b2a58a4c8a26aea0020d09b9682cab55587d718ff6df79` |
| 无线串口模块使用文档 USB 升级版 V2.0，13 页 | `docs/source-pdf/baoqian-usb-wireless-uart-v2.0.pdf` | `fc439ecd5ff3f597e9d3a81d5a4ff67483b08e5356b273a97cb2888cf1c75cb4` |
| 客户单片机测试程序 `API.h`，原始 GBK/GB18030 | `docs/extracted/baoqian-usb-wireless-uart-v2.0/reference-code/API.original-gbk.h` | `e577d8349ca8d03ea8946fce5d5ff11ab9998542b7c6c352142c532455cce01a` |
| `API.h` UTF-8 检索副本 | `docs/extracted/baoqian-usb-wireless-uart-v2.0/reference-code/API.utf8.h` | 仅转码和规范换行，逻辑未改 |

自动提取和关键页面图片位于：

- `docs/extracted/nf-02-pa-v1.0/`
- `docs/extracted/baoqian-usb-wireless-uart-v2.0/`

## 2. 结论摘要

这组资料足以关闭“不能盲扫的基础协议参数”中的大部分项目，但只能给出**出厂默认值和厂家例程契约**。COM7 适配器的当前实机参数仍需用其实际串口波特率发送 `AT?` 查询，不能假设仍为出厂状态。

## 3. 用户要求的参数矩阵

| 参数 | 文档结论 | 证据与注意事项 |
|---|---|---|
| `RF_CH` | 出厂默认 `0`，即 2.400 GHz；可配 `0..125`，对应 2.400..2.525 GHz，1 MHz 步进 | USB 手册 p9；`API.h` 写 `RF_CH=0`。实际 COM7 当前值必须 `AT?` 查询 |
| 地址宽度 | 固定 5 字节；目标侧应为 `SETUP_AW=0x03` | USB 手册 p7、p9。厂家例程定义 TX/RX 地址宽度为 5，但没有显式写 `SETUP_AW`，依赖芯片复位默认值 |
| 接收地址 | 出厂默认 `FF FF FF FF FF`；可用 `AT+RXA=` 修改 | USB 手册 p9-p10。当前实际值未知，必须查询 |
| 目标发送地址 | 出厂默认 `FF FF FF FF FF`；可用 `AT+TXA=` 修改 | 发送方目标地址必须等于接收方本地 `RX_ADDR_P0` |
| 地址字节序 | nRF24L01+ 多字节地址寄存器 SPI 规定 **LSByte first**；厂家 `API.h` 从数组下标 0 开始顺序写入 | `API.h` 的 `NRF24L01_Write_Buf()` 先发送 `pBuf[0]`。USB 手册没有明确 AT 显示的第一个字节是否称为 LSByte；非对称地址仍需用 `AT?` + 目标寄存器读回做一次验证 |
| 空口速率 | 250 kbps / 1 Mbps / 2 Mbps；出厂默认 2 Mbps | `AT+RATE=1/2/3`；USB 手册 p8-p10 |
| CRC | 支持 1-byte 或 2-byte；出厂默认 2-byte；没有关闭 CRC 的 AT 命令 | `AT+CRC=8/16`；`API.h` 使用 `CONFIG=0x0F/0x0E`，即 `EN_CRC=1`、`CRCO=1` |
| Auto ACK | 开启，仅 Data Pipe 0 | `API.h` 写 `EN_AA=0x01`、`EN_RXADDR=0x01`；USB 手册没有关闭 Auto ACK 的命令 |
| `SETUP_RETR` | 厂家目标 MCU 例程为 `0x1A` | ARD=1，对应 500 us 自动重发延迟；ARC=10，最多重发 10 次。USB 手册未直接显示该寄存器，因此属于“配套例程契约” |
| Payload 类型 | 静态 Payload | `API.h` 写 `RX_PW_P0`，未配置 `FEATURE`/`DYNPD`；USB 手册明确固定 32 字节 RF 包 |
| 静态 Payload 宽度 | 32 字节 | Byte0 为有效用户长度，Byte1..Byte31 为用户数据；用户单次 1..31 字节 |
| COM7 输出 | 对用户表现为原始用户字节流；内部长度字节不输出，没有文档声明额外帧头 | USB 手册 p7-p8、p12。两个 USB 模块实验发送 `12345`，接收串口显示 `12345`。COM 是字节流，不能依赖一次串口 read 对应一个 RF 包 |
| USB 串口波特率 | 出厂默认 9600；可配 4800/9600/14400/19200/38400/57600/115200 | `AT+BAUD=1..7`；USB 手册 p8。此前用 115200 打开 COM7 不证明参数正确，实物可能仍是 9600 |
| nRF 核心发射功率 | USB 产品文档为 0 dBm、LNA 开启 | USB 手册 p8-p10 |
| NF-02-PA 模块射频输出 | 最高 20 dBm，厂家标称 TX 电流 250 mA | NF-02-PA p2。外置 PA/LNA 模块的供电能力必须单独审核 |

## 4. 透明串口的 RF 帧

厂家定义固定 32 字节 RF Payload：

```text
Byte0      = N，本次用户数据长度，范围 1..31
Byte1      = 第 1 个用户字节
...
ByteN      = 第 N 个用户字节
ByteN+1..31 = 文档未定义的剩余区
```

### 4.1 PC/COM7 -> MSPM0

例如电脑发送 ASCII `ABCDE`：

```text
RF Payload = 05 41 42 43 44 45 ...
```

目标 MCU 必须：

1. 验证 `Byte0` 在 `1..31`；
2. 从 `Byte1` 开始取 `Byte0` 个字节；
3. 不把 Byte0 当作用户数据；
4. 不读取未使用的尾部区。

### 4.2 MSPM0 -> COM7

发送 `ABCDE` 时，目标 MCU 构造：

```c
uint8_t packet[32] = {5, 'A', 'B', 'C', 'D', 'E'};
```

USB 接收适配器根据 Byte0 取出 Byte1..ByteN，并向 COM 端输出用户字节。因此 COM7 侧看到 `ABCDE`，不是 `05ABCDE`。

### 4.3 未定义行为

手册没有说明：

- 剩余尾部字节是否清零；
- 连续串口字节按什么超时或长度条件分包；
- 一次发送超过 31 字节时如何分段；
- COM 接收 API 的 read 边界是否保持 RF 包边界；
- 丢包、重复包和乱序如何向 COM 端表现。

应用协议必须自行增加帧头、长度、序号和应用层校验，不能把 USB 透明串口的单次 read 当成完整业务帧。

## 5. AT 命令

所有命令要求：大写、英文半角标点、无空格。

| 功能 | 命令 |
|---|---|
| 查询当前配置 | `AT?` |
| 设置串口波特率 | `AT+BAUD=n`，`n=1..7` |
| 设置空口速率 | `AT+RATE=1/2/3` |
| 设置本地 RX 地址 | `AT+RXA=0xAA,0xBB,0xCC,0xDD,0xEE` |
| 设置目标 TX 地址 | `AT+TXA=0x11,0x22,0x33,0x44,0x55` |
| 设置频率 | `AT+FREQ=2.424G` |
| 设置 CRC | `AT+CRC=8` 或 `AT+CRC=16` |

`AT?` 应返回：串口波特率、目标地址、本地接收地址、频率、CRC、发射功率、空口速率和低噪声放大状态。

## 6. `API.h` 寄存器契约

厂家配套代码在 `NRF24L01_RT_Init()` 中执行：

```text
RX_PW_P0   = 32
TX_ADDR     = 调用者传入的 5 字节目标地址
RX_ADDR_P0  = 调用者传入的 5 字节本地/ACK 地址
EN_AA       = 0x01
EN_RXADDR   = 0x01
SETUP_RETR  = 0x1A
RF_CH       = 0
RF_SETUP    = 调用者传入 RATE
CONFIG RX   = 0x0F
CONFIG TX   = 0x0E
```

### 静态审查注意事项

- 代码是 8051 风格：使用 `sbit P1^x`、`intrins.h` 和软件位带 SPI，不能直接复制到 MSPM0。
- 软件 SPI 行为为 SCK 空闲低、先输出数据再拉高采样，MSB first，等价于 nRF24 常用 SPI Mode 0。
- `while (NRF_IRQ == 1);` 没有超时；硬件断线会永久阻塞，MSPM0 实现必须有超时和错误状态。
- 例程没有显式写 `SETUP_AW`，假设芯片仍处于 5 字节地址复位默认值；MSPM0 实现应显式配置并读回验证。
- 例程没有启用动态 Payload；`FEATURE` 和 `DYNPD` 应保持关闭并在初始化读回确认。
- 配套代码把 TX 完成和 MAX_RT 作为结果，但没有业务层序号、重复包去重或链路统计。

## 7. 地址字节序的实施规则

nRF24L01+ 对 `TX_ADDR` 和 `RX_ADDR_P0` 的 SPI 多字节访问要求 LSByte first；每个字节内部仍是 MSBit first。厂家例程按数组递增地址依次发送，因此：

```text
address[0] = 第一个经 SPI 写入的 LSByte
address[4] = 最后写入的 MSByte
```

USB 手册的 AT 显示未明确标注 LSByte/MSByte。为降低风险：

1. 第一次使用可保持对称默认地址 `FF FF FF FF FF`，此时字节序不影响匹配；
2. 改为非对称地址时，先对 COM7 执行 `AT+RXA=...` 和 `AT?`；
3. MSPM0 按 AT 显示的左到右顺序填入数组并写入；
4. 通过目标侧寄存器读回和一次发包验证；
5. 未验证前不要在固件文档中把 AT 左侧第一个字节永久命名为 MSByte。

## 8. NF-02-PA 电气事实

| 项目 | 规格 |
|---|---|
| 供电 | 1.9..3.6 V，典型 3.3 V |
| 发射电流 | 250 mA，20 dBm |
| 接收电流 | 23 mA，2 Mbps |
| Standby-I | 26 uA |
| SPI | 最高 10 Mbps |
| 空口速率 | 250 kbps / 1 Mbps / 2 Mbps |
| 信道 | 2400..2525 MHz，1 MHz 步进 |
| IRQ | 输出，低有效 |

DIP-8 引脚：

| Pin | 信号 |
|---:|---|
| 1 | VCC |
| 2 | GND |
| 3 | CSN |
| 4 | CE |
| 5 | MOSI |
| 6 | SCK |
| 7 | IRQ，低有效 |
| 8 | MISO |

这关闭了“NF-02-PA 型号本身”的 pin1/pin2 定义，但只有在用户确认车载实物确为 NF-02-PA、观察面和 Pin 1 一致后，才能更新冻结接线文档。当前不自动修改 Pin Plan。

## 9. 当前仍需实机关闭

1. COM7 当前实际波特率：先尝试出厂 9600，再查询 `AT?`；
2. COM7 当前 RF_CH、地址、速率和 CRC，可能已被商家或用户修改；
3. USB 适配器内部 Auto ACK 和 `SETUP_RETR` 是否与配套 `API.h` 完全一致；
4. AT 地址显示顺序与目标寄存器 LSByte/MSByte 的实测映射；
5. NF-02-PA 是否就是车载实物模块，而不是普通板载天线 nRF24L01；
6. 3.3 V 电源是否能承受至少资料给出的 250 mA 发射电流和负载瞬态；
7. COM 串口连续流的分包超时与超过 31 字节行为。

## 10. 未执行

- 未打开或发送 COM7；
- 未发送 AT 命令；
- 未修改 `.syscfg`、源码、Pin Plan 或 Debug 生成物；
- 未接线、供电或访问 SPI；
- 未构建、烧录、复位或进行无线通信测试。

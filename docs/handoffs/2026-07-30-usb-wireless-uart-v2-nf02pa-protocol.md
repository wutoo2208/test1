# 交接：USB 无线串口 V2.0 / NF-02-PA 协议参数

日期：2026-07-30
会话任务：从两份 PDF 和厂家 API.h 提取 COM7 无线串口对接参数
任务状态：完成
授权等级：L1（仅 docs 文档与资料归档）

## 已确认事实

- USB 无线串口 RF 包固定 32 字节：Byte0 是用户长度 1..31，Byte1..ByteN 是用户数据；COM 侧输出用户原始字节，不输出内部长度字节。
- 出厂默认：RF_CH=0、5 字节地址、TX/RX 地址均为 FF FF FF FF FF、2 Mbps、2-byte CRC、串口 9600。
- 厂家目标 MCU 例程：Pipe0 Auto ACK 开启，`SETUP_RETR=0x1A`，静态 Payload 32 字节，RX/TX `CONFIG=0x0F/0x0E`。
- nRF24 多字节地址通过 SPI 按 LSByte first 写；例程从数组下标 0 开始发送。AT 显示顺序的 LSByte 命名仍需非对称地址实测确认。
- NF-02-PA pin1=VCC、pin2=GND，典型 3.3 V；20 dBm 下资料标称 TX 电流 250 mA。
- COM7 当前实际参数仍未知，必须先以可能的出厂波特率 9600 查询 `AT?`，不能假设之前使用的 115200 正确。

## 本会话结果

- 归档：`docs/source-pdf/nf-02-pa-v1.0.pdf`
- 归档：`docs/source-pdf/baoqian-usb-wireless-uart-v2.0.pdf`
- 提取：`docs/extracted/nf-02-pa-v1.0/`
- 提取和例程：`docs/extracted/baoqian-usb-wireless-uart-v2.0/`
- 复核：`docs/reviewed/baoqian-usb-wireless-uart-v2-nf02pa-protocol-review.md`
- 实施契约：`docs/hardware/modules/baoqian-usb-wireless-uart-v2-nf02pa.md`
- 厂家 `API.h` 已同时保存原始 GBK 文件和仅转码的 UTF-8 副本；没有修改逻辑。
- 未修改共享主文档、Pin Plan、源码、`.syscfg` 或 Debug 文件。

## 验证与边界

- 两份 PDF 分别为 2 页和 13 页，已提取全部文本并复核关键页面图像。
- API.h 原始编码为 GBK/GB18030，SHA-256：`e577d8349ca8d03ea8946fce5d5ff11ab9998542b7c6c352142c532455cce01a`。
- 证据等级：厂家规格书 + 产品手册 + 配套例程静态复核。
- NOT RUN：COM7、AT 命令、SPI、SysConfig、构建、烧录、接线和上电。

## 当前阻塞 / 待用户确认

1. COM7 原始 `AT?` 回复和当前真实波特率。
2. 车载实物是否确为 NF-02-PA，以及 Pin 1 观察面。
3. 3.3 V 电源是否能承受至少 250 mA 发射电流和瞬态。
4. USB 适配器内部 `SETUP_RETR` 是否完全采用配套 API.h 值。
5. AT 地址显示顺序与 SPI LSByte first 的非对称地址实测。

## 下一步唯一动作

- 另行获得串口访问授权后，关闭 CCS 对 COM7 的占用，以 9600 8N1 发送 `AT?` 并保存完整原始回复；不要先恢复出厂或改参数。

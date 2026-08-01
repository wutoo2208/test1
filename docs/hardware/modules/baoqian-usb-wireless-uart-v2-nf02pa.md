# USB 无线串口 V2.0 / NF-02-PA：MSPM0 对接契约

> **状态**：`PROTOCOL-REVIEWED / FACTORY-DEFAULT-KNOWN / ACTUAL-COM7-CONFIG-UNKNOWN / NOT TESTED`。
>
> 本文提供 Claude Code 实现无线兼容层所需的资料契约。它不授权修改当前源码、SysConfig、接线或上电。

## 1. 先纠正当前测试路径

照片中的 COM7 是宝嵌 CH340T + 控制器 + nRF24L01 USB 无线串口适配器，不是天猛星板载 CH340。**当前用户确认 MSPM0 的 UART0（PA10/PA11）未接外设/线束，因此不得把历史 `@BOOT/@STATUS` 设想写成当前可用路径，也不会自动进入 COM7。**

要让 COM7 收到 MSPM0 数据，MSPM0 必须：

1. 通过 SPI0 初始化车载无线模块；
2. 与 COM7 适配器使用相同 RF_CH、地址、速率、CRC、Auto ACK 和 Payload 契约；
3. 把业务字节封装成固定 32 字节 RF Payload；
4. 执行 `W_TX_PAYLOAD` 和 CE 发射；
5. 处理 TX_DS、MAX_RT、超时和重发结果。

## 2. 出厂默认兼容配置

以下仅用于“COM7 仍是出厂设置”的首次候选。真正实施前必须通过 `AT?` 查询实际值。

| 寄存器/策略 | 候选值 |
|---|---|
| `SETUP_AW` | `0x03`，5 字节地址 |
| `TX_ADDR` | `FF FF FF FF FF`，LSByte first 写入 |
| `RX_ADDR_P0` | `FF FF FF FF FF`，LSByte first 写入 |
| `EN_AA` | `0x01`，Pipe 0 Auto ACK 开启 |
| `EN_RXADDR` | `0x01`，Pipe 0 开启 |
| `SETUP_RETR` | `0x1A`，500 us，最多 10 次 |
| `RF_CH` | `0`，2.400 GHz |
| 空口速率 | 2 Mbps |
| nRF 核心功率 | 0 dBm，USB 文档默认；NF-02-PA 外部 PA 最大输出另计 |
| CRC | 2-byte，不能关闭 |
| `RX_PW_P0` | `32` |
| `FEATURE` / `DYNPD` | 关闭，静态 Payload |
| RX `CONFIG` | `0x0F` |
| TX `CONFIG` | `0x0E` |

不要在未查询 COM7 前把这些值写死为最终比赛参数。

## 3. RF Payload 编解码

### 3.1 发送到 COM7

```c
bool radio_uart_encode(const uint8_t *src, size_t len, uint8_t out[32])
{
    if ((src == NULL) || (out == NULL) || (len == 0U) || (len > 31U)) {
        return false;
    }

    memset(out, 0, 32U);
    out[0] = (uint8_t) len;
    memcpy(&out[1], src, len);
    return true;
}
```

COM7 透明串口预计输出 `src[0..len-1]`，不会输出内部 Byte0。

### 3.2 接收 COM7 数据

```c
bool radio_uart_decode(const uint8_t in[32], uint8_t *dst, size_t cap,
    size_t *decoded_len)
{
    size_t len;

    if ((in == NULL) || (dst == NULL) || (decoded_len == NULL)) {
        return false;
    }

    len = in[0];
    if ((len == 0U) || (len > 31U) || (len > cap)) {
        return false;
    }

    memcpy(dst, &in[1], len);
    *decoded_len = len;
    return true;
}
```

以上只是协议适配示例，不是对当前工程源码的授权修改。

## 4. 应用层仍需自己的帧

COM7 是连续字节流，RF 包边界不会可靠映射为上位机串口 read 边界。建议业务层在 31 字节以内使用独立小帧，例如：

```text
MAGIC | TYPE | SEQ | LEN | DATA | CRC8
```

必须处理：

- 部分读取与多包合并；
- 重复包去重；
- 丢包和超时；
- MAX_RT；
- 无效长度；
- 旧数据/乱序；
- 禁止把无线链路作为正式比赛人工遥控通道。

## 5. COM7 查询顺序

当前不应直接以 115200 假设 COM7 参数。厂家出厂默认是 9600。未来获得串口访问授权后，建议：

1. 关闭 CCS 对 COM7 的占用；
2. 以 `9600 8N1` 打开 COM7；
3. 发送大写 ASCII `AT?`；
4. 保存原始回复；
5. 如无回复，再检查当前波特率是否被改为 115200，而不是立刻恢复出厂；
6. 从回复中冻结 RF_CH、TX 地址、RX 地址、CRC 和速率。

不要在获得原始 `AT?` 回复前修改配置。

## 6. NF-02-PA 与冻结 Pin Plan

如果用户确认车载模块确为 NF-02-PA，则资料给出的模块端引脚是：

```text
1 VCC, 2 GND, 3 CSN, 4 CE,
5 MOSI, 6 SCK, 7 IRQ(active-low), 8 MISO
```

这与当前 v1.2 信号编号 3..8 一致，并补齐了 pin1/pin2：

```text
pin1 = VCC, 1.9..3.6 V, typical 3.3 V
pin2 = GND
```

但 NF-02-PA 在 20 dBm 下资料标称发射电流 250 mA，不能未经电源预算直接接开发板 3.3 V。当前冻结 Pin Plan 仍保持不变，等待：

- 实物型号和 Pin 1 观察面确认；
- 3.3 V 电源持续/瞬态能力；
- 本地去耦和回流方案；
- 首次上电限流方案。

## 7. 厂家 `API.h` 只能作参数参考

归档例程是 8051 风格软件 SPI，存在：

- `sbit` 和 `P1^x` 平台绑定；
- 无超时 IRQ 等待；
- 延时函数未标定；
- 隐式依赖 `SETUP_AW` 复位默认值；
- 缺少应用层帧、链路统计和故障恢复。

MSPM0 实现应使用 SysConfig + DriverLib 的 SPI0/GPIO/IRQ，并重新设计超时和状态机；不得复制该文件中的 GPIO 和忙等实现。

## 8. Claude Code 下一步输入

协议资料阶段已完成。Claude Code 后续如继续，应先读取：

1. `docs/reviewed/baoqian-usb-wireless-uart-v2-nf02pa-protocol-review.md`
2. `docs/hardware/modules/baoqian-usb-wireless-uart-v2-nf02pa.md`
3. `docs/extracted/baoqian-usb-wireless-uart-v2.0/reference-code/API.utf8.h`
4. `docs/pin-plan/mspm0g3507-pin-plan-frozen-v1.2.md`

下一门槛不是立即写驱动，而是获取 COM7 原始 `AT?` 回复和确认车载实物确为 NF-02-PA。

## 9. 当前未授权

- 不修改 `.syscfg`；
- 不修改源码或生成文件；
- 不构建或烧录；
- 不打开 COM7 或发送 AT；
- 不接线、不供电、不访问 SPI。

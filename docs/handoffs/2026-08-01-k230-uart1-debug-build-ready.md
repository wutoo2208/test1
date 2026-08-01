# 交接：K230 UART1只读诊断Debug镜像待烧录

日期：2026-08-01

## 用户授权和边界

- 允许修改`empty.c`和`firmware_tests/test_firmware.py`，并已先行批准`empty.syscfg`配置。
- 允许SysConfig生成、Python测试和Debug构建。
- 禁止修改REQ-002、烧录、接通12V或启动D36A。
- 用户确认当前未接电源；本轮未访问任何硬件。

## 实施

- `empty.syscfg`新增`K230_UART`：UART1、PA9 RX、PA8 TX、115200、FIFO、RX中断、无DMA。
- `empty.c`新增UART1 ISR，将RX FIFO字节送入现有`K230Link_pushRxByteFromIsr()`。
- 主循环调用`K230Link_service()`并将只读诊断数据镜像到RAM。
- 未修改`app/req002.c`、循迹PID、车轮PWM、电机方向、KEY2或D36A控制。

## 验证

- SysConfig 1.26.2严格隔离验证：PASS，0警告。
- 正式Debug SysConfig生成：PASS；生成宏确认UART1/PA9/PA8/115200。
- Python合同测试：10/10 PASS。
- Debug TI-Clang构建/链接：PASS，未见编译或链接警告。
- 烧录、复位、串口、K230实物通信：NOT RUN。

## Debug诊断镜像

- 文件：`Debug/test1.out`
- 大小：551800 bytes
- SHA-256：`D5053F353AF5F37987FF2DA0DDB2F35FB9F846D4622D6EF35A62165DDA642D4C`
- 这是Debug安全构建，默认不开放REQ-002执行器。

## RAM诊断镜像

符号：`gK230RuntimeDiag`

```text
base = 0x20200518
size = 0x84 bytes
```

所有字段均为4字节小端；位置和速度为有符号32位，其余为无符号32位：

| 偏移 | 字段 |
|---:|---|
| 0x00 | serviceCount |
| 0x04 | haveSample |
| 0x08 | status |
| 0x0C | positionTenthMm |
| 0x10 | velocityTenthMmPerSecond |
| 0x14 | confidence |
| 0x18 | pixelX |
| 0x1C | pixelY |
| 0x20 | sequence |
| 0x24 | receivedMs |
| 0x28 | frameAgeMs |
| 0x2C | sequenceAgeMs |
| 0x30 | linkFresh |
| 0x34 | measurementUsable |
| 0x38 | predicted |
| 0x3C | sequenceAdvanced |
| 0x40 | lostPayloadCanonical |
| 0x44 | pendingBytes |
| 0x48 | rxBytes |
| 0x4C | queueOverflows |
| 0x50 | parserResetsAfterOverflow |
| 0x54 | headerResyncs |
| 0x58 | validFrames |
| 0x5C | measuredFrames |
| 0x60 | predictedFrames |
| 0x64 | lostFrames |
| 0x68 | crcErrors |
| 0x6C | statusErrors |
| 0x70 | lostPayloadMismatches |
| 0x74 | duplicateSequences |
| 0x78 | sequenceGapEvents |
| 0x7C | timeoutEvents |
| 0x80 | linkRecoveries |

## 下一门槛

烧录前需要：

1. 目标板逻辑域由DAP或USB供电；12V、电池和D36A功率保持断开。
2. K230独立稳定5V供电；K230 TX(IO9)接PA9，GND共地；不得将5V接到PA9。
3. 明确授权使用Horco CMSIS-DAP UID `da941ca0`烧录上述Debug镜像，禁止软件复位，之后由用户物理RESET。
4. 复位后只读RAM `0x20200518/0x84`，不暂停、不写内存；先验证`rxBytes`和`validFrames`，再由用户移动钢球确认O、+5 cm和-5 cm符号。

## 本会话烧录尝试结果

- 用户确认目标逻辑域和K230已供电、K230 TX(IO9)接PA9、GND共地，12V/电池/D36A功率断开，并明确授权烧录。
- 烧录前重新核对`Debug/test1.out` SHA-256：`D5053F353AF5F37987FF2DA0DDB2F35FB9F846D4622D6EF35A62165DDA642D4C`，完全匹配。
- 外部审批器在`pyocd list --no-config`启动前因当前任务上下文过长拒绝执行。
- 探针：NOT ACCESSED。
- 擦除/编程/校验：NOT RUN。
- 软件复位：NOT RUN。
- 板上固件不得视为K230 Debug版本。
- 下一新任务只需读取本交接，重新核对哈希，枚举UID `da941ca0`并按既定参数无软件复位烧录；成功后要求用户物理RESET，再只读`0x20200518/0x84`。
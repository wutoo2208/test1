# 交接：DAPLink 烧录与 MotorSelfTest 无线 SPI0 调试

日期：2026-07-31  
任务：使用外部 CMSIS-DAP 烧录固定 MotorSelfTest，并验证 MSPM0G3507→SPI0→NF-02-PA→COM7 无线诊断链路  
状态：烧录成功；无线单向诊断发送实测成功；电机动作尚未执行

## 1. 探针与烧录后端

- 已识别探针：`Horco CMSIS-DAP v2`，UID `da941ca0`。
- 探针复合接口：CMSIS-DAP v2、虚拟串口 COM8、配置盘 E:。
- 配置盘 `CONF.TXT`：`mode=master`、`rate=2M`；该 DAP 当前使用无线主机模式。
- 使用临时 pyOCD 0.45.1 和 TI CMSIS-Pack `TexasInstruments.MSPM0G_DFP 1.2.1`。
- SWD 以 100 kHz 成功识别 Cortex-M0+，CPUID `0x410CC601`。
- 未使用 XDS110、CCS Debug Session、Mass Erase、Factory Reset 或 NONMAIN 操作。

## 2. 烧录前后证据

烧录前板上向量：

```text
SP    = 0x20208000
Reset = 0x0000B2FB
```

这与固定 MotorSelfTest 不符，证明板上原先不是目标镜像。

烧录并修复后的镜像：

```text
MotorSelfTest/test1_motor_selftest.out
SHA-256: 2095D7E67847DBC1CF16E3F212ECF3F5426EAA66B37247C9016786F4B66382CF
```

烧录结果：

```text
MAIN Flash sector erase/program 成功
Flash 向量：SP=0x20208000，Reset=0x0000C393
CPU：Running
```

## 3. MotorSelfTest 静态安全事实

- `MOTOR_SELFTEST_BUILD=1`。
- Cortex-M0+ / Thumb v6-M 原生镜像。
- 单轮测试占空比 `120‰`（12%）。
- 单次持续 `1000 ms`。
- SysTick 超时调用 `BoardSafety_stop()`。
- 本轮没有发送 `motor_test_left` 或 `motor_test_right`，没有执行电机动作。

## 4. 无线问题定位与最小修复

COM7 `AT?` 实机查询结果：

```text
串口：9600 8N1
RF_CH：0 / 2.400 GHz
TX 地址：FF FF FF FF FF
RX_ADDR_P0：FF FF FF FF FF
空口速率：2 Mbps
CRC：16 位
发射功率：0 dBm
外部 PA：开启
```

发现 `drivers/nrf24_ptx.c` 的 `rfSetupValue()` 只设置速率位：2 Mbps 时写 `RF_SETUP=0x08`，实际对应 -18 dBm，与厂家固定 0 dBm 不一致。

已做最小修复：增加 `NRF_RF_PWR_0_DBM=(3U<<1U)`，所有速率返回值都包含 0 dBm 功率位；2 Mbps 现在写 `RF_SETUP=0x0E`。

验证：TI clang 5.1.1 LTS 增量编译和链接成功，无编译/链接错误；未运行 SysConfig。

## 5. 无线实测证据

修复前/早期测试曾出现：

```text
spiErrors=0
initErrors=0
txSuccess=0
maxRetry=1
```

修复后通过 SWD 读取运行状态：

```text
spiErrors=0
initErrors=0
txSuccess=1
maxRetry=0
txTimeout=0
```

当无线 DAP 主机仍插在电脑时，COM7 多次监听为 0 字节或 RF 结果不稳定。拔掉电脑侧 DAP 主机、保留目标逻辑供电并按 RESET 后，COM7@9600 原始 Hex 收到 24 字节：

```text
40 52 46 54 45 53 54 20 4E 46 30 32 50 41 20 4C
49 4E 4B 20 4F 4B 0D 0A
```

ASCII：

```text
@RFTEST NF02PA LINK OK\r\n
```

证据结论：`MSPM0G3507→SPI0→NF-02-PA→2.4 GHz→USB 无线接收器→CH340/COM7` 单向诊断链路实测成功。无线 DAP 主机与 NF-02-PA 同时工作存在明显干扰/共存风险，后续测试应避免同时使用，或另行规划频道和时序。

## 6. 当前限制

- 当前 nRF 驱动是 PTX 发送端；已证明 MCU→电脑无线日志，不代表 COM7→MCU 无线命令接收已实现。
- `DiagConsole_reportBoot()` 当前早于 `Nrf24Ptx_init()`，因此 `@BOOT/@STATUS` 不会进入无线队列；目前无线只发送一次 `@RFTEST NF02PA LINK OK`。
- 若要用无线作为完整调试控制台，需要另行实现/批准 PRX 命令接收，或调整初始化顺序并设计双向协议。
- COM8 是 DAP 虚拟串口，但 PA10/PA11 当前未接 DAP UART，因此本轮未使用 COM8。

## 7. 未执行

- SysConfig：NOT RUN / NOT MODIFIED
- PID、循迹、K230 和正式题目功能代码：NOT MODIFIED
- 电机功率与运动测试：NOT RUN
- 编码器手转/速度测试：NOT RUN
- 地面测试：NOT RUN

## 8. 下一步建议

保持未来正式功能代码由 Claude Code 负责。下一步只能领取一个任务：

1. 若优先验证无线日志：由 Claude Code 审阅最小 RF 功率修复，并决定是否在 MotorSelfTest 中让 nRF 先初始化、再输出 `@BOOT/@STATUS`；或
2. 若优先进入 L4 电机测试：恢复一个可发送 `motor_status/motor_test_left/motor_test_right/stop` 的命令输入路径。当前无线仅单向 PTX，不能从 COM7 向 MCU 下发这些命令；推荐使用 DAP 虚拟 UART TX→PA11/UART0_RX，日志仍由 COM7 接收，首次只发送 `motor_status`。
## 9. 后续状态更新：用户授权 MAIN 全擦除

用户发现右轮在未发送测试命令时转动，随后物理断开 12 V 电机功率并明确授权仅擦除 MAIN。

已通过 pyOCD 对 `0x00000000-0x00020000` 执行逐扇区擦除，日志覆盖全部 128 个 1 KB 扇区，最后一个扇区为 `0x0001FC00`。未请求或执行 `--chip`、`--mass`、Factory Reset、BSL 或 NONMAIN 擦除。

擦除结束后工具报告 board-uninit 错误；目标逻辑域恢复并按复位后，MSPM0G3507 pack、under-reset 及通用 Cortex-M 连接仍分别出现 `FAULT ACK`、`Invalid AP address (#0)` 或 `No cores were discovered`。因此当前状态必须记录为：

```text
MAIN ERASE COMMAND COMPLETED
READBACK VERIFICATION BLOCKED BY SWD RECONNECT FAILURE
MOTORSELFTEST NOT PRESENT / NOT RUNNING
MOTOR POWER MUST REMAIN DISCONNECTED
```

在恢复 SWD、重新烧录、验证向量和测得四个 DRV 输入停止态均接近 0 V 前，禁止恢复 12 V 电机功率或进入 L4。
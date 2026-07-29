# DRV8870 双路电机驱动模块原理图 PDF 复核

> **复核状态**：资料复核；未接线、未上电、未测量。  
> **原始 PDF**：`docs/source-pdf/drv8870-dual-module-schematic.pdf`，SHA-256 `BF24A993CEA7A9BE6E9AB0A790EBBE847132493F71463F4822A05523ACD5F001`。  
> **页面范围**：唯一 1 页，以下硬件事实均引自原 PDF p1，并通过渲染图复核。

## 1. 归档与页面复核

| 资料 | 路径 |
|---|---|
| 原始 PDF | `docs/source-pdf/drv8870-dual-module-schematic.pdf` |
| 文字提取 | `docs/extracted/drv8870-dual-module-schematic/drv8870-dual-module-schematic.md` |
| 全页渲染 | `docs/extracted/drv8870-dual-module-schematic/images/page-001.png` |
| 逻辑接口细节 | `docs/extracted/drv8870-dual-module-schematic/images/h3-detail.png` |
| 两路驱动细节 | `docs/extracted/drv8870-dual-module-schematic/images/drivers.png` |
| 供电排针细节 | `docs/extracted/drv8870-dual-module-schematic/images/h1-power-detail.png` |

## 2. 已确认的原理图事实

| ID | 硬件事实 | 原 PDF 页码 | 证据边界 |
|---|---|---|---|
| `DRV-SCH-001` | 模块使用两颗 `DRV8870DDAR-S`，分别为 U1 与 U2。 | p1 | 证明原理图设计，不证明实物 PCB/芯片批次与图一致。 |
| `DRV-SCH-002` | 两颗 DRV8870 的 `VBB` 都接 `VIN`，`VREF` 都接 `3V3`；两颗 `LSS` 分别经 R1/R2 的 `100 mΩ` 电阻接 GND。 | p1 | PDF 没有给出电流限值计算、热设计或连续堵转能力。 |
| `DRV-SCH-003` | A 通道控制网络为 `AIN1`→U2 `IN1`、`AIN2`→U2 `IN2`；输出网络为 `AOUT1`/`AOUT2`。 | p1 | 网络命名不等于当前 MSPM0 的外设/引脚配置。 |
| `DRV-SCH-004` | B 通道控制网络为 `BIN1`→U1 `IN1`、`BIN2`→U1 `IN2`；输出网络为 `BOUT1`/`BOUT2`。 | p1 | 同上。 |
| `DRV-SCH-005` | 模块有 `VIN_IN` 与 `VIN` 网络，SW4（`SS-12F44-G5`）位于二者之间。 | p1 | 只证明图中开关/网络关系；未证明开关电流、保险、反接或故障保护。 |
| `DRV-SCH-006` | 图中 U42 为 `XL1509-5.0E`，从 `VIN` 生成 `+5V`；可见 L1=`68 uH`、D1=`SS34` 和输入/输出电容网络。 | p1 | 不能据此确认实际模块的输出电流、纹波、温升或与电机噪声共存能力。 |
| `DRV-SCH-007` | 图中 U3 为 `AMS1117-3.3`，输入接 `+5V`、输出为 `3V3`。 | p1 | 不等于 3.3 V 域可向任意外设提供足够电流。 |
| `DRV-SCH-008` | H3 为 2×4 逻辑/编码器接口：1=`AIN1`、3=`AIN2`、5=`EA1`、7=`EB1`；2=`BIN1`、4=`BIN2`、6=`EB2`、8=`EA2`。 | p1 | 以 PDF 原理图符号中的焊盘号/网络标签为准；实际排针面向和线束方向仍须实物确认。 |
| `DRV-SCH-009` | U40/U41 为两组 8 针电机/编码器连接器，图中可见 `AOUT1/AOUT2/EA1/EB1` 与 `BOUT1/BOUT2/EA2/EB2`、GND、3V3 相关网络。 | p1 | 本记录不猜测未清晰标注的每个物理针位；接线前应按原图和实物丝印逐针复核。 |
| `DRV-SCH-010` | H1 为电源分配排针，图中出现 GND、`3V3` 与 `+5V` 网络。 | p1 | 未把图像中未逐针明确复核的 H1 针位序列写成最终线序。 |

## 3. 与天猛星 V2.0 拓展板的关系

天猛星 V2.0 EPRO 中的 U2/U3（标注 TB6612 模块）使用了 `AIN1`、`AIN2`、`BIN1`、`BIN2`、`EA1`、`EB1`、`EA2`、`EB2`、`AOUT*`、`BOUT*`、`VBAT_IN` 与 GND 等同名网络；本 DRV8870 原理图也使用这些控制、编码器和电机输出网络。来源：天猛星 EPRO `SCH-1` / `PCB` 静态网络记录；DRV 原理图 p1。

这支持“**可通过明确的转接线/线束复用同名信号**”这一候选，但不等于“可机械直插且无条件兼容”。仍须分开确认：

1. 电源：天猛星 `VBAT_IN` 到 DRV 模块 `VIN_IN`/`VIN` 的实际接线点、总开关、保险和电流余量；
2. 控制：天猛星 AIN/BIN 线在目标固件中的默认安全状态、PWM 方式和电平；
3. 编码器：H3 的交错双列针号与实物线束方向；
4. 电机：`AOUT*`/`BOUT*` 与电机端子及转向定义；
5. 供电与逻辑：DRV 模块的 `3V3`/`+5V` 输出不得未经电源树审查反向给 MSPM0 或天猛星拓展板供电。

## 4. 当前项目的关键风险和待确认

| ID | 待确认项 | 需要的证据 |
|---|---|---|
| `DRV-SCH-Q01` | 实物双路 DRV8870 模块与该 PDF 的元件、接口丝印和版本是否一致。 | 实物正反面高清照片、芯片/连接器近照。 |
| `DRV-SCH-Q02` | 天猛星拓展板 TB6612 接口与 DRV H3/U40/U41 的**逐针线束表**。 | 两份原理图 + 实物排针面向 + 线束照片；待用户确认后建立。 |
| `DRV-SCH-Q03` | VIN/VBAT、5V、3V3 的唯一供电源及禁止反向供电规则。 | 实际电源树决策和模块实物版本。 |
| `DRV-SCH-Q04` | R1/R2=100 mΩ、VREF=3V3 所对应的实际限流、温升和 3.2 A 堵转工况。 | DRV8870 芯片 datasheet 与后续获授权的台架测试。 |
| `DRV-SCH-Q05` | H3 中 EA/EB 的物理线序与两个电机的 A/B 相、正反转定义。 | 实物线束标识、编码器数据与低风险台架验证。 |

## 5. 本轮未执行

- 未修改 `.syscfg`、CCS 工程、源码或生成文件；
- 未构建、烧录、连接调试器或串口；
- 未接线、上电、测量 VIN/5V/3V3、控制输入或编码器信号；
- 未驱动电机，未验证过流、温升、制动或失效安全行为。
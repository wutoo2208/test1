# MSPM0G3507 Pin Plan v1.5 — U12 六路巡线与 GY OLED 总线重构

> **状态**：`FROZEN-DESIGN / USER-APPROVED / NOT SYSCONFIG-APPLIED / NOT WIRED`。  
> **冻结日期**：2026-07-31。  
> **基线链**：v1.2 主资源表 + v1.3 START_BUTTON/蜂鸣器修订 + v1.4 KEY2 实物映射修订。  
> **v1.4 快照**：SHA-256 `BE3C0819221B9BD478FB2073517A7A7D30F9A2AA9AE45F97C6F0D710ACAB7876`；本文件不修改 Claude Code 持有的 v1.4。  
> **授权边界**：本版只冻结文档中的资源与接线设计；不授权修改 `.syscfg`、源码、生成文件、构建、烧录、接线、上电或运动测试。

## 0. 当前唯一性

从本版起，当前设计资源按以下链解释：

1. `mspm0g3507-pin-plan-frozen-v1.2.md`：未被后续修订覆盖的基础 Owner；
2. `mspm0g3507-pin-plan-frozen-v1.3.md`：PB21/PB27 修订；
3. `mspm0g3507-pin-plan-frozen-v1.4.md`：PA21 KEY2、PB21/PB27 修订；
4. **本文件 v1.5**：LineFollower、OLED、MPU6050、U12、U8、PA0/PA1、PA28/PA31、PB2/PB3 修订。

配套唯一线束文件：

- `mspm0g3507-adapter-harness-v1.5.md`

以下文件转为历史候选，不得施工：

- `mspm0g3507-pin-plan-linefollower-6ch-v1.5-candidate.md`
- `mspm0g3507-adapter-harness-linefollower-6ch-v1.5-candidate.md`

## 1. 用户批准的拓扑

```text
I2C0 / PA28 SDA / PA31 SCL
    └── U12 → HiWonder/AiBlock LineFollower_6CH V1.0 / 0x5C

I2C1 / PB3 SDA / PB2 SCL
    └── 原 MPU6050 四针接口 → OLED / 0x3C 或 0x3D

MPU6050
    └── NOT FITTED / 暂不装车
```

设计目的：

- 六路巡线独占 I2C0，避免 OLED 刷新和 MPU 采样占用巡线控制总线；
- OLED 独占 I2C1；
- 移除 MPU6050，减少上拉、地址、采样、标定和机械安装复杂度；
- U12 原网络无需 PCB 切线，直接复用 PA28/PA31 的原厂 I2C0 功能；
- H10 不再承担巡线模块接线，H10 pin1/2/6 的无线资源保持不变。

## 2. v1.5 Owner 修订

| MCU引脚/资源 | v1.4以前Owner | v1.5唯一Owner | 物理路径 | 状态 |
|---|---|---|---|---|
| PA28 | 未分配 | LineFollower SDA | U21 pin4 → 扩展pad22/CSB_R → U12 pin3 | FROZEN-DESIGN / SYSCONFIG-PENDING |
| PA31 | 未分配 | LineFollower SCL | U21 pin6 → 扩展pad23/CSB_T → U12 pin2 | FROZEN-DESIGN / SYSCONFIG-PENDING |
| PB3 | MPU6050 SDA | OLED SDA | GY_SDA → 原MPU接口pad4 | FROZEN-DESIGN / SYSCONFIG-PENDING |
| PB2 | MPU6050 SCL | OLED SCK（I2C1 SCL） | GY_SCL → 原MPU接口pad3 | FROZEN-DESIGN / SYSCONFIG-PENDING |
| PA0 | OLED SDA | DNC / RELEASED | 旧U8 pad4；板载4.7k上拉网络仍存在 | FROZEN-DNC |
| PA1 | OLED SCL | DNC / RELEASED | 旧U8 pad3；板载4.7k上拉网络仍存在 | FROZEN-DNC |
| PA25 | TCRT OUT1 | DNC / RELEASED | 旧H10 pin7 | FROZEN-DNC |
| PA27 | TCRT OUT2 | DNC / RELEASED | 旧H10 pin8 | FROZEN-DNC |
| PA16 | TCRT OUT3 | DNC / RELEASED | 旧H10 pin3 | FROZEN-DNC |
| PA14 | TCRT OUT4 | DNC / RELEASED | 旧H10 pin4 | FROZEN-DNC |
| PB20 | TCRT OUT5 | DNC / RELEASED | 旧H10 pin5 | FROZEN-DNC |
| I2C0 | OLED | LineFollower_6CH only | PA28 SDA、PA31 SCL | FROZEN-DESIGN |
| I2C1 | MPU6050 | OLED only | PB3 SDA、PB2 SCL | FROZEN-DESIGN |
| U12 | DNC / 超声波预留 | LineFollower_6CH四线接口 | pin1 5V、pin2 SCL、pin3 SDA、pin4 GND | FROZEN-DESIGN |
| 原MPU6050接口 | MPU6050 | OLED四线接口 | pad1 5V、pad2 GND、pad3 SCL、pad4 SDA | FROZEN-DESIGN |
| U8 OLED接口 | OLED | DNC | pad1 GND、pad2 5V、pad3 PA1、pad4 PA0 | FROZEN-DNC |
| MPU6050模块 | 车体辅助候选 | NOT FITTED | 不接线、不配置、不采样 | USER-APPROVED |

这些释放引脚只是 DNC，不代表自动授权分配给其他模块。

## 3. U12到LineFollower完整网络

扩展板与开发板资料交叉确认：

```text
U12 pin1 → 5V
U12 pin2 → CSB_T → 扩展pad23 → U21 pin6 → PA31 / I2C0 SCL
U12 pin3 → CSB_R → 扩展pad22 → U21 pin4 → PA28 / I2C0 SDA
U12 pin4 → GND
```

LineFollower资料端口顺序为 `5V / GND / SDA / SCL`，所以必须制作交叉线束：

```text
LineFollower 5V  → U12 pin1
LineFollower GND → U12 pin4
LineFollower SDA → U12 pin3 / PA28
LineFollower SCL → U12 pin2 / PA31
```

禁止把模块四针插头原样直插U12。

## 4. 原MPU接口到OLED完整网络

原MPU6050接口：

```text
pad1 = 5V
pad2 = GND
pad3 = GY_SCL / PB2 / I2C1 SCL
pad4 = GY_SDA / PB3 / I2C1 SDA
```

OLED实物照片确认针序为 `GND / VDD / SCK / SDA`（资料名 `GND / VCC / SCL / SDA`），所以必须使用转接线：

```text
OLED VDD（资料VCC） → 原MPU接口 pad1 / 5V
OLED GND → 原MPU接口 pad2 / GND
OLED SCK（I²C SCL） → 原MPU接口 pad3 / PB2
OLED SDA → 原MPU接口 pad4 / PB3
```

禁止使用完全直通的四针排线，避免5V/GND反接。

## 5. 未变化资源

以下继续按 v1.2/v1.3/v1.4：

- KEY2：PA21，低有效、内部上拉、30 ms轮询消抖；PB21 DNC；
- 无线：PB25 CSN、PB17 MOSI、PA12 SCK、PB19 MISO、PB1 CE、PB16 IRQ；
- K230：PA8/PA9 UART1；
- CH340：PA10/PA11 UART0；
- D36A：PA26 ST1、PA24 DIR1、PB0 EN1；
- MS42CG：PA29 A、PA30 B、PA13 PWM、PB23 Z；
- DRV8870和左右轮编码器端点不变；
- PB27 DNC；蜂鸣器仍不配置、不连接。

## 6. 软件配置目标（尚未执行）

未来SysConfig目标：

```text
LINE_I2C:
  peripheral = I2C0
  SDA = PA28
  SCL = PA31

OLED_I2C:
  peripheral = I2C1
  SDA = PB3
  SCL = PB2

remove:
  MPU_I2C
  TCRT_OUT1..TCRT_OUT5 GPIO inputs
```

必须通过 `.syscfg` 修改和重新生成完成；禁止手工修改 `Debug/ti_msp_dl_config.c/.h`。

当前Claude Code工作副本仍是旧映射：OLED I2C0 PA0/PA1、MPU I2C1 PB2/PB3、TCRT GPIO。**当前固件与v1.5设计不一致，禁止按v1.5接线后直接运行旧固件。**

## 7. 电气门槛

1. LineFollower模块原理图显示SDA/SCL上拉到板载VDD33，但R1/R2阻值未知；PA28/PA31开发板没有PA0/PA1那组板载4.7k上拉。首次上电必须测SDA/SCL空闲电压和上升沿。
2. MPU移除后，PB2/PB3不再获得MPU模块的4.7k上拉。OLED资料显示SCL/SDA各有4.7k上拉，但实物GM009605V4.3仍需测量SCK/SDA空闲电压和上升沿，不能仅按资料默认。
3. U12 pin1的5V供电需要验证极性、空载/带载电压和至少85mA资料负载能力。
4. OLED照片已确认pin1在GND侧、pin4在SDA侧；仍需确认U12和原MPU/GY接口的实物Pin 1观察面，不能只依据编号制作线束。
5. 线束远离电机、DRV、D36A和MP1584开关节点，并有应力固定。

## 8. 冻结不等于实施

本轮未执行：

- `.syscfg`修改或SysConfig生成；
- 源码、OLED驱动、LineFollower驱动修改；
- 构建、烧录、串口或I2C扫描；
- 接线、上电、学习、200Hz调度或赛道测试。

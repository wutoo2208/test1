# MSPM0G3507 最新转接线与改线矩阵 v1.2

> **状态**：`FROZEN-DESIGN / USER-ACCEPTED / NOT WIRED`。
> **配套 Pin Plan**：`docs/pin-plan/mspm0g3507-pin-plan-frozen-v1.2.md`。
> **最新版规则**：所有模块接线只以本文件与 v1.2 冻结 Pin Plan 为准；旧 harness 和旧 Pin Plan 禁止用于施工。
> **边界**：冻结端点不代表已切线、接线、上电或测试。

## 1. 统一线束规则

1. 每个插头均需记录元件面、焊接面、插接面、卡扣和 Pin 1；不得只写“左到右”。
2. `DNC` 必须单独绝缘；不得与相邻针、电源、机架或屏蔽层接触。
3. 模块电源输出不得互相并联：DRV 3.3V/5V、D36A 5V、拓展板 5V/3.3V、MP1584 输出分别审查。
4. 逻辑线与电机/步进功率线分束；功率回流不得串过 MCU/编码器地线。
5. 当前未授权实际施工；本文件只定义未来线束端点。

## 2. H10：TCRT 与无线共用最新版

| H10 针脚 | MCU | 连接对象 | 模块端 |
|---:|---|---|---|
| 1 OUT1 | PB19/SPI0 POCI | 无线 | pin8 MISO |
| 2 OUT2 | PB17/SPI0 PICO | 无线 | pin5 MOSI |
| 3 OUT3 | PA16 GPIO | TCRT | OUT3 |
| 4 OUT4 | PA14 GPIO | TCRT | OUT4 |
| 5 OUT5 | PB20 GPIO | TCRT | OUT5 |
| 6 OUT6 | PB25/SPI0 CS0 | 无线 | pin3 CSN |
| 7 OUT7 | PA25 GPIO | TCRT | OUT1 |
| 8 OUT8 | PA27 GPIO | TCRT | OUT2 |
| 9 5V | — | TCRT 电源候选 | 电源/电平确认前不接 |
| 10 GND | — | TCRT 逻辑地候选 | 回流确认后接 |

### TCRT 模块线束

```text
TCRT OUT1 → H10 pin7
TCRT OUT2 → H10 pin8
TCRT OUT3 → H10 pin3
TCRT OUT4 → H10 pin4
TCRT OUT5 → H10 pin5
```

这是交叉线束，禁止使用旧 v1.0 的 OUT1→pin1、OUT2→pin2 直通方案。

## 3. 无线 SPI0 六根信号线

| 无线针脚 | 功能 | MCU 端 | 拓展板端 | 处理 |
|---:|---|---|---|---|
| 1 | UNKNOWN | — | — | DNC，绝缘，不推断 VCC/GND。 |
| 2 | UNKNOWN | — | — | DNC，绝缘，不推断 VCC/GND。 |
| 3 | CSN | PB25/SPI0 CS0 | H10 pin6 | CONNECT only after module identity closes。 |
| 4 | CE | PB1/GPIO | H1 pin6 | 默认低。 |
| 5 | MOSI | PB17/SPI0 PICO | H10 pin2 | CONNECT only after electrical review。 |
| 6 | SCK | PA12/SPI0 SCLK | H3 pin15 | **不得接 PB18/KEY3**。 |
| 7 | IRQ 低有效 | PB16/GPIO IRQ | H3 pin7 | 上拉/触发边沿待输出结构确认。 |
| 8 | MISO | PB19/SPI0 POCI | H10 pin1 | 模块未选中三态待确认。 |

无线 pin1/pin2 未确认前，不给模块供电，也不连接上述信号到实物模块。

## 4. MS42CG V2 最新六线

按手册编码器插座对接面：

```text
GND | Z | PWM | B | A | VCC
```

| MS42CG | MCU/拓展板 | 模式/处理 |
|---|---|---|
| VCC | 经批准的 3.3V | 只用 3.3V。 |
| A | H4 pin17 → PA29/TIMG6 CCP0 | Capture。 |
| B | H2 pin7 → PA30/TIMG6 CCP1 | Capture。 |
| PWM | H4 pin15 → PA13/TIMG0 CCP1 | 单输入双边沿 Capture 候选，待 SysConfig 验证。 |
| Z | H3 pin16 → PB23/GPIO IRQ | 固定零位事件。 |
| GND | 逻辑地 | 与功率回流分开。 |

旧 v1.0 的 PA12/PA13/PB26 A/B/PWM 线束全部作废；PB26 保持 DNC。

## 5. D36A 通道1

| D36A | MCU/拓展板 | 状态 |
|---|---|---|
| ST1 | H1 pin19 → PA26/TIMG7 CCP0 | STEP，上升沿有效。 |
| DIR1 | H1 pin18 → PA24/GPIO | 方向。 |
| EN1 | H2 pin6 → PB0/GPIO | 低休眠；要求安全偏置。 |
| GND | 独立逻辑参考 | 接线前确认星形地。 |
| 5V / ADC | — | DNC。 |
| ST2/DIR2/EN2 | — | DNC。 |

电机 A+/A-/B+/B- 必须由实际绕组资料或断电电阻测量确认，不在本版推断线色。

## 6. U2 改线和 DRV8870 控制

U2 不插 TB6612/DRV，只作四路逻辑汇接。

| 操作 | 最新结果 |
|---|---|
| 隔离 PB9→U2 pad14；桥接 pad16→pad14 | pad14=PB14/TIMA0 CCP0/AIN1 |
| 保留 PB12→U2 pad15 | pad15=PB12/GPIO/AIN2 |
| 隔离 PB7→U2 pad12；桥接 pad10→pad12 | pad12=PA7/TIMA0 CCP2/BIN1 |
| 隔离 PB6→U2 pad11；桥接 H1 pin17→pad11 | pad11=PB24/GPIO/BIN2 |

| U2 | DRV H3 | 功能 |
|---|---:|---|
| pad14/PB14 | 1 AIN1 | 右轮 PWM |
| pad15/PB12 | 3 AIN2 | 右轮第二输入 |
| pad12/PA7 | 2 BIN1 | 左轮 PWM |
| pad11/PB24 | 4 BIN2 | 左轮第二输入 |
| — | 5-8 EA/EB | DNC |

DRV VIN/GND/AOUT/BOUT 使用独立重载线，完全绕过 U2/U6/U16 功率针。

## 7. 两轮编码器

| 接口 | Pin | 功能 | MCU | 最新状态 |
|---|---:|---|---|---|
| U6 右轮 | 1 AO2 | 旧功率 | — | DNC |
| U6 | 2 GND | 编码器地 | GND | 条件接入 |
| U6 | 3 B1 | 右轮 B | PB11/TIMG8 CCP1 | CONNECT-DESIGN |
| U6 | 4 A1 | 右轮 A | PB10/TIMG8 CCP0 | CONNECT-DESIGN |
| U6 | 5 3.3V | 编码器电源 | 3.3V | 电源批准后接 |
| U6 | 6 AO1 | 旧功率 | — | DNC |
| U16 左轮 | 1 BO2 | 旧功率 | — | DNC |
| U16 | 2 GND | 编码器地 | GND | 条件接入 |
| U16 | 3 B2 | 左轮 B | PB5/TIMA1 CCP1 | CONNECT-DESIGN |
| U16 | 4 A2 | 左轮 A | PB4/TIMA1 CCP0 | CONNECT-DESIGN |
| U16 | 5 3.3V | 编码器电源 | 3.3V | 电源批准后接 |
| U16 | 6 BO1 | 旧功率 | — | DNC |

## 8. UART

### K230 / USART1

| USART1 | MCU | K230 |
|---:|---|---|
| pin1 PA9 | UART1 RX | K230 TX |
| pin2 PA8 | UART1 TX | K230 RX |
| pin3 GND | 共参考 | GND |
| pin4 5V | DNC | K230 独立稳定供电 |

### UART0 / CH340

PA10/PA11 只归板载 CH340。USART0 外接座 DNC。

### UART2

UART2 已取消完整预留：

```text
PB16 → 无线 IRQ
PB15 → DNC
```

禁止按旧 v1.0 把 PB15/PB16 当 UART2 对接线。

## 9. OLED、MPU6050、蜂鸣器

| 接口 | 最新 MCU/电源 | 条件 |
|---|---|---|
| U8 OLED GND/VCC/SCL/SDA | GND/待核电源/PA1/PA0 | 所有 I2C 上拉必须在 3.3V 域。 |
| MPU6050 VCC/GND/GY_SCL/GY_SDA | 待核电源/GND/PB2/PB3 | 地址、载板和上拉电压待确认。 |
| H13 GND/BEEP/3.3V | GND/PB27/3.3V | 仅低电流有源模块；无源方案 DNC。 |

## 10. 全局 DNC / 旧线束作废

- 无线 pin1/pin2；
- PB15、PB18/KEY3、PB26；
- U3、U12、H8；
- 开发板 PB21/PB22 KEY/LED 及未重新分配的拓展板 KEY/LED；
- U6/U16 pin1、pin6；
- U2 未在第 6 节列出的焊盘；
- DRV H3 pin5-8；
- D36A 第二通道、ADC、5V；
- USART0 外接座、USART1 5V；
- 任何 v1.0/v1.1/v1.2-candidate 中与本文件冲突的旧线束。

## 11. 未来施工前检查

在获得后续授权后，仍须先完成：

1. 所有观察面、Pin 1、线色和断电连续性表；
2. TCRT 电平/极性/车体顺序；
3. MS42CG 新 A/B/PWM 捕获的 SysConfig 隔离验证；
4. 无线 pin1/pin2、供电、IRQ 输出结构和 SPI 参数；
5. CE 默认低、DRV 输入双低、D36A EN 低、所有 Timer 无有效脉冲；
6. 电机和步进功率保持断开，先做纯逻辑域检查。

本文件未执行任何施工或测试。

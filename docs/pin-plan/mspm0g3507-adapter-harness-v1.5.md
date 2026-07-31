# MSPM0G3507 Adapter Harness v1.5 — U12 LineFollower + GY OLED

> **状态**：`FROZEN-DESIGN / USER-APPROVED / NOT WIRED`。  
> **配套Pin Plan**：`mspm0g3507-pin-plan-frozen-v1.5.md`。  
> **边界**：只冻结线束端点；未授权制作、接线、上电或修改SysConfig。

## 1. LineFollower经U12

### 1.1 两端针序

```text
LineFollower资料顺序：5V | GND | SDA | SCL
U12网络顺序：         5V | SCL | SDA | GND
```

### 1.2 唯一接线

| LineFollower | U12 | MCU/网络 |
|---|---:|---|
| 5V | pin1 | 5V |
| GND | pin4 | GND |
| SDA | pin3 | PA28 / I2C0 SDA |
| SCL | pin2 | PA31 / I2C0 SCL |

```text
模块pin1 → U12 pin1
模块pin2 → U12 pin4
模块pin3 → U12 pin3
模块pin4 → U12 pin2
```

禁止直通四芯线，必须按信号交叉。

## 2. OLED经原MPU6050接口

### 2.1 两端针序

```text
OLED实物顺序：      GND | VDD | SCK | SDA
资料对应名称：       GND | VCC | SCL | SDA
原MPU接口顺序：    5V | GND | SCL | SDA
```

### 2.2 唯一接线

| OLED | 原MPU接口 | MCU/网络 |
|---|---:|---|
| VDD（资料VCC） | pad1 | 5V |
| GND | pad2 | GND |
| SCK（I²C SCL） | pad3 | PB2 / I2C1 SCL |
| SDA | pad4 | PB3 / I2C1 SDA |

```text
OLED GND → GY pad2
OLED VDD → GY pad1
OLED SCK → GY pad3 / I2C1 SCL
OLED SDA → GY pad4
```

禁止将OLED原插头直通接入GY接口；前两根电源线需要按信号重排。

## 3. 退出使用的连接

- MPU6050模块完全移除并绝缘保存；
- OLED不再连接U8；U8 pad1～4保持DNC；
- 五路TCRT模块及其交叉线束退出；H10 pin3/4/5/7/8不接巡线模块；
- 不切H10、不桥接H10到GY；
- H10 pin1/2/6继续按无线v1.2设计，不能挪给巡线。

## 4. 断电连续性验收模板

### U12-LineFollower线束

| 检查 | 期望 | 实测 | 状态 |
|---|---|---|---|
| 模块5V ↔ U12 pin1 | 低阻 | — | NOT RUN |
| 模块GND ↔ U12 pin4 | 低阻 | — | NOT RUN |
| 模块SDA ↔ U12 pin3 | 低阻 | — | NOT RUN |
| 模块SCL ↔ U12 pin2 | 低阻 | — | NOT RUN |
| U12 pin2 ↔ PA31 | 低阻 | — | NOT RUN |
| U12 pin3 ↔ PA28 | 低阻 | — | NOT RUN |
| 5V/GND/SDA/SCL相互短路 | 不短路 | — | NOT RUN |

### GY-OLED线束

| 检查 | 期望 | 实测 | 状态 |
|---|---|---|---|
| OLED VDD ↔ GY pad1 | 低阻 | — | NOT RUN |
| OLED GND ↔ GY pad2 | 低阻 | — | NOT RUN |
| OLED SCK ↔ GY pad3/PB2 I2C1 SCL | 低阻 | — | NOT RUN |
| OLED SDA ↔ GY pad4/PB3 | 低阻 | — | NOT RUN |
| OLED VDD ↔ GND | 不短路 | — | NOT RUN |
| SDA ↔ SCL | 不短路 | — | NOT RUN |

## 5. 首次上电前门槛

1. OLED照片已确认其pin1=GND、pin4=SDA；仍需提供U12、原MPU/GY接口和LineFollower插头近照，确认各接口Pin 1观察面。
2. 先完成SysConfig v1.5生成、构建和静态输出检查，再接线。
3. 电机、DRV功率、D36A和执行器保持断开。
4. 分别单独给LineFollower和OLED上电，测量LineFollower SDA/SCL与OLED SDA/SCK空闲电压；任何接近5V、长期0V或异常发热立即断电。
5. 分别确认I2C0地址0x5C和I2C1 OLED地址0x3C/0x3D，再组合系统。
6. 只有逻辑域通过后，才另行申请传感器学习和200Hz调度验证。

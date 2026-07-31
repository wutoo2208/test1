# HiWonder/AiBlock LineFollower_6CH V1.0 - 六路 I²C 巡线模块事实卡

> **状态**：`FROZEN-DESIGN / USER-APPROVED / NOT SYSCONFIG-APPLIED / NOT WIRED`。  
> **详细来源**：[六路巡线资料复核](../../reviewed/linefollower-6ch-pdf-review.md)。  
> **替代关系**：作为主循迹传感器替换原五路 TCRT5000；旧 TCRT 文档保留为历史证据，不再作为新固件/接线基线。

## 核心参数

| 项目 | 结论 |
|---|---|
| 探头 | 6 路红外反射探头，板载 MCU 处理 |
| 供电 | DC 5 V |
| 资料工作电流 | 85 mA |
| 接口 | 4 线 I²C：5V、GND、SDA、SCL |
| 地址 | 7 位固定 `0x5C` |
| 逻辑电平 | 原理图显示 SDA/SCL 上拉到板载 3.3 V；实物待测 |
| 数据 | 6 路数字状态 + 6 路 16 位模拟值 + 阈值候选 |
| 高度资料范围 | 0.5–8 cm；学习和运行高度必须一致 |
| 不带壳尺寸 | 89 × 18.8 × 14.5 mm；孔距 83 mm；M3 候选 |
| 带壳尺寸 | 87.6 × 23.5 × 13.2 mm（独立尺寸图）；M4 候选 |

## 推荐读取合同

```text
I2C address: 0x5C (7-bit)
register 0x05: 1 byte digital state, bit0..bit5 = channel1..channel6
register 0x06: 12 bytes analog values, six uint16 little-endian
threshold registers: BLOCKED BY DOCUMENT CONFLICT
```

数字有效极性、通道左右顺序、I²C 速率和失效值均未冻结。

## frozen v1.5 接口

```text
I2C0:
  SDA = PA28
  SCL = PA31
  physical = U12

LineFollower 5V  → U12 pin1
LineFollower GND → U12 pin4
LineFollower SDA → U12 pin3 / PA28
LineFollower SCL → U12 pin2 / PA31
```

- LineFollower 独占 I2C0，不与 OLED 或 MPU6050 共享。
- 模块端顺序 `5V/GND/SDA/SCL` 与 U12 网络顺序 `5V/SCL/SDA/GND` 不同，必须使用按信号交叉的线束，禁止四针直通。
- H10/GY共享方案和H10切线/桥接候选已否决；历史 candidate 仅供追溯。
- 权威设计：`docs/pin-plan/mspm0g3507-pin-plan-frozen-v1.5.md`；权威线束：`docs/pin-plan/mspm0g3507-adapter-harness-v1.5.md`。

## 软件与控制边界

- 主循迹优先使用六路模拟值形成归一化线位置；数字状态用于检测/降级候选，具体算法待实测。
- 200 Hz 是控制调度候选，不是已证明的传感器更新率；必须测量单次事务时间、总线占用和最坏超时。
- 黑白学习状态、阈值保存、bit 极性和异常帧必须在台架上确认。
- 传感器只负责赛道红外循迹，不测钢球位置；K230 仍是钢球位置链路。

## 禁止动作

frozen v1.5 只确认设计。当前未授权修改 SysConfig/源码、制作线束、接线、上电、I²C扫描、构建、烧录或赛道测试。

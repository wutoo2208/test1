# K230单向UART协议v1交接

- 日期：2026-07-31
- 状态：`USER-APPROVED DESIGN / NOT IMPLEMENTED / NOT TESTED`
- 本轮范围：只整理文档；未修改Claude Code正在处理的SysConfig、源码、生成文件或Debug输出。
- 协议权威文件：`docs/protocols/k230-ball-position-uart-v1.md`
- 当前接线设计：K230 IO9/TXD（3.3 V TTL）→USART1 pin1/PA9/UART1_RX；GND共地；PA8保留但DNC；USART1 pin4/5V保持DNC，K230使用独立稳定5 V。
- UART：115200、8N1、单向接收。
- 帧：`55 AA STATUS SEQ POSL POSH VELL VELH CONF XL XH YL YH CRC8`，固定14字节；CRC-8/ATM覆盖前13字节。
- 保护：STATUS 0禁用坐标；STATUS 2仅作降权候选；最后有效帧超100 ms进入丢球保护；CRC/帧头错误丢弃；SEQ停滞判卡帧。
- 尚未冻结：STATUS 2具体降权、SEQ卡帧阈值、字段合理范围、控制增益/限幅、正方向与坐标标定。
- 重要限制：协议v1无K230采集时间戳，帧率、端到端延迟和抖动必须实测。
- NOT RUN：SysConfig、源码实现、构建、烧录、串口监听、逻辑分析仪、上电和运动测试。

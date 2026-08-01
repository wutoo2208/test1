from pathlib import Path
p=Path(r'tmp/design-report/build_report.py')
s=p.read_text(encoding='utf-8')
s=s.replace('针对轮式小车循线行驶过程中钢球在凹槽摆杆上的位置稳定问题，设计了一种以MSPM0G3507为核心的分层双闭环控制系统。','针对循线小车运动时钢球在凹槽摆杆上的位置稳定问题，设计了一种以MSPM0G3507为核心的分层双闭环控制系统。')
s=s.replace("story += [PT('2、电源与安全设计',h2),PT('逻辑域、视觉模块和功率执行器分域供电并共地；车轮与步进功率不由开发板3.3 V供电。电机支路设置保险和物理断能。上电默认PWM为0、D36A失能，控制超期、丢线、编码器停滞、过流或棕断均进入STOP。',body),PT('K230控制链与场外视频展示链解耦；I²C0仅服务循迹模块，I²C1仅服务OLED，以减少共享故障。',body),PageBreak()]", "story += [PT('2、电源与安全设计',h2),PT('逻辑、视觉和功率执行器分域供电并共地；车轮与步进功率采用独立电源，不由开发板3.3V供电。电机支路设置保险和物理断能。上电默认PWM为0、D36A失能；故障均进入STOP，包括控制超期、丢线、编码器停滞、过流和棕断。',body),PT('K230控制链与场外视频展示链解耦；I2C0仅服务循迹模块，I2C1仅服务OLED，以减少共享故障。',body),PageBreak()]")
p.write_text(s,encoding='utf-8')

from pathlib import Path
p=Path(r'tmp/design-report/build_report.py')
lines=p.read_text(encoding='utf-8').splitlines()
out=[]
for line in lines:
    if line.startswith("body=ParagraphStyle('CNBody'"):
        line="body=ParagraphStyle('CNBody',fontName='SimSun',fontSize=11.3,leading=17.2,alignment=TA_LEFT,firstLineIndent=22,spaceAfter=1)"
    if line.startswith("center=ParagraphStyle('CenterCN'"):
        out.append(line)
        out.append("equation=ParagraphStyle('EquationCN',fontName='SimSun',fontSize=11.0,leading=16,alignment=TA_CENTER,spaceBefore=1,spaceAfter=1)")
        continue
    if line.startswith("story += [PT('二、系统理论分析与计算'"):
        line="story += [PT('二、系统理论分析与计算',h1),PT('1、小车循迹控制理论',h2),PT('设左右轮线速度为vL、vR，轮距为B，则车体速度关系为：',body),PT('v = (vR + vL) / 2，ω = (vR - vL) / B',equation),PT('六路传感器位置权值为{-1.0，-0.6，-0.2，0.2，0.6，1.0}。各通道按白底/黑线标定值归一化后，以加权质心计算循迹误差：',body),PT('eL = Σ(pi × si) / Σsi - e0',equation),PT('循迹控制采用PID并叠加弯道限速。直线段比较10ms编码器增量；左轮A相1倍频乘4后与右轮QEI同量纲比较。速度PI仅在软启动完成、转向量较小且两轮有新脉冲时更新，缺脉冲或故障清积分。',body)]"
    if line.startswith("story += [PT('2、球—杆系统控制理论'"):
        line="story += [PT('2、球—杆系统控制理论',h2),PT('将钢球视为无滑动滚动的实心球，x为相对摆杆中心的位置，θ为摆杆小角度，at为车辆沿杆方向的等效扰动。忽略高阶项时：',body),PT('d²x / dt² ≈ (5/7) × (gθ - at)',equation),PT('该对象近似为二重积分环节。外环依据K230球位、速度和置信度计算期望杆角θd，采用PD为主、弱积分消除静差，并对杆角、角速度和积分限幅；内环依据MS42CG计数及连杆标定表控制D36A。',body),RLImage(str(FIG_CTRL),width=15.3*cm,height=6.6*cm),PT('图2-1  循迹与滚球双闭环控制结构',center),PT('当视觉帧超过100ms未更新、CRC错误、置信度不足或帧号停滞时，旧坐标立即失效；控制器清积分、摆杆回安全角并禁止继续使用预测位置。',body),PageBreak()]"
    out.append(line)
p.write_text('\n'.join(out)+'\n',encoding='utf-8')

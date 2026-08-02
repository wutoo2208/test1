# 交接：REQ-002 第二问最终提交固件冻结

日期：2026-08-01  
会话任务：将用户现场确认效果良好的循迹第二问固件冻结为最终提交版本。  
任务状态：完成  
授权等级：文档冻结；不执行新的构建、烧录或测试

## 最终版本身份
- 用途：`REQ-002` 单圈顺时针循迹与返回 A 点停车（第二问）。
- 文件：`MotorSelfTest/test1_motor_selftest.out`
- 大小：`630476 bytes`
- SHA-256：`BDE6EFE861A40E0AA3DACCFC68093E32AB54C026FA8DA3B825545ECBB15FEBAA`
- 当前工作区重新核对：哈希匹配。
- 用户最终决定：循迹第二问使用本版本，不再继续调整参数。

## 烧录证据
- 交接证据：`docs/handoffs/2026-08-01-req002-right-wheel-brake-flash-complete.md`。
- Horco CMSIS-DAP UID `da941ca0`，MSPM0G3507，指定 TI Pack，SWD `100 kHz`，sector erase，ELF，`--no-reset`。
- pyOCD 退出码 `0`；擦除 `72704 bytes (71 sectors)`，编程 `72704 bytes (71 pages)`。
- 烧录工具证据只证明写入成功；用户后续现场陈述该版本循迹效果很好，并明确选为最终作品版本。

## 最终循迹参数
- 直线：基础 `850` permille，右侧固定 trim `420` permille；直线轮速 PI `Kp=4.0`、`Ki=12.0`、输出限幅 `50` permille，左编码器 1x 按 `4.0` 归一化到右 QEI 4x，并使用左速目标比例 `1.20`。
- `RIGHT_CURVE_APPROACH`：触发误差 `0.06`、确认 `10 ms`、最长 `150 ms`；左 `820`、右 `420` permille。
- `RIGHT_CURVE_ARC`：触发误差 `0.10`、确认 `5 ms`；左 `800`、右 `0`（0/0 滑行）；最短 `300 ms`、最长 `700 ms`；回中误差 `<=0.04` 连续 `60 ms`。
- 急弯右轮单独制动：误差 `>=0.15` 且右转方向连续 `10 ms`，每个锁存右弯最多一次；左轮 `800`，右轮先准备 `1 ms`，随后 DRV8870 A 通道 `1/1` 电气制动 `100 ms`，再释放回 ARC `800/0`。
- `RIGHT_CURVE_RECOVER`：`200 ms`，左 `800→850`、右 `450→430`；再次明显右偏时返回 ARC。
- 所有右弯阶段关闭轮速 PI；左向恢复转向力度为 `400`，仅作弱纠偏。
- ARC/急弯期间短时丢线保持右向有界处理，持续异常进入安全故障锁止。

## 终点与显示
- 返回标记首次识别后，两轮 IN1 先连续高 `1 ms`，随后两路 DRV8870 进入 `1/1` 电气制动；标记确认窗口为 `50 ms`。
- 确认完成后释放为 `0/0` 滑行并保持 `COMPLETE` 软件锁；人工中止或普通故障不保证执行终点制动路径。
- OLED 固件模块包含 `READY/RUNNING/COMPLETE/FAULT` 状态显示；运动过程中不持续刷新，终态时更新。

## 代码范围
- `app/req002.c`
- `config/firmware_config.h`
- `drivers/motor_driver.c`
- `drivers/motor_driver.h`
- `firmware_tests/test_firmware.py`
- 构建产物：`MotorSelfTest/test1_motor_selftest.out` 及同目录生成文件。

## 验证与局限
- 静态单元测试：11/11 PASS。
- CCS gmake `MotorSelfTest`：PASS；SysConfig 生成文件报告 Unchanged。
- 烧录：PASS（pyOCD 工具日志）。
- 运动效果：用户现场陈述“效果很好”，并选为最终第二问版本。
- 未在项目中取得正式计时 `<=20 s`、停车偏差 `<=2 cm` 的独立测量记录；不得把用户现场效果陈述扩写成裁判正式验收。

## 冻结规则
- 最终提交前不得再修改上述源码、配置或构建产物。
- 不得重新构建后仍沿用本 SHA-256；任何重新构建都必须视为新版本并重新核对。
- 若需恢复/复烧，只允许使用大小 `630476 bytes`、SHA-256 为 `BDE6...FEBAA` 的该 ELF，并保持原烧录参数。

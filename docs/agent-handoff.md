# Claude / Codex 项目交接入口

> 初始化日期：2026-07-28  
> 新会话先读本文件，再领取一个未阻塞的 `TASK-*`。任何候选、静态结论都不得冒充最终接线或硬件实测。

## 1. 30 秒启动摘要

- 项目根目录：`C:\Users\Administrator\workspace_ccstheia\test1`。
- 当前是 MSPM0G3507 CCS Theia DriverLib/SysConfig 空工程。
- 当前应用源码只完成 `SYSCFG_DL_init()`，随后周期延时并翻转 PB22 LED。
- 已有天猛星 MSPM0G3507 引脚图、原理图、提取资料和人工审阅硬件笔记。
- 当前没有正式题面、评分点、确认 BOM、外接模块最终接线或控制算法。
- 当前限制：不得修改 `.syscfg`、源码、CCS 工程和生成文件；不得构建、烧录、调试、探针、串口或连接硬件。
- 下一项可执行工作：由人类提供 `TASK-001` 的正式题面/评分规则，以及 `TASK-002` 的实际 BOM/精确型号。

## 2. 强制阅读顺序

1. `docs/agent-handoff.md`：当前状态、权限、下一步。
2. `docs/task-board.md`：任务、依赖、Owner 和阻塞。
3. `docs/requirements.md`：正式要求、约束和未知项。
4. `docs/decisions.md`：已接受决策和证据规则。
5. `docs/hardware-interface.md`：板级事实、候选接口与最终接线。
6. `docs/architecture.md`：当前固件基线和架构门槛。
7. `docs/test-log.md`：什么真的被验证过。
8. 需要硬件细节时再读：
   - `docs/hardware/README.md`
   - 对应的 `docs/hardware/*.md`
   - `docs/reviewed/tianmengxing-mspm0g3507-source-facts.md`
   - 必要时回查 `docs/source-pdf/*.pdf`

## 3. 文档职责映射

| 问题 | 唯一主文档 |
|---|---|
| 比赛必须完成什么 | `docs/requirements.md` |
| 为什么选择某方案 | `docs/decisions.md` |
| 模块具体接到哪里 | `docs/hardware-interface.md` |
| 软件和控制如何组织 | `docs/architecture.md` |
| 当前下一步做什么 | `docs/task-board.md` |
| 什么被验证或实测过 | `docs/test-log.md` |
| 上一会话做了什么 | `docs/agent-handoff.md` |

若历史交接摘要和主文档冲突，以主文档中来源更高、证据更完整且未被 supersede 的记录为准。

## 4. 当前已确认基线

| 状态 | 事实 | 证据 |
|---|---|---|
| `[已确认|静态核验]` | 工程目标为 MSPM0G3507，当前应用为 PB22 LED 翻转空工程。 | `empty.syscfg`、`empty.c` |
| `[已确认|资料复核]` | PB22 是板载 LED，PB21 是板载按键。 | 原理图 p1；`docs/reviewed/tianmengxing-mspm0g3507-source-facts.md` |
| `[资料复核 + 用户当前状态]` | PA19/PA20 用于 SWD；原理图记录 PA10/PA11 到板载 CH340E 网络；**用户确认当前 UART0 未接任何外设/线束，不能视为已连接或可用 COM 端口。** | 原理图 p3；用户现场陈述；已复核摘要 |
| `[已确认|资料复核]` | 原始 PDF 是硬件事实源，提取稿仅供检索。 | `docs/PDF_WORKFLOW.md`、`docs/hardware/README.md` |

以上不证明板卡当前连接、固件已烧录或 LED 已在实物上闪烁。

## 5. 当前未知与阻塞

- `UNK-001`–`UNK-003`：正式题面、评分规则和量化边界。
- `UNK-004`–`UNK-006`：实际 BOM、模块参数和最终接线。
- `UNK-007`：控制对象、周期、算法和调参方法。
- `UNK-008`：失效安全和急停要求。
- `TASK-003`–`TASK-008` 因前置资料、决策或授权缺失而阻塞。

## 6. 当前允许与禁止动作

### 允许

- 读取现有文件和新提供的比赛/硬件资料。
- 对正式资料提取原文、页码、需求和未知项。
- 在用户明确授权时，向项目管理文档追加可追溯记录。
- 进行不触发工程工具或硬件连接的静态一致性分析。

### 禁止

- 修改已有文档、`.syscfg`、源码、CCS 工程、目标配置和生成文件。
- 运行 SysConfig、构建、编译、链接、烧录或调试。
- 检测探针、扫描串口、接线、上电或控制执行器。
- 猜测模块型号、电平、地址、频率、引脚、控制周期或算法。
- 把 `候选` 写成 `已确认`，或把静态分析写成实测。

## 7. 下一项可执行工作

1. **`TASK-001`**：向负责人索取正式题面 PDF、文件版本、发布日期和评分细则。
2. **`TASK-002`**：向负责人索取实际硬件清单，逐项记录模块精确型号、数量、实物丝印和 datasheet。
3. 输入到位后，不先设计；先按 `docs/requirements.md` 和 `docs/hardware-interface.md` 的模板登记来源与未知项。

## 8. 交接纪律

每个 Claude/Codex 会话必须：

1. 读取最新 `HO-*` 和任务看板。
2. 只领取一个未阻塞、无人占用的任务。
3. 核对任务依赖、用户授权和禁止动作。
4. 每个结论附状态、证据路径和稳定 ID。
5. 正式题面原文与设计解释分开记录。
6. 候选方案不得进入最终接线或已接受架构。
7. 静态核验不得描述为构建或实测。
8. 完成后追加交接记录，明确执行与未执行事项、阻塞和精确下一步。

## 9. 会话交接记录

### HO-20260728-01 — 文档体系初始化

- **Timestamp**：2026-07-28
- **Agent**：Claude
- **Task ID**：`TASK-000`
- **Goal**：按指定结构补齐 7 个项目管理文档。
- **Files read**：现有 `docs/PDF_WORKFLOW.md`、`docs/hardware/**`、`docs/reviewed/**`，以及工程静态配置/源码信息。
- **Inputs received**：仅增添、不修改已有内容；不修改 `.syscfg`、CCS 工程或生成文件；不构建、不烧录。
- **Actions performed**：创建 `requirements.md`、`architecture.md`、`hardware-interface.md`、`task-board.md`、`test-log.md`、`agent-handoff.md`、`decisions.md`。
- **Files changed**：仅上述 7 个新文件；既有文件不在变更范围。
- **Decisions added**：`DEC-001`–`DEC-006`。
- **Validation performed**：仅文件范围、内容边界和静态一致性验证。
- **What was not run**：SysConfig、构建、烧录、调试、探针、串口、硬件连接和实测均未运行。
- **Current blockers**：正式题面、评分点、实际 BOM、模块 datasheet、最终接线和控制目标缺失。
- **Exact next action**：取得 `TASK-001` 与 `TASK-002` 所需原始资料，并先登记来源和版本。
- **Warnings**：当前没有已确认外设接线，也没有任何硬件实测结论。
- **Confidence and evidence**：项目静态基线可由本项目文件复查；硬件事实需按原始 PDF 页码复查。

## 10. 新交接记录模板

```text
Handoff ID: HO-YYYYMMDD-NN
Timestamp:
Agent: Claude / Codex / Human
Task ID:
Goal:
Files read:
Inputs received:
Actions performed:
Files created/changed:
Decisions added:
Validation performed:
What was not run:
Current blockers:
Exact next action:
Warnings / prohibited actions:
Confidence and evidence:
```

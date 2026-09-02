# Refute 审查记录 — ams_system_dialog「安全告警」提交

> 对 orchestrator 调度的 security-scanner + logic-scanner 原始发现执行三层对抗性验证。规则见 [`orchestrator/refute-rules.md`](skills/codecheck/orchestrator/refute-rules.md)。
> 原则：只质疑，不发现；质疑必须有代码证据；P0 推翻需「确认不可达」级证据。

## 审查概要

| 指标 | 数值 |
|------|------|
| 审查范围 | security: 2 条候选, logic: 2 条候选 |
| P0/P1 全审 | 1 条 |
| P2 抽样审 | 3 条 |
| 维持 | 2 条 |
| 降级 | 1 条（SEC-01: P0→P1） |
| 推翻 | 0 条 |
| 合并 | 1 组 → 1 条（SEC-02 + LOG-01） |
| **进入最终报告** | **3 条**（P0: 0, P1: 1, P2: 2, P3: 0） |

---

## 逐条审查

### ⬇️ 降级 — SEC-01: P0 → P1

- **发现摘要**：`IntentDetailPage.ets:170` 以 `hilog.info(DOMAIN, TAG, 'getBundleInfo success: bundleName=%{public}s', bundleInfo.name)` 打印已安装应用包名。
- **scanner 原始等级**：security-scanner 内部 9-10 档（P0），依据 AUTH-002「日志红线关键字」，典型后果敏感信息泄漏 P0。
- **审查结论**：降级为 P1。
- **降级理由（第二层·影响夸大证伪）**：
  1. `bundleInfo.name` 属「已安装应用包名」红线关键字，但非凭据类（非 token/password/udid/challenge/SN），不触发「凭据视同已泄露需轮换」。
  2. 攻击前提为持有 hilog 读取权限（开发者/日志采集），非任意进程可达。
  3. 该行为为系统对话框面向用户自身的已装应用，泄漏面为「设备已装应用清单」，低敏感。
- **未推翻理由**：提交将该行由模板字面量改为格式串时显式选择 `%{public}s`，确属错失脱敏，应改 `%{private}s`；问题真实存在，不可推翻，仅下调严重度。维持进入报告（P1，must_fix）。
- **同类横扫（不另立条目，并入 SEC-01 同类清单）**：`IntentDetailPage.ets:164/173/201/322`、`IntentContent.ets:163/165` 均以模板字面量内插应用标识符（默认 public），为提交前既有写法，列入 SEC-01 同类清单并在建议中要求一并 privatize。

---

### 🔀 合并 — SEC-02 + LOG-01 → 1 条（SEC-02/LOG-01, P2）

- **SEC-02 摘要**（security）：`IntentContent.ets` 的 `isCompActive` 销毁守卫仅覆盖 `getInfos` 的 111/136/152 三处，未覆盖 130/140 等 await 恢复点 → G6 点位封堵，组件销毁后仍改写 @State。
- **LOG-01 摘要**（logic）：同一函数 `getInfos` 内 await 恢复点缺 `isCompActive` 复检，存在 use-after-destroy 改写 @State 与在销毁 `this` 上调用 `getUIContext()` 的逻辑窗口。
- **审查结论**：合并为一条，等级 P2。
- **合并理由（第三层·同一修复改动）**：两 scanner 命中同一函数同一根因（`isCompActive` 守卫点不完整），修复方案为同一处改动（在每个 await 恢复点统一加复检 / 引入可取消令牌机制）。属「同一修复改动」合并。
- **降级/维持判定**：
  - 候选原等级 P1（use-after-destroy 字面含 UAF 嫌疑）。经第二层证伪：ArkUI 框架对组件销毁后 @State 更新通常记警告而非崩溃；`setAppName:181` 的 `getUIContext()` 销毁后返回 undefined 触发的 TypeError 被 `getInfos` per-bundle try-catch（129/146）兜住，不构成崩溃/数据损坏。 PixelMap 释放路径（`trackPixelMap` 的 `!isCompActive` 分支 + `releasePixelMaps`）已覆盖迟到 PixelMap，无 double-free/UAF。
  - 故从候选 P1 降为 P2，维持进入报告（followups）。

---

### ✅ 维持 — LOG-02: P2

- **发现摘要**：`IntentDetailPage.ets:143-148` 的 `onBackPress` 无 try-catch，与 `Index.ets:29-38`、`IntentContent.ets:96-105` 不一致；本提交为三处统一加 null guard 但独留 IntentDetailPage 缺异常包裹。
- **审查结论**：维持 P2。
- **理由**：第一层证伪未发现上游硬 guard——`AppStorage.get('context')` 可返回 undefined（已由本提交 null guard 覆盖），但 `context.terminateSelf()` 在有效 context 上抛出的路径无任何上游防护；兄弟页面均有 try-catch，结构不一致为事实。本提交已触碰该函数却未补齐异常处理，不推翻。terminateSelf 在有效 UIAbilityContext 上抛出概率低 → 维持 P2（不升 P1）。

---

## refute 耗时与统计

- 审查耗时：~若干轮调用链追踪 + 代码片段二次验证。
- 调用链追踪：3 次（`getUIExtensionType`→`getBundleInfo`；`getInfos`→`getApplicationInfo`/`setAppName`；`onBackPress`→`AppStorage.get`/`terminateSelf`）。
- 代码片段二次验证：4 次（`trackPixelMap`/`releasePixelMaps` 交错；`uint8ArrayToImage` finally 释放 ImageSource；同类横扫 6 处 hilog；三处 onBackPress 异常包裹对比）。
- 被推翻发现：0 条（无条目保留在本日志的「推翻」区段）。

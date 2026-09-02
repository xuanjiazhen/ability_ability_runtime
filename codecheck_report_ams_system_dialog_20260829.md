# 代码检视报告 — ams_system_dialog「安全告警」提交（Round 1 / 最新提交）

> 统一报告由 codecheck 工作台生成，**用于门禁管控**。所有 codecheck 报告（含 orchestrator 合并出的统一报告、单 scanner 直接产出的统一报告）必须遵循本模板：章节顺序、字段名、报告元数据块、评分与门禁规则均为**固定格式**，跨报告保持一致，便于门禁脚本解析与历史对比。
> 生成入口：[`README.md`](skills/codecheck/README.md) → Step 5；合并逻辑见 [`orchestrator/SKILL.md`](skills/codecheck/orchestrator/SKILL.md)。

---

## 报告元数据

> **门禁脚本只读取本 YAML 块**。字段名与取值域为固定合约，禁止改名、增删或自定义取值。人工阅读部分从「1. 门禁结论」开始。

<!-- codecheck-report-metadata:start -->
```yaml
codecheck_report:
  schema_version: "1.0"
  scope: "ams_system_dialog (安全告警提交)"
  round: 1
  commit_id: "536dc987990676f416ee04cc0f58d4ef5d58c877"
  change_id: "I452d9123705cd1b56d15612a4a53a7305366af06"
  report_id: "I452d9123705cd1b56d15612a4a53a7305366af06-R1"
  date: "2026-08-29"
  gate_decision: "conditional"
  risk_level: "medium"
  score: 78
  dimensions_required: ["security-scanner", "logic-scanner"]
  dimensions_executed: ["security-scanner", "logic-scanner"]
  findings_total: 3
  findings_by_severity: {P0: 0, P1: 1, P2: 2, P3: 0}
  gate_blockers: []
  must_fix: ["SEC-01"]
  followups: ["SEC-02/LOG-01", "LOG-02"]
```
<!-- codecheck-report-metadata:end -->


---

## 1. 门禁结论

| 项目 | 结论 |
|---|---|
| 决策 | **conditional** |
| 风险等级 | 🟠 medium |
| 评分 | **78/100** |
| 阻塞项 | 无 |
| 必须修复（P0/P1） | 1 项 |
| 建议跟进（P2/P3） | 2 项 |

**一句话结论**：本提交（空指针保护 + PixelMap 生命周期 + 异步销毁守卫 + hilog 重构）方向正确且修复了既有崩溃与泄漏，但未做机制性根治——`isCompActive` 守卫未覆盖全部 await 恢复点（G6 点位封堵），且对红线标识符 bundleName 显式使用 `%{public}s`（AUTH-002），需修复 P1 后放行。

---

## 3. 必须立即处理（P0/P1）

| ID | 优先级 | Scanner | 问题 | file:line | 触发路径 | 影响 |
|---|---|---|---|---|---|---|
| SEC-01 | P1 | security | 红线标识符 bundleName 以 `%{public}s` 打印 | `services/dialog_ui/ams_system_dialog/feature/src/main/ets/view/IntentDetailPage.ets:170` | 任意触发 `getUIExtensionType`（用户进入意图详情页），`bundleManager.getBundleInfo` 成功后执行 `hilog.info(... 'getBundleInfo success: bundleName=%{public}s', bundleInfo.name)`，将已安装应用包名以 public 写入 hilog | 持有 hilog 读取权限者可获取设备已安装应用清单（隐私泄漏）；同类横扫另见 6 处模板字面量（默认 public）同写法 |

> 无 P0 项。P1 项详情见第 6 节。

---

## 4. 建议本轮或下一补档处理（P2/P3）

| ID | 优先级 | 问题 | 建议行动 | 排期 |
|---|---|---|---|---|
| SEC-02/LOG-01 | P2 | 异步销毁守卫为点位封堵：`isCompActive` 仅在 `getInfos` 的 3 处检查（111/136/152），未覆盖 `await bundle.getApplicationInfo`（130）与 `await this.setAppName`（140）等恢复点，组件销毁后仍可能改写 `this.intermediateGroupMaterial`（@State）并在销毁后的 `this` 上调用 `getUIContext()` | 将每个 await 恢复点统一包裹 `if (!this.isCompActive) return;`；或把全部异步副作用收敛到一个可在 `aboutToDisappear` 中取消的 `AbortSignal`/令牌机制，实现机制性根治 | 本轮或下一补档 |
| LOG-02 | P2 | `IntentDetailPage.onBackPress`（143-148）缺 try-catch，与同模块 `Index.ets:29-38`、`IntentContent.ets:96-105` 不一致；本提交为三处统一加了 null guard，但 IntentDetailPage 仍无异常包裹，`context.terminateSelf()` 在有效 context 上抛出时将逃逸事件回调 | 为 `IntentDetailPage.onBackPress` 补 try-catch + hilog.error，与兄弟页面保持一致 | 本轮或下一补档 |

---

## 5. 分维度速览

| 维度 | 结果 | 关键说明 |
|---|---|---|
| security-scanner | 通过（含 1 P1 / 1 P2） | AUTH-002：bundleName 显式 `%{public}s`（P1）；G6 点位封堵：PixelMap/异步守卫机制正确但不完整（P2，与 logic 合并）。空指针保护（MEM-001）三处均已修复，无残留 P0；PixelMap 由 `createdPixelMaps`+`trackPixelMap`+`releasePixelMaps` 全链路追踪，迟到 PixelMap 经 `!isCompActive` 分支释放，未发现 double-free/UAF。 |
| logic-scanner | 通过（含 1 P2） | LOG-009/CONC：`getInfos` 内 await 恢复点销毁守卫不完整（P2，与 security 合并）；LOG-009：`onBackPress` 异常处理三页不一致（P2）。状态机/控制流/边界其余项不适用或无发现。 |

> 路径特征探测：目标为 `services/dialog_ui/.../feature/src/main/ets/` 下 ArkTS UI 代码，无 IPC Stub/Parcel、无 DB/文件持久化、无 `interfaces/kits` `.d.ts` SDK 信号 → 按通用路径组合 security+logic 执行，input/api 维度 N/A（不计为缺失）。

---

## 6. 关键发现详情

### [SEC-01] 红线标识符 bundleName 以 %{public}s 打印 (P1, scanner=security-scanner)

- **位置**：`services/dialog_ui/ams_system_dialog/feature/src/main/ets/view/IntentDetailPage.ets:170`
- **触发路径**：用户进入意图详情页 → `getUIExtensionType(bundleName, moduleName, abilityName)` → `bundleManager.getBundleInfo(...)` 成功 → `hilog.info(DOMAIN, TAG, 'getBundleInfo success: bundleName=%{public}s', bundleInfo.name)`。`bundleInfo.name` 为系统返回的规范包名（已安装应用标识），经 `%{public}s` 标记后以明文写入 hilog 缓冲区。
- **影响**：持有 `hilog` 读取权限的进程/开发者可获取设备已安装应用清单，构成隐私泄漏。本提交将该行由模板字面量改为格式串时，对红线关键字显式选择 `%{public}s`，错失脱敏机会（本应 `%{private}s`）。
- **证据**：`patterns/privilege-auth.md` AUTH-002「日志红线关键字」明确将「已安装应用包名」列为必查项，典型后果敏感信息泄漏。本行 `%{public}s` 与 `bundleInfo.name` 参数为提交新增（见 commit diff hunk `IntentDetailPage.ets:170`）。同类横扫（security-scanner Step 3 必做）全模块命中 6 处同写法：`IntentDetailPage.ets:164/173/201/322`、`IntentContent.ets:163/165` 均以模板字面量内插 bundleName/appName/intentName/moduleName（默认 public）。
- **建议**：将本行改为 `hilog.info(DOMAIN, TAG, 'getBundleInfo success: bundleName=%{private}s', bundleInfo.name)`；同类 6 处模板字面量统一改为 `%{private}s` 占位符 + 参数形式。`bundleName`/`appName`/`moduleName`/`intentName` 等已安装应用标识符一律 private。
- **Refute**：AUTH-002 名义 P0，经第二层「影响夸大证伪」降级为 P1——bundleName 非凭据类（非 token/password/udid/challenge），属低敏感标识符，且攻击需 hilog 读取权限；不是“凭据视同已泄露需轮换”情形。维持 P1，不推翻。

---

### [SEC-02/LOG-01] 异步销毁守卫点位封堵：isCompActive 未覆盖全部 await 恢复点 (P2, scanner=security+logic)

- **位置**：`services/dialog_ui/ams_system_dialog/feature/src/main/ets/view/IntentContent.ets:130` 与 `:140`（守卫仅见于 `:111` / `:136` / `:152`）
- **触发路径**：`aboutToAppear` → `getInfos()` 启动；`bundleNames.map(async ...)` 对每个 bundle 并发执行：`await bundle.getApplicationInfo`（130）恢复后直接访问 `result.debug`（132）并改写 `this.intermediateGroupMaterial[bundleName]`（133-134）；`await this.setAppName(materialItems[0])`（140）恢复后改写 `materialItems[key].bundleNameCN`（141）。若用户在上述任一 await 期间快速返回触发 `aboutToDisappear`（`isCompActive=false`），由于 130/140 恢复点无 `isCompActive` 复检，闭包继续在已销毁组件的 `this` 上改写 @State，并在 `setAppName:181` 调用 `this.getUIContext().getHostContext()`（销毁后 getUIContext 可能返回 undefined）。
- **影响**：组件销毁后改写 @State → ArkUI 通常记警告而非崩溃；`getUIContext()` 销毁后返回 undefined 时 `.getHostContext()` 抛 TypeError，被 `getInfos` 的 per-bundle try-catch（129/146）兜住，产生噪声日志与无效异步工作。属逻辑健壮性缺陷，非崩溃/非数据损坏。
- **证据**：`patterns/concurrency.md` CONC-003「对象销毁前未确保异步回调已注销/等待完成」；`patterns/known-defect-patterns/` G6「点位封堵 vs 机制性根治」。本提交引入 `isCompActive` 机制正确（迟到 PixelMap 经 `trackPixelMap` 的 `!isCompActive` 分支释放，无 double-free），但守卫点不完整：仅 111/136/152 三处，130/140 恢复点遗漏。security-scanner「修复方式审查」要求对补丁保持怀疑、验证机制性根治，此处判为点位封堵。
- **建议**：在每个 await 恢复点统一加 `if (!this.isCompActive) return;`；更优方案是用可取消令牌（`aboutToDisappear` 置位、各 await 后检查）把全部异步副作用收敛到单一机制，避免逐点补 guard。同步复核 `setAppName:181` 在销毁后对 `getUIContext()` 的使用。
- **Refute**：security 与 logic 两 scanner 同位置同根因（缺 isCompActive 复检），按 refute 第三层「同一修复改动」合并为一条。维持 P2（框架容忍销毁后 @State 更新，不构成 P0/P1）。

---

### [LOG-02] IntentDetailPage.onBackPress 缺 try-catch，与兄弟页面不一致 (P2, scanner=logic-scanner)

- **位置**：`services/dialog_ui/ams_system_dialog/feature/src/main/ets/view/IntentDetailPage.ets:143-148`
- **触发路径**：用户在详情页按返回 → `onBackPress()` → `AppStorage.get('context') as common.UIAbilityContext`（144）→ `if (context) context.terminateSelf()`（145-147）。本提交新增的 null guard 覆盖了 `AppStorage.get` 返回 undefined 的情形，但函数体无 try-catch；若 `context` 为有效对象但并非真正的 UIAbilityContext（`as` 断言无运行时校验），或 `terminateSelf` 在有效 context 上抛出，异常将逃逸事件回调。
- **影响**：异常逃逸 onBackPress 回调 → ArkUI 记未处理异常，极端情况下影响返回键响应一致性。同模块 `Index.ets:29-38` 与 `IntentContent.ets:96-105` 的同类调用均包裹 try-catch + hilog.error，唯 IntentDetailPage 缺失，结构不一致。
- **证据**：`patterns/error-handling.md` ERR-001~006「错误路径完整性 / 异常处理一致性」；`patterns/logic-correctness.md` LOG-009。本提交 diff 同时触碰三处 onBackPress/terminateSelf，仅为 IntentDetailPage 留下无 try-catch 的缺口。
- **建议**：为 `IntentDetailPage.onBackPress` 补 `try { ... } catch (err) { hilog.error(DOMAIN, TAG, 'Failed to context.terminateSelf. Cause: %{public}s', JSON.stringify(err)); }`，与兄弟页面统一。更进一步：抽一个共享 `safeTerminateSelf(context)` 工具方法，三处复用，消除不一致根因。
- **Refute**：terminateSelf 在有效 UIAbilityContext 上抛出概率低，但结构不一致是事实，且本提交已触碰该函数却未补齐异常处理。维持 P2。

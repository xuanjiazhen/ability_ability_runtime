# 代码检视报告 — commit 61082c292c 安全告警修复（Round 1 / 最新提交）

> 统一报告由 codecheck 工作台生成，用于门禁管控。

---

## 报告元数据

<!-- codecheck-report-metadata:start -->
```yaml
codecheck_report:
  schema_version: "1.0"
  scope: "commit 61082c292c 安全告警修复"
  round: 1
  commit_id: "61082c292c8f3030efb4609bca221105f13214c8"
  change_id: "I63b03585cdaff08e8d48f5f1314351c1cdeb84ba"
  report_id: "I63b03585cdaff08e8d48f5f1314351c1cdeb84ba-R1"
  date: "2026-08-27"
  gate_decision: "approve"
  risk_level: "low"
  score: 94
  dimensions_required: ["security-scanner", "logic-scanner", "input-scanner"]
  dimensions_executed: ["security-scanner", "logic-scanner", "input-scanner"]
  findings_total: 3
  findings_by_severity: {P0: 0, P1: 0, P2: 0, P3: 3}
  gate_blockers: []
  must_fix: []
  followups: ["SEC-01", "LOG-01", "LOG-02"]
```
<!-- codecheck-report-metadata:end -->

---

## 1. 门禁结论

| 项目 | 结论 |
|---|---|
| 决策 | **approve** |
| 风险等级 | 🟢 low |
| 评分 | **94/100** |
| 阻塞项 | 无 |
| 必须修复（P0/P1） | 0 项 |
| 建议跟进（P2/P3） | 3 项 |

**一句话结论**：本次安全告警修复（路径遍历前缀校验 + PixelMap 生命周期管理 + NPE 防御 + 日志合规）实现正确、对合法路径零逻辑影响，仅 3 项 P3 跟进项（无阻塞），准予上库。

---

## 2. 扣分原因

（gate_decision=approve，本节省略）

---

## 3. 必须立即处理（P0/P1）

**无。**

---

## 4. 建议本轮或下一补档处理（P2/P3）

| ID | 优先级 | 问题 | 建议行动 | 排期 |
|---|---|---|---|---|
| SEC-01 | P3 | `IntentContent.ets:60` aboutToDisappear 调 async releasePixelMaps 不 await（fire-and-forget），组件销毁后 release 异步执行，进程立即退出时可能未完成 | 可接受（OS 回收 native 资源）；若需确定释放可在 UIAbility 生命周期或延迟任务中释放 | 下一补档 |
| LOG-01 | P3 | `IntentContent.ets:130` setAppName(materialItems[0]) 循环内始终传 [0]，同 bundle 多 module 时所有 item 的 bundleNameCN 被设为第一个 module 的应用名（预存逻辑 bug，非本次引入） | 改为 materialItems[key]；独立逻辑缺陷，建议另起任务 | 独立任务 |
| LOG-02 | P3 | `IntentDetailPage.ets:144` onBackPress context 为 null 时静默无 fallback，用户按返回无响应 | 可接受（优先防 NPE）；若需更好 UX 可加 articlePathStack.pop() fallback | 下一补档 |

---

## 5. 分维度速览

| 维度 | 结果 | 关键说明 |
|---|---|---|
| security-scanner | ✅ 通过 | 路径遍历修复（IsPathWithinCodePath）实现正确：codePath realpath 规范化 + 补尾 `/` 防误匹配 + size 边界 + 前缀比较；GetSafeData/ReadAmiData 校验点位于 realpath 后 fopen 前。PixelMap 释放（trackPixelMap/releasePixelMaps）正确管理生命周期。NPE 防御（context 空检查）到位。无 P0/P1 高影响缺陷。 |
| logic-scanner | ✅ 通过 | 零逻辑影响：合法路径 realpath 后必在 codePath 内，IsPathWithinCodePath 恒通过；PixelMap 正常加载路径不变；isCompActive 三处异步销毁判断正确。预存 materialItems[0] 逻辑 bug 列为 P3 跟进。 |
| input-scanner | ✅ 通过 | uri→ami→realpath→IsPathWithinCodePath 输入链路校验有效，阻断 `../` 越界；ami.find("el2") 子串匹配虽未改，但 IsPathWithinCodePath 兜底拦截越界，纵深防御成立；IntentContent 异步输入 isCompActive 防销毁后写。 |

---

## 6. 关键发现详情

### [SEC-01] aboutToDisappear fire-and-forget async releasePixelMaps (P3, security-scanner)

- **位置**：`services/dialog_ui/ams_system_dialog/feature/src/main/ets/view/IntentContent.ets:60-62`
- **触发路径**：组件销毁 → aboutToDisappear → isCompActive=false → this.releasePixelMaps()（async，不 await）→ for...of await pm.release() 异步执行 → 若进程立即退出，release 可能未完成
- **影响**：PixelMap native 资源依赖底层 Finalizer 延迟回收；aboutToDisappear 不 await async，理论上 release 可能未完成。实际 OS 在进程退出时回收 native 资源，影响低。
- **证据**：
  ```ts
  aboutToDisappear() {
    this.isCompActive = false;
    this.releasePixelMaps();  // async, not awaited
  }
  ```
- **建议**：当前可接受（ArkTS 生命周期约束，aboutToDisappear 非 async）。若需确定释放，可在 UIAbility 的 onShow/onForeground 阶段或单次延迟任务中释放。不阻塞上库。

### [LOG-01] setAppName 循环内始终传 materialItems[0] (P3, logic-scanner)

- **位置**：`services/dialog_ui/ams_system_dialog/feature/src/main/ets/view/IntentContent.ets:130`
- **触发路径**：getInfos → bundleNames.map async → for (key of materialItems) → setAppName(materialItems[0])（循环内始终传第一个 item）→ 同 bundle 多 module 时所有 item.bundleNameCN 被设为第一个 module 的应用名
- **影响**：UI 显示应用名错误（同 bundle 多 module 场景）。功能缺陷，非安全。
- **证据**：
  ```ts
  for (const key of Object.keys(materialItems)) {
    const iconImg = await this.getAllIntentIcon(materialItems[key].icon, bundleName);
    ...
    const appName = await this.setAppName(materialItems[0]);  // 应为 materialItems[key]
  ```
- **建议**：改为 `this.setAppName(materialItems[key])`。预存 bug（非本次 commit 引入，该行在 diff 上下文未改动），建议另起独立任务修复。

### [LOG-02] onBackPress context null 无 fallback (P3, logic-scanner)

- **位置**：`services/dialog_ui/ams_system_dialog/feature/src/main/ets/view/IntentDetailPage.ets:144-146`
- **触发路径**：用户按返回 → onBackPress → AppStorage.get('context') → 若为 null → if (context) 不执行 terminateSelf → 无其他 fallback（如 articlePathStack.pop）→ 用户按返回无响应
- **影响**：context 为 null 时返回键无响应，用户体验退化。防 NPE 崩溃优先，可接受。
- **证据**：
  ```ts
  onBackPress() {
    let context = AppStorage.get('context') as common.UIAbilityContext;
    if (context) {
      context.terminateSelf();
    }
    // 无 else fallback
  }
  ```
- **建议**：可接受（安全告警优先防 NPE）。若需更好 UX，加 `else { this.articlePathStack.pop(); }` fallback。不阻塞上库。

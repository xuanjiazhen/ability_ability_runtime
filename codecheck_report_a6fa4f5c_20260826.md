# 代码检视报告 — services/dialog_ui/ams_system_dialog（Round 1 / 提交 a6fa4f5c）

> 统一报告由 codecheck 工作台生成，**用于门禁管控**。

---

## 报告元数据

<!-- codecheck-report-metadata:start -->
```yaml
codecheck_report:
  schema_version: "1.0"
  scope: "services/dialog_ui/ams_system_dialog (commit a6fa4f5c fix: 意图调试应用bug)"
  round: 1
  commit_id: "a6fa4f5c294b63c22621a67b83f6ad05dcca8b88"
  change_id: "a6fa4f5c294b63c22621a67b83f6ad05dcca8b88"
  report_id: "a6fa4f5c294b63c22621a67b83f6ad05dcca8b88-R1"
  date: "2026-08-26"
  gate_decision: "approve"
  risk_level: "low"
  score: 90
  dimensions_required: ["security-scanner", "logic-scanner"]
  dimensions_executed: ["security-scanner", "logic-scanner"]
  findings_total: 5
  findings_by_severity: {"P0": 0, "P1": 0, "P2": 0, "P3": 5}
  gate_blockers: []
  must_fix: []
  followups: ["LOG-001", "LOG-002", "LOG-003", "LOG-004", "SEC-001"]
```
<!-- codecheck-report-metadata:end -->

---

## 1. 门禁结论

| 项目 | 结论 |
|---|---|
| 决策 | **approve** |
| 风险等级 | 🟢 low |
| 评分 | **90/100** |
| 阻塞项 | 无（0 项 P0） |
| 必须修复（P0/P1） | 0 项 |
| 建议跟进（P2/P3） | 5 项 |

**一句话结论**：本提交为意图调试系统对话框应用（`services/dialog_ui/ams_system_dialog`，纯 ArkTS UI，无 IPC/DB/公共 API 变更）的 bug 修复——`EntryAbility` 增加深色模式窗口外观适配与 `getMainWindow` 错误处理，`IntentDetailPage` 表单必填校验增强（补 `''`/NaN 检查并弹出"Missing required fields"告警）。变更整体低风险，未发现 P0/P1 缺陷；5 条 P3 观察项集中在窗口生命周期防御与校验覆盖边界。安全维度经检视无真实攻击面（详见 §5）。

---

## 2. 扣分原因

> 仅 gate_decision=block 时呈现。本报告为 approve，本节省略。

---

## 3. 必须立即处理（P0/P1）

**无。**　详情见第 6 节。

---

## 4. 建议本轮或下一补档处理（P2/P3）

| ID | 优先级 | 问题 | 建议行动 | 排期 |
|---|---|---|---|---|
| LOG-001 | P3 | `getMainWindow` 回调未对 `data` 做空值防护，极端下同步抛异常触发全局 restartApp | 在回调内 `if (!data) return;`，补齐本次"crash 加固" | 下一补档 |
| LOG-002 | P3 | `mainWindow` 未在 `onWindowStageDestroy` 置空，配置更新会对已销毁窗口重复 applyAppearance | `onWindowStageDestroy` 中 `this.mainWindow = null` | 下一补档 |
| LOG-003 | P3 | 必填校验不递归 object/array 嵌套字段，嵌套必填子字段全空仍判定通过 | 对 object/array 类型做递归校验或至少提示"需填充子字段" | 下一补档 |
| LOG-004 | P3 | `Number.isNaN(executeParam[key] as number)` 对字符串恒 false，`as number` 不转换 | 改为 `typeof v === 'number' && Number.isNaN(v)` 或统一类型校验 | 下一补档 |
| SEC-001 | P3 | 校验失败日志模板字符串直接插值 `${key}`（无 hilog 格式符/隐私标记），`key` 源自意图元数据 | 改用 `%{public}s` 格式符参数化 | 下一补档 |

---

## 5. 分维度速览

| 维度 | 结果 | 关键说明 |
|---|---|---|
| security-scanner | ✅ 执行（低风险） | 纯 UI 调试工具，无 IPC/权限/敏感数据面；唯一观察项 SEC-001（日志插值）。 |
| logic-scanner | ✅ 执行 | 4 条 P3：窗口生命周期防御（LOG-001/002）、必填校验边界（LOG-003/004）。 |
| input-scanner | N/A | 变更无外部输入→持久化链路（无 DB/文件/IPC 写入），不适用。 |
| api-scanner | N/A | 变更不涉及对外 API（`interfaces/kits/`、NAPI/ANI/C 绑定），不适用。 |

---

## 6. 关键发现详情

> P0/P1 必出全量卡片；P2/P3 按需精选或全出。

### [LOG-001] getMainWindow 回调缺少 data 空指针防护（P3, scanner=logic-scanner）

- **位置**：`services/dialog_ui/ams_system_dialog/feature/src/main/ets/entryability/EntryAbility.ets:112-119`
- **触发路径**：`windowStage.getMainWindow(cb)` 成功分支 → `this.mainWindow = data; this.applyAppearance(data)`。若框架在异常态回调 `(err=undefined, data=null)`（违反 AsyncCallback 契约的极端场景），`applyAppearance` 内 `data.setWindowLayoutFullScreen(true)`（EntryAbility.ets:132）在对象上同步访问属性 → 抛 TypeError，且该异常位于框架回调内（外层 try 仅包住 `getMainWindow` 调用本身，不覆盖回调执行）→ 触发全局 `errorManager` → `restartApp()` 重启整个调试应用。
- **影响**：极端态下调试应用被意外重启（调试工具，无数据破坏）。当前 AsyncCallback 契约下成功必有 data，属防御性缺口。
- **证据**：EntryAbility.ets:112-120（错误分支仅判 `err && err.code !== 0`，无 `!data` 判空）；EntryAbility.ets:131-142（`applyAppearance` 对 `data` 直接解引用）。
- **建议**：`if (err && err.code !== 0 || !data) return;` 或在 `applyAppearance` 入口判空。

### [LOG-002] mainWindow 未在 onWindowStageDestroy 置空（P3, scanner=logic-scanner）

- **位置**：`services/dialog_ui/ams_system_dialog/feature/src/main/ets/entryability/EntryAbility.ets:161-163`
- **触发路径**：窗口阶段销毁（Ability 后台/退出/折叠等）后收到 `onConfigurationUpdate` → `onConfigurationUpdate`（153-159）命中 `if (this.mainWindow)` → 对已销毁窗口调用 `applyAppearance`。
- **影响**：同步 `setWindowBackgroundColor` 已被 try-catch、异步调用均带 `.catch`，不会崩溃，但产生冗余失败日志与生命周期状态残留；窗口销毁后重建场景下可能沿用过期窗口句柄。
- **证据**：`onWindowStageDestroy`（161-163）仅打日志、未置空 `this.mainWindow`；`onConfigurationUpdate`（156-158）直接复用 `this.mainWindow`。
- **建议**：`onWindowStageDestroy` 中 `this.mainWindow = null`，与 `onWindowStageCreate` 对称。

### [LOG-003] 必填校验不递归 object/array 嵌套字段（P3, scanner=logic-scanner）

- **位置**：`services/dialog_ui/ams_system_dialog/feature/src/main/ets/view/IntentDetailPage.ets:358-366`（配合 260-262）
- **触发路径**：必填字段为 object/array 类型时，`executeParam[key]` 为（可能子字段全空的）对象 → 恒不满足 `undefined/null/''/NaN` 四条件 → 校验通过并 `executeLinkIntent` 执行。
  - 例：必填 `person` 且其子字段 `person.name` 为空 → `format(this.formData)` 产出 `executeParam['person'] = { name: '' }` → 校验通过。
- **影响**：与本次"Missing required fields"告警目的不符——嵌套必填子字段缺失时既不弹告警也不拦截。前置实现同样未递归，属本次校验增强的覆盖盲区，非本提交引入的回归。
- **证据**：`changeProperties`（260-262）仅对顶层 `propKey` 标记 `formObj.required`；`submitForm`（358-366）仅查 `executeParam[key]` 顶层值；`format()`（format.js:16-32）将点号键展开为嵌套对象。
- **建议**：对 object/array 必填字段递归校验子字段，或在 UI 上对嵌套必填项同样展示"必填"标记并逐层校验。

### [LOG-004] Number.isNaN 的 as number 断言不转换，对字符串恒 false（P3, scanner=logic-scanner）

- **位置**：`services/dialog_ui/ams_system_dialog/feature/src/main/ets/view/IntentDetailPage.ets:360`
- **触发路径**：`Number.isNaN(executeParam[key] as number)` —— ArkTS/TS 的 `as number` 是编译期类型断言，非运行时转换。`Number.isNaN('abc') === false`。当前 number 字段经 `buildNumberInput` 的 `parseInt`（523 行）在非数字输入时存**真实 NaN**，故该检查对现有 number 路径有效；但一旦值以字符串形态出现（如 string 字段存数字串、或后续改存原始输入），非数字内容将绕过校验。
- **影响**：校验覆盖面脆弱、类型依赖隐性，后续重构易引入"无效数值放行"。
- **证据**：IntentDetailPage.ets:360（`as number` 断言）；523（`parseInt` 失败产 NaN）。
- **建议**：改为 `typeof v === 'number' && Number.isNaN(v)`，或对 number 类型字段统一用 `Number(value)` 解析并校验 `isNaN`。

### [SEC-001] 校验失败日志模板字符串直接插值（P3, scanner=security-scanner）

- **位置**：`services/dialog_ui/ams_system_dialog/feature/src/main/ets/view/IntentDetailPage.ets:363`
- **触发路径**：`hilog.error(DOMAIN, TAG, \`submitForm validation failed: required field '${key}' is missing\`)` —— `${key}` 来自意图元数据 `this.required`（`this.intentDetail.parameters['required']`），以模板字符串直接嵌入，未走 hilog 格式符/隐私标记。
- **影响**：`key` 含 `\r`/`\n` 时可造成日志注入/格式污染；调试工具且数据为测试意图，实际影响极小。
- **证据**：IntentDetailPage.ets:363（模板插值）；key 来源见 128-129 行。
- **建议**：改用 `hilog.error(DOMAIN, TAG, 'submitForm validation failed: required field %{public}s is missing', key)`。

---

> 修复由用户确认后另起任务执行，本检视不改源码。

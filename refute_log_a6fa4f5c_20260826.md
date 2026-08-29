# Refute Log — 提交 a6fa4f5c294b63c22621a67b83f6ad05dcca8b88（Round 1）

> 对抗性验证记录。本提交无 P0 发现，按 conventions §13.2，仅需对 P0 做强制 refute；此处对全部 5 条 P3 发现补记三层质疑结论，便于人工复核捞回。

## 发现的判定总览

| ID | 判定 | 说明 |
|---|---|---|
| LOG-001 | ✅ 维持（P3） | 防御性缺口确认，但触发依赖违反 AsyncCallback 契约的极端态，维持 P3 不升级。 |
| LOG-002 | ✅ 维持（P3） | 生命周期残留确认；所有窗口 API 均有错误保护，无崩溃路径，维持 P3。 |
| LOG-003 | ✅ 维持（P3） | 递归校验盲区确认；属前置行为 + 本次增强覆盖范围外，维持 P3。 |
| LOG-004 | ✅ 维持（P3） | `as number` 非转换确认；当前 parseInt 路径使检查有效，但类型依赖脆弱，维持 P3。 |
| SEC-001 | ✅ 维持（P3） | 模板字符串插值确认；调试工具影响极小，维持 P3。 |

---

## 逐条三层质疑

### LOG-001 — getMainWindow 回调 data 空指针防护
- **第一层（触发路径证伪）**：`getMainWindow` 的 AsyncCallback 契约下，成功时 `err` 为 undefined 且 `data` 必为 Window。`data=null` 需框架违反契约。→ 未推翻，但概率极低。
- **第二层（影响夸大证伪）**：若触发，为回调内同步 TypeError，外层 try 不覆盖回调执行（try 仅包 `getMainWindow` 调用），将进入全局 errorManager → `restartApp()` 重启调试应用。后果为应用重启、无数据破坏；且依赖契约违反才可达。→ 未夸大，维持 P3（防御项）。
- 结论：✅ 维持 P3。

### LOG-002 — mainWindow 未在 onWindowStageDestroy 置空
- **第一层**：`onConfigurationUpdate` 在窗口阶段销毁后仍可回调，代码可到达。→ 可达。
- **第二层**：`applyAppearance` 中 `setWindowBackgroundColor`（同步）在 try-catch 内，`setWindowLayoutFullScreen`/`setWindowSystemBarProperties`（异步）均有 `.catch`，对已销毁窗口最多抛错被捕获 → 无崩溃、仅冗余日志。→ 影响未夸大，维持 P3。
- 结论：✅ 维持 P3。

### LOG-003 — 必填校验不递归嵌套字段
- **第一层**：object/array 必填字段存在且 `format()` 将点号键展开为嵌套对象（format.js:16-32），`executeParam[key]` 为对象恒通过四条件。→ 可达。
- **第二层**：后果为"嵌套空必填字段放行执行"，仅影响调试工具校验完备性，无崩溃/安全影响。→ 维持 P3。
- **前置性核对**：旧实现仅判 undefined/null，嵌套对象同样通过；本提交未引入回归，但属本次校验增强覆盖盲区，登记跟进。→ ✅ 维持 P3。

### LOG-004 — Number.isNaN 对字符串恒 false
- **第一层**：`Number.isNaN('abc' as number)` 经断言后运行时仍为字符串 → `Number.isNaN('abc')===false`。确认 `as` 为编译期断言。→ 属实。
- **第二层**：当前 number 字段全部经 `parseInt`（失败存真实 NaN），使检查在现有路径有效；无当前可达的错误放行。→ 影响未夸大，维持 P3（脆弱性/潜在风险）。
- 结论：✅ 维持 P3。

### SEC-001 — 日志模板字符串插值
- **第一层**：`key` 源自 `this.required`（意图元数据），确实以 `${key}` 直接嵌入 hilog 字符串。→ 属实。
- **第二层**：`key` 中注入 `\r`/`\n` 仅造成调试日志污染；工具为开发者调试用途、数据为测试意图、日志输出域受限。→ 影响未夸大，维持 P3。
- 结论：✅ 维持 P3。

---

## 被推翻/降级条目

无。所有发现均维持。

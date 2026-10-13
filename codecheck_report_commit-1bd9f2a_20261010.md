# 代码检视报告 — commit 1bd9f2a（镜像启动支持静态）（Round 1 / 最新提交）

> 统一报告由 codecheck 工作台生成，**用于门禁管控**。所有 codecheck 报告（含 orchestrator 合并出的统一报告、单 scanner 直接产出的统一报告）必须遵循本模板：章节顺序、字段名、报告元数据块、评分与门禁规则均为**固定格式**，跨报告保持一致，便于门禁脚本解析与历史对比。
> 生成入口：[`README.md`](skills/codecheck/README.md) → Step 5；合并逻辑见 [`orchestrator/SKILL.md`](skills/codecheck/orchestrator/SKILL.md)。
> 权威评分与门禁规则为**通用规则**，不在输出报告中呈现；生成时必须按 [`conventions.md`](skills/codecheck/conventions.md) §7（等级归一化）/ §8（必检维度）/ §9（评分与决策矩阵）计算，不得自创分值。

---

## 报告元数据

> **门禁脚本只读取本 YAML 块**。字段名与取值域为固定合约，禁止改名、增删或自定义取值。人工阅读部分从「1. 门禁结论」开始。

<!-- codecheck-report-metadata:start -->
```yaml
codecheck_report:
  schema_version: "1.0"
  scope: "commit-1bd9f2a-镜像启动支持静态"
  round: 1
  commit_id: "1bd9f2a31ab3a93c637d7dc4f2442a1633e90236"
  change_id: "If48f5494232fdf2bad74ae3cfa11b6bb952d2658"
  report_id: "If48f5494232fdf2bad74ae3cfa11b6bb952d2658-R1"
  date: "2026-10-10"
  gate_decision: "approve"
  risk_level: "low"
  score: 91
  dimensions_required: ["security-scanner", "logic-scanner"]
  dimensions_executed: ["security-scanner", "logic-scanner"]
  findings_total: 3
  findings_by_severity: {P0: 0, P1: 0, P2: 1, P3: 2}
  gate_blockers: []
  must_fix: []
  followups: ["SEC-01/LOG-01", "SEC-02", "LOG-02"]
```
<!-- codecheck-report-metadata:end


---

## 1. 门禁结论

| 项目 | 结论 |
|---|---|
| 决策 | **approve** |
| 风险等级 | 🟢 low |
| 评分 | **91/100** |
| 阻塞项 | 无 |
| 必须修复（P0/P1） | 0 项 |
| 建议跟进（P2/P3） | 3 项 |

**一句话结论**：路径特征无 IPC / 持久化信号，走 security + logic 双维度扫描；未发现 P0/P1 致命/严重问题，1 项 P2 路径分隔符保护回归 + 2 项 P3 观察项，评分 91，准予上库。

---

## 3. 必须立即处理（P0/P1）

**无。**

---

## 4. 建议本轮或下一补档处理（P2/P3）

| ID | 优先级 | 问题 | 建议行动 | 排期 |
|---|---|---|---|---|
| SEC-01/LOG-01 | P2 | `rfind(".")` 替换 `RemoveFileExtension` 丢失路径分隔符保护，路径含点时截断 | 恢复 `RemoveFileExtension` 调用或内联补 `lastSep` 检查 | 本轮或下一补档 |
| SEC-02 | P3 | `preloadList_` 无 mutex 保护，与同类 `cachedPatches_` 不一致 | 确认访问始终单线程后登记为观察项，或补 `mutex` | 下一补档 |
| LOG-02 | P3 | 生产者/消费者 `srcPath` 起始字段不同（`info.package` vs `hapModuleInfo.name`） | 验证消费侧 key 构造一致性，确认无跨路径 key 碰撞 | 下一补档 |

---

## 5. 分维度速览

| 维度 | 结果 | 关键说明 |
|---|---|---|
| security-scanner | ✅ 通过 | 11 项 Core Scan 全量判定；1 项 P2（路径分隔符回归），1 项 P3（无锁容器观察）；G12 同类横扫 3 处 `rfind(".")` 无 `lastSep`（2 处本提交引入，1 处预存） |
| logic-scanner | ✅ 通过 | 控制流/数据流/边界/错误处理逐项扫描；1 项 P2 与 security 同根因（合并），1 项 P3（key 构造差异观察） |

---

## 6. 关键发现详情

### [SEC-01/LOG-01] `rfind(".")` 替换 `RemoveFileExtension` 丢失路径分隔符保护 (P2, scanner=security+logic)

- **位置**：`frameworks/native/appkit/app/main_thread.cpp:2386-2390`
- **触发路径**：`HandleLaunchApplication` → `PreloadModule` → `ProcessMainAbility` → 构造 `srcPath`（起始 `info.package` + `/` + `info.srcEntrance`）→ `rfind(".")` 找到路径中最后一个点 → `erase(pos)` 截断 → `append(".abc")` → 生成垃圾路径 → `PreloadMainAbility` → `LoadModule(错误路径)` → 返回 nullptr → 存入 `preloadList_`。
- **影响**：当路径在文件名之前含有 `.`（如 `info.package` 为 `com.example.app` 或目录名含点 `ets/v1.0/`），`rfind(".")` 定位到非扩展名点，路径被错误截断。原 `RemoveFileExtension`（`js_runtime_utils.h:73-80`）通过 `lastSep` 检查确保只擦除最后一个路径分隔符之后的点，新内联代码丢失此保护。实际后果为预加载失败（模块加载两次），非崩溃——`LoadModule` 返回 nullptr 被 `PopPreloadObj` 正确处理，消费者回退到自身 `LoadModule`。
- **证据**：
  - 原函数 `RemoveFileExtension`（`interfaces/inner_api/runtime/include/js_runtime_utils.h:73-80`）检查 `lastSep`：`if (lastDot != std::string::npos && (lastSep == std::string::npos || lastDot > lastSep))`
  - 新代码（`main_thread.cpp:2386-2390`）仅检查 `pos != std::string::npos`，无 `lastSep` 检查
  - 同类写法（G 类横扫）：`ets_ability_stage.cpp:163-167`（本提交引入，`srcPath` 起始 `hapModuleInfo.name` 通常无点，影响低）；`js_test_runner.cpp:266-269`（预存，非本提交引入）
- **建议**：恢复使用 `AbilityRuntime::RemoveFileExtension(srcPath)` 并在之后判断是否需要追加 `.abc`（静态模式不追加），或内联补 `lastSep` 检查：
  ```cpp
  auto pos = srcPath.rfind(".");
  auto lastSep = srcPath.rfind("/");
  if (pos != std::string::npos && (lastSep == std::string::npos || pos > lastSep)) {
      srcPath.erase(pos);
      srcPath.append(".abc");
  }
  ```

### [SEC-02] `preloadList_` 无 mutex 保护，与同类成员不一致 (P3, scanner=security)

- **位置**：`interfaces/inner_api/runtime/include/ets_runtime.h:127`（成员声明），`frameworks/native/runtime/ets_runtime.cpp:743`（`PreloadMainAbility` 写入），`frameworks/native/runtime/ets_runtime.cpp:746-758`（`PopPreloadObj` 读写）
- **触发路径**：`preloadList_` 为 `std::unordered_map<string, unique_ptr<ETSNativeReference>>`，在 `PreloadMainAbility`（`operator[]` 写入）和 `PopPreloadObj`（`find` + `operator[]` + `std::move` + `erase`）中均无锁保护。同类成员 `cachedPatches_` 有 `quickFixMutex_` 保护（`ets_runtime.h:128-129`），保护策略不一致。
- **影响**：当前不可触发——预加载在父进程 fork 前主线程执行，`PopPreloadObj` 在子进程 fork 后主线程执行，fork 后单线程。若未来线程模型变更（如预加载在 worker 线程执行），则 `find` + `operator[]` + `std::move` + `erase` 序列存在 TOCTOU 竞态，可导致 UAF 或记录丢失。
- **证据**：`ets_runtime.h:127-129` 对比 `preloadList_`（无锁）与 `cachedPatches_`（有 `quickFixMutex_`）；`ets_runtime.cpp:748` `preloadList_.find(key)` 与 `ets_runtime.cpp:751` `preloadList_[key]` 之间存在窗口（当前单线程无实际竞态）
- **建议**：确认访问始终单线程后登记为观察项；若不确定线程模型，为 `preloadList_` 补 `std::mutex` 并在 `PreloadMainAbility` / `PopPreloadObj` 中加锁。

### [LOG-02] 生产者/消费者 srcPath 起始字段不同 (P3, scanner=logic)

- **位置**：`frameworks/native/appkit/app/main_thread.cpp:2368`（`std::string srcPath(info.package)`）vs `frameworks/native/appkit/ability_runtime/app/ets_ability_stage.cpp:156`（`std::string srcPath(hapModuleInfo.name)`）
- **触发路径**：`ProcessMainAbility` 构造 `srcPath` 起始为 `info.package`，而 `ETSAbilityStage::LoadModule` 构造 `srcPath` 起始为 `hapModuleInfo.name`。预加载 key = `moduleName + "::" + srcPath`，若两侧 `srcPath` 前缀不同则 key 不匹配，预加载条目不会被 pop。
- **影响**：当前确认这两条路径预加载的是不同对象（MainAbility vs AbilityStage），key 前缀（`moduleName`）也不同，本就不应匹配——这是设计如此。但消费侧（被排除扫描的 `ets_ui_ability.cpp` 等）的 `srcPath` 构造未验证，存在潜在 key 不一致风险。
- **证据**：`main_thread.cpp:2368` `info.package` vs `ets_ability_stage.cpp:156` `hapModuleInfo.name`；`ets_ability_stage.cpp:158` `moduleName` 为 `hapModuleInfo.moduleName + "::AbilityStage"`，与 `main_thread.cpp:2395` `info.moduleName + "::" + info.name` 不同
- **建议**：下一补档验证 `ets_ui_ability.cpp` 等消费侧的 `srcPath` 构造与 `ProcessMainAbility` 一致，确认 STATIC 模式下 key 能正确匹配。

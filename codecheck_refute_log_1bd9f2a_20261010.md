# Refute 审查记录 — commit 1bd9f2a31ab3a93c637d7dc4f2442a1633e90236

## 审查概要

| 指标 | 数值 |
|------|------|
| 审查范围 | security: 2 条, logic: 2 条 |
| P0/P1 全审 | 0 条（无 P0/P1） |
| P2 审查 | 1 条 |
| P3 不进入 refute | 2 条 |
| 维持 | 3 条 |
| 降级 | 0 条 |
| 推翻 | 0 条 |
| 合并 | 1 组 → 1 条 |
| **进入最终报告** | **3 条** (P0: 0, P1: 0, P2: 1, P3: 2) |

## 逐条审查

### ✅ 维持 P2 — SEC-01 / LOG-01（合并）

**发现摘要**：`frameworks/native/appkit/app/main_thread.cpp:2386-2390` 用内联 `rfind(".")` 替换 `RemoveFileExtension(srcPath)`，丢失了原函数的 `lastSep`（路径分隔符）保护。当路径在文件名之前含有 `.`（如 `info.package` 为 `com.example.app` 或目录名含点），`rfind(".")` 找到的不是文件扩展名点而是路径中间的点，`erase(pos)` 截断路径，生成垃圾 `srcPath`。

**第一层：触发路径证伪**
- 追溯上游：`srcPath` 起始为 `info.package`（`main_thread.cpp:2368`），追加 `/` + `info.srcEntrance`。
- Stage 模型下 `info.package` 通常为模块名（如 `entry`），不含点。`info.srcEntrance` 在 STATIC 模式下使用 OHM 寻址（`/` 分隔，无 `.ets` 扩展名）。
- 但 `info.package` 可被开发者设为含点名称（如 `com.example.app`），且 `srcEntrance` 路径中目录名也可能含点（如 `ets/v1.0/pages/Index`）。触发条件非常见但确实可达。
- 判定：**不推翻**——触发条件不常见但确实可达，不满足"确认不可达"标准。

**第二层：影响夸大证伪**
- 路径被截断后：`PreloadMainAbility` → `LoadModule(错误路径)` → 返回 nullptr → 存入 `preloadList_`。
- `PopPreloadObj` 检查 nullptr → `erase` + 返回 false → 消费者回退到自身 `LoadModule`。
- 消费者侧 `srcPath` 起始为 `hapModuleInfo.name`（如 `entry`，通常无点），消费者路径正确。
- 实际影响：预加载浪费（模块被加载两次），非崩溃。P2 合适。
- 判定：**维持 P2**。

**第三层：跨 scanner 根因去重**
- SEC-01（安全视角：路径/内存安全）和 LOG-01（逻辑视角：`RemoveFileExtension` 不变量破坏）根因相同——`main_thread.cpp:2386` 缺 `lastSep` 检查。
- 修复方案同一处：恢复 `lastSep` 检查或直接复用 `RemoveFileExtension`。
- 判定：**合并为 SEC-01/LOG-01**。

### ✅ 维持 P3 — SEC-02

**发现摘要**：`preloadList_`（`ets_runtime.h:127`）为 `std::unordered_map`，在 `PreloadMainAbility`（写）和 `PopPreloadObj`（读写）中无 mutex 保护。同类 `cachedPatches_` 有 `quickFixMutex_` 保护，不一致。

**第一层：触发路径证伪**
- 预加载在父进程（fork 前）主线程执行；`PopPreloadObj` 在子进程（fork 后）主线程执行。fork 后单线程。
- 当前不可触发竞态。
- 判定：**维持 P3**（当前不可达，未来风险观察项）。

### ✅ 维持 P3 — LOG-02

**发现摘要**：`ProcessMainAbility`（`main_thread.cpp:2368`）的 `srcPath` 起始为 `info.package`，而 `ETSAbilityStage::LoadModule`（`ets_ability_stage.cpp:156`）的 `srcPath` 起始为 `hapModuleInfo.name`。若两者不同，预加载 key 不匹配，预加载条目永远不会被 pop。

**第一层：触发路径证伪**
- `ProcessMainAbility` 预加载的是 MainAbility，`ETSAbilityStage::LoadModule` 加载的是 AbilityStage——不同代码路径、不同 key 前缀（`moduleName` 也不同），本就不应匹配。
- 这是设计如此，不是 bug。
- 判定：**维持 P3**（观察项，记录 key 构造差异供后续验证消费侧一致性）。

## refute 耗时与统计
- 审查耗时：约 3 分钟
- 调用链追踪：4 次
- 代码片段二次验证：3 次

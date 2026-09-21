# Refute Log — startAbilityByType 异步回调重构（Round 1）

> 对抗性验证记录。对 4 个 scanner（security/logic/input/api）提交的 25 条原始发现执行三层质疑，记录每条判定：✅ 维持 | ⬇️ 降级 | ❌ 推翻 | 🔀 合并。被推翻项不进入统一报告正文。

- **commit**: 2fd02d65fc17a85b78c67ba441aaec970ca3f508
- **date**: 2026-09-20

---

## 第三层：跨 scanner 根因去重

### 🔀 合并组 A — SUPPORT_SCREEN 未定义时异步回调不触发导致 Promise 永久挂起

| 原始 ID | scanner | 原始等级 | 合并后 |
|---------|---------|---------|--------|
| LOG-01 | logic | P1 | P1 |
| SEC-01 | security | P2 | — |
| API-02 | api | P2 | — |

**根因**：`#ifdef SUPPORT_SCREEN` 块在 SUPPORT_SCREEN 未定义时被整体跳过，新异步模型删除了旧同步模型的默认错误返回，且未补 `#else` 回退路径。表现为两类入口点：
- **execute lambda 层**（LOG-01）：`js_ability_context.cpp:3147`、`js_ui_extension_context.cpp:1707`、`ets_ability_context.cpp:1723` 中 `#ifdef SUPPORT_SCREEN` 包裹 `context->StartAbilityByType()` 调用，跳过后 `innerErrCode` 保持 `ERR_OK` → complete 回调 Hold → 挂死。
- **content_session 创建层**（SEC-01/API-02）：`ets_ui_extension_content_session.cpp:922`、`js_ui_extension_content_session.cpp:998` 中 `#ifdef SUPPORT_SCREEN` 包裹 ModalUIExtension 创建，跳过后 asyncCallback 不触发 → 引用泄漏。

**判定**：🔀 合并，取最高 P1。修复点不同但根因相同（异步模型遗漏 `#else` 回退）。合并 ID：LOG-01+SEC-01+API-02。

### 🔀 合并组 B — ClearAsyncResult 死代码

| 原始 ID | scanner | 原始等级 | 合并后 |
|---------|---------|---------|--------|
| SEC-03 | security | P3 | P3 |
| LOG-07 | logic | P3 | — |

**根因**：`ClearAsyncResult()`（`js_ui_extension_callback.cpp:140`）定义但全库 grep 无调用点。
**判定**：🔀 合并，同一 file:line 同一问题。合并 ID：SEC-03+LOG-07。相关测试缺失 API-10 独立保留（不同性质：死代码 vs 测试缺失）。

---

## 第一层 + 第二层：逐条质疑

| ID | scanner | 原始 | 判定 | 理由 |
|----|---------|------|------|------|
| LOG-01 | logic | P1 | 🔀→P1 | 合并组 A |
| SEC-01 | security | P2 | 🔀 | 合并组 A |
| API-02 | api | P2 | 🔀 | 合并组 A |
| LOG-02 | logic | P1 | ✅ 维持 P1 | OnError 不处理 deferred_/completionCallback_ 为确定事实；CloseModalUIExtension 后 onAbilityErrorCode 是否触发有不确定性，证据等级 Likely |
| API-01 | api | P1 | ✅ 维持 P1 | ABILITY_SERVICE_NOT_CONNECTED 不在 INNER_TO_JS_ERROR_CODE_MAP 已确认；码 16000050 与消息"Service unavailable."矛盾，误导应用 |
| API-08 | api | P1 | ✅ 维持 P1 | Promise 模式 reject 路径零测试，生产 Promise 走 deferred_ 分支 |
| API-07 | api | P0 | ✅ 维持 P0 | 核心新增方法 OnAbilityByTypeResult 两条路径（resolve/reject）零测试，grep 确认；P0 推翻需"确认不可达"级证据，测试缺失为事实不可推翻 |
| LOG-03 | logic | P2 | ✅ 维持 P2 | 无超时兜底，Ace 层不回调则 Promise 永久 pending |
| LOG-04 | logic | P2 | ✅ 维持 P2 | phase != FOREGROUND 直接 return，非前台错误不触发 OnAbilityByTypeResult |
| LOG-05 | logic | P2 | ✅ 维持 P2 | napi_create_reference 失败概率低（OOM），但一旦发生无错误反馈路径 |
| LOG-06 | logic | P2 | ✅ 维持 P2 | 旧代码有 .then(handler, errorHandler)，新代码移除错误处理，taskpool 拒绝时 AsyncCallbackWrapper 永不触发 |
| INP-01 | input | P2 | ✅ 维持 P2 | targetType 应用可控，%{public}s 不过滤 CRLF；pre-existing 但 commit 变更 gating 条件（HasParameter）扩大进入路径 |
| API-03 | api | P2 | ✅ 维持 P2 | 基类位于 interfaces/kits/native/（公共边界），新增纯虚对 out-of-tree 子类是 ABI 破坏；in-tree 2 子类已实现 |
| API-06 | api | P2 | ✅ 维持 P2 | NAPI 无 callback 分支，ANI 通过 AsyncCallbackWrapper 支持双模式，需核对 d.ts 是否有 callback 重载 |
| API-09 | api | P2 | ✅ 维持 P2 | CountCompletionCallback 仅计次不校验 argv，无法防止码映射回归 |
| API-10 | api | P2 | ✅ 维持 P2 | ClearAsyncResult 新增方法零测试 |
| SEC-02 | security | P2 | ✅ 维持 P2 | 成功路径 onAbilityErrorCode 是否以 errorCode=0 触发依赖 Ace 层实现（不在本仓库），证据 Suspicious |
| SEC-03 | security | P3 | 🔀→P3 | 合并组 B |
| LOG-07 | logic | P3 | 🔀 | 合并组 B |
| SEC-04 | security | P3 | ✅ 维持 P3 | pre-existing；本 commit 同文件 SetCompletionCallback 正确检查形成不一致；触发需 napi_create_reference 失败（极端条件） |
| LOG-08 | logic | P3 | ✅ 维持 P3 | HasParameter vs !empty() 语义变更，参数存在但值为空时行为改变；可能有意修复 |
| INP-02 | input | P3 | ✅ 维持 P3 | HiSysEvent MODULE_NAME 为结构化 STRING 字段，非 format string，已充分校验 |
| API-04 | api | P3 | ✅ 维持 P3 | NAPI/ANI 双模式处理模型不对称，功能等价但维护分叉 |
| API-05 | api | P3 | ✅ 维持 P3 | 本 commit 已清理合并冲突标记，登记为流程观察项 |
| API-11 | api | P3 | ✅ 维持 P3 | ANI 绑定层新增三方法零测试 |

---

## 汇总

- 原始发现：25 条（security 4 + logic 8 + input 2 + api 11）
- 合并：2 组（LOG-01+SEC-01+API-02、SEC-03+LOG-07）→ 减少 3 条
- 推翻：0 条
- 降级：0 条
- **进入统一报告：22 条**（P0: 1, P1: 4, P2: 10, P3: 7）
- **score = max(0, 100 − (30×1 + 12×4 + 5×10 + 2×7)) = max(0, 100 − 142) = 0**
- **gate_decision = block**（存在 P0）| **risk_level = high**

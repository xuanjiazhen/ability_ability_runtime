# Execution Plan: setAbilityInstanceInfo 新增必选 groupId 参数重载

## 输入状态

| 输入 | 路径 | 要求状态 |
|------|------|----------|
<<<<<<< Updated upstream
| Requirement | `.codespec/changes/issue-16070-add-groupid/proposal.md` | Approved |
| Design | `.codespec/changes/issue-16070-add-groupid/design.md` | Approved |
| Spec | `.codespec/changes/issue-16070-add-groupid/spec.md` | Approved |

## 执行原则

- **Spec 权威：** 实现与 AC、错误码、兼容性声明冲突时，停止并修订 spec/design/plan。
- **测试/证据先行：** 每个 Task 先确认失败测试或可复现证据缺口，再做最小实现并取得 fresh PASS 证据。
- **任务小型化：** 一个 Task 只覆盖一个独立闭环。
- **文件边界：** 只能修改 Task 的 Files/允许修改范围。
- **证据回填：** Task 完成当次会话回填追溯状态、实际文件、Commit、Review Evidence 和 Actual Result。

## 受影响文件全量清单

| 仓 | 层 | 文件路径 | 修改类型 | 说明 |
|----|-----|----------|----------|------|
| ability_runtime | C++ SDK 接口 | `interfaces/kits/native/ability/ability_runtime/ability_context.h` | 修改 | 新增 groupId 第三参数声明 |
| ability_runtime | C++ SDK 实现声明 | `interfaces/kits/native/ability/ability_runtime/ability_context_impl.h` | 修改 | 新增 groupId 参数 override 声明 |
| ability_runtime | C++ SDK 实现 | `frameworks/native/ability/ability_runtime/ability_context_impl.cpp` | 修改 | 透传 groupId 到 SetSessionLabelAndIcon |
| ability_runtime | ETS 接口 | `frameworks/ets/ets/application/UIAbilityContext.ets` | 修改 | 新增 groupId 可选参数 |
| ability_runtime | ANI 绑定 | `frameworks/ets/ani/ui_ability/src/ets_ability_context.cpp` | 修改 | groupId 解析、校验、异步调度 |
| ability_runtime | ANI 绑定头文件 | `frameworks/ets/ani/ui_ability/include/ets_ability_context.h` | 修改 | groupId 参数声明 |
| ability_runtime | NAPI 绑定 | `frameworks/native/ability/native/ability_runtime/js_ability_context.cpp` | 修改 | groupId 可选解析、校验 |
| ability_runtime | NAPI 绑定头文件 | `interfaces/kits/native/ability/native/ability_runtime/js_ability_context.h` | 修改 | groupId 参数声明 |
| ability_runtime | FA JS 绑定 | `frameworks/js/napi/ability_context/ability_context.js` | 修改 | 新增 groupId 参数 |
| ability_runtime | 单元测试 | `test/unittest/frameworks_kits_ability_ability_runtime_test/ability_context_impl_second_test.cpp` | 修改 | groupId 场景覆盖 |

**检查项：**
- [x] design.md 调用链每一层都有对应文件列出
- [x] 每个文件修改类型和职责说明明确
- [x] 无映射行的层意味着设计到实现的翻译缺失

## AC 到 Task 追溯

| AC | 来源 | Task | 计划代码范围 | 实际 Code Ref | Commit | Review Evidence | 验证状态 |
|----|------|------|--------------|-----------------|--------|-----------------|----------|
| AC-1.1 | spec.md | TASK-1 | `ability_context.h:490`, `ability_context_impl.cpp:1149-1164` | `ability_context.h:490`, `ability_context_impl.cpp:1149-1164` | `75708f6d89` | `evidence/reviews/spec-compliance.md#AC-1.1` | Pass |
| AC-1.2 | spec.md | TASK-1 | `ability_context_impl.cpp:1153-1154` | `ability_context_impl.cpp:1153-1154` | `75708f6d89` | `evidence/reviews/spec-compliance.md#AC-1.2` | Pass |
| AC-2.1 | spec.md | TASK-2 | `UIAbilityContext.ets:780`, `js_ability_context.cpp:3075-3076` | `UIAbilityContext.ets:780`, `js_ability_context.cpp:3075-3076` | `75708f6d89` | `evidence/reviews/spec-compliance.md#AC-2.1` | Pass |
| AC-2.2 | spec.md | TASK-2 | `UIAbilityContext.ets:780` | `UIAbilityContext.ets:780` | `75708f6d89` | `evidence/reviews/spec-compliance.md#AC-2.2` | Pass |
| AC-3.1 | spec.md | TASK-2 | `js_ability_context.cpp:3078-3083`, `ets_ability_context.cpp:810-814` | `js_ability_context.cpp:3078-3083`, `ets_ability_context.cpp:810-814` | `75708f6d89` | `evidence/reviews/spec-compliance.md#AC-3.1` | Pass |

## 首批实现边界

**首批必须实现：** C++ SDK 接口层 groupId 参数（TASK-1），绑定层依赖 SDK 层签名
**可后置：** 测试补全（TASK-3）
**不建议延后：** 无

## Task 列表

| Task ID | 目标 | 文件范围 | AC 映射 | 前置依赖 | 完成判据 | 验证命令 | 状态 |
|---------|------|----------|---------|----------|----------|----------|------|
| TASK-1 | C++ SDK groupId 参数新增与透传 | `ability_context.h`, `ability_context_impl.h`, `ability_context_impl.cpp` | AC-1.1, AC-1.2 | 无 | 接口签名含 groupId；实现透传至 SetSessionLabelAndIcon；单测通过 | `ability_context_impl_second_test.cpp::SetAbilityInstanceInfo_0100` | Done |
| TASK-2 | 绑定层 groupId 解析、校验与 ETS/JS 接口 | `js_ability_context.cpp`, `ets_ability_context.cpp`, `UIAbilityContext.ets`, `ability_context.js` | AC-2.1, AC-2.2, AC-3.1 | TASK-1 | ETS groupId 可选默认空串；NAPI/ANI 校验 64 字节上限；超长抛参数错误 | 编译通过 | Done |
| TASK-3 | 单元测试 groupId 场景覆盖 | `ability_context_impl_second_test.cpp` | AC-1.1, AC-1.2 | TASK-1, TASK-2 | SetAbilityInstanceInfo_0100 含 groupId 场景；TransferSetAbilityInstanceInfoErr_0100 通过 | `ability_context_impl_second_test` | Done |

## Task 详情

### TASK-1: C++ SDK groupId 参数新增与透传

| 字段 | 内容 |
|------|------|
| 任务目标 | 在 `AbilityContext::SetAbilityInstanceInfo` 接口和实现中新增 groupId 参数，透传至窗口 ISession |
| AC 映射 | AC-1.1, AC-1.2 |
| 前置依赖 | 无 |
| 非目标 | 不在 SDK 层做 groupId 长度校验（在绑定层） |
| 完成判据 | 接口签名含 groupId；实现调用 `SetSessionLabelAndIcon(label, icon, groupId)` |
| 停止条件 | 接口签名变更导致编译失败且无法修复 |
| Design 状态/不变量 | N/A |

**状态所有权和生命周期**

| Design Ref | 状态 | Owner | Key / Index | 创建 | 读取/更新 | 清理触发 | 生命周期不变量 |
|------------|------|-------|-------------|------|-----------|----------|----------------|
| N/A | N/A | N/A | N/A | N/A | N/A | N/A | N/A |

**任务间接口（Produces / Consumes）**

| Direction | Contract | Provider / Consumer | Dependency | Compatibility Constraint |
|-----------|----------|---------------------|------------|--------------------------|
| Produces | `SetAbilityInstanceInfo(label, icon, groupId)` 签名 | TASK-1 | TASK-2 | groupId 为 `const std::string&` 必填参数 |
| Consumes | `ISession::SetSessionLabelAndIcon(label, icon, groupId)` | TASK-1 | 窗口侧（跨仓，已存在） | 窗口侧已支持第三参数 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Modify | `interfaces/kits/native/ability/ability_runtime/ability_context.h` | 新增 groupId 纯虚参数声明 |
| Modify | `interfaces/kits/native/ability/ability_runtime/ability_context_impl.h` | 新增 groupId override 声明 |
| Modify | `frameworks/native/ability/ability_runtime/ability_context_impl.cpp` | 实现透传 groupId 到 SetSessionLabelAndIcon |

**Spec References**

- AC / Rules: `AC-1.1`, `AC-1.2`, `R-1`, `R-3`
- Anchors: `spec.md#验收追溯`, `spec.md#规则定义`

**Design References**

- IDs / Anchors: `ADR-1`, `ADR-4`, `design.md#关键设计决策`, `design.md#代码事实基线`

**Required Rules**

| Rule ID | Must / Must Not |
|---------|-----------------|
| OH-ARCH-LAYERING | Must: SDK 层透传 groupId 至 ISession，不跨层 |
| OH-ARCH-API-LEVEL | Must: groupId 为 Public 参数 |
| OH-ARCH-ERROR-LOG | Must: 复用 TransferSetAbilityInstanceInfoErr 错误码映射 |

**Verification**

| 证据类型 | 命令/路径 | Expected Result | Actual Result |
|----------|-----------|-----------------|---------------|
| 测试 | `SetAbilityInstanceInfo_0100` | SceneBoard 未启用返回 2097230；null session 返回 ERR_INVALID_VALUE；有效 session 不返回 ERR_INVALID_VALUE | Pass（已实现） |
| 测试 | `TransferSetAbilityInstanceInfoErr_0100` | WS_OK→ERR_OK; WS_ERROR_INVALID_PERMISSION→CHECK_PERMISSION_FAILED; WS_ERROR_SET_SESSION_LABEL_FAILED→INVALID_PARAMETERS_ERR; WS_ERROR_DEVICE_NOT_SUPPORT→ERR_CAPABILITY_NOT_SUPPORT | Pass（已实现） |
=======
| Proposal | proposal.md | Approved |
| Spec | spec.md | Approved |
| Design | design.md | Approved |

## 执行原则

- **Spec 权威：** 若实现细节与 `spec.md` 的 AC、错误码或兼容性声明冲突，先更新 spec/design，再继续实现。
- **测试/证据先行：** 每个 Task 先写失败测试；无法单测的集成行为必须先写明可复现证据缺口。
- **任务小型化：** 一个 Task 只覆盖一个独立闭环。
- **文件边界：** Task 只能修改 `Files` 表列出的文件。
- **状态所有权唯一：** 本次无新增状态（groupId 为参数透传）。
- **证据回填：** Task 完成后必须回填本计划「AC 到 Task 追溯」验证状态、「代码范围映射」实际文件、per-task `Actual Result`。
- **反伪完成：** 只补声明、只覆盖 happy path、只跑非相关测试，都不能替代 AC 闭环。

## AC 到 Task 追溯

| AC | 来源 | Task | 验证方式 | 验证状态 |
|----|------|------|----------|----------|
| AC1 | spec.md | TASK-1 | `git show` 检查 C++ 头文件签名含 groupId | Pass |
| AC2 | spec.md | TASK-1 | `git show` 检查 C++ 实现 `SetSessionLabelAndIcon(label, icon, groupId)` | Pass |
| AC3 | spec.md | TASK-2 | `git show` 检查 ETS 两个重载 | Pass |
| AC4 | spec.md | TASK-2, TASK-3 | `git show` 检查 NAPI/ANI groupId 校验 ≤ 64 | Pass |
| AC5 | spec.md | TASK-1 | `git show` 检查 `TransferSetAbilityInstanceInfoErr` 不变 | Pass |
| AC6 | spec.md | TASK-3 | `git show` 检查单测适配 + 编译 | Pass |
| AC7 | spec.md | TASK-2 | `git show` 检查 FA JS 绑定签名 | Pass |

## 实现边界

**必须实现：** C++ SDK 签名变更 + 实现；ETS 新重载；NAPI groupId 可选解析+校验；ANI groupId 必选+校验；FA JS 绑定签名；单测适配。

**可后置：** 无。

**不建议延后：** C++ SDK 签名变更是所有下游的基础，不可延后。

## 禁止项

- 每个 AC 必须有明确的验证方式。
- Agent 不得自行寻找未列出的上下文文件作为修改依据。
- 不得修改 Task 列出范围外的文件。
- 不得在未通过验证时标记 Task 完成。

## Task 依赖

```
T1[C++ SDK + 实现] ──→ T2[ETS + FA 绑定] ──→ T3[NAPI + ANI 绑定 + 单测]
```

## Task 列表

| TASK ID | 目标 | 文件范围 | AC 映射 | 前置依赖 | 完成判据 | 验证命令 | 状态 |
|---------|------|----------|---------|----------|----------|----------|------|
| TASK-1 | C++ SDK 签名 + 实现透传 groupId | ability_context.h, ability_context_impl.h, ability_context_impl.cpp | AC1, AC2, AC5 | - | 签名含 groupId；实现调 SetSessionLabelAndIcon 3 参数 | git show 检查 | Pass |
| TASK-2 | ETS 公共 API 重载 + FA JS 绑定 | UIAbilityContext.ets, ability_context.js | AC3, AC7 | TASK-1 | ETS 两重载；FA JS 签名更新 | git show 检查 | Pass |
| TASK-3 | NAPI + ANI 绑定 + 单测适配 | js_ability_context.cpp, ets_ability_context.h, ets_ability_context.cpp, ability_context_impl_second_test.cpp, ability_context_test.cpp | AC4, AC6 | TASK-1 | NAPI groupId 可选解析+校验；ANI groupId 必选+校验；单测传入 "" | git show 检查 + 编译 | Pass |

## Task 详情

### TASK-1: C++ SDK 签名 + 实现透传 groupId

**目标：** `AbilityContext::SetAbilityInstanceInfo` 签名新增 `const std::string& groupId` 必选参数；实现调用 `SetSessionLabelAndIcon(label, icon, groupId)` 透传。

**AC 映射：** AC1, AC2, AC5

**前置依赖：** 无

**非目标：** ETS/NAPI/ANI 绑定层（TASK-2/3）、单测（TASK-3）。

**状态所有权：** 无（groupId 为参数透传，不存储）。

**任务间接口：** Produces=`SetAbilityInstanceInfo(label, icon, groupId)` C++ 签名（供 TASK-2/3 绑定层调用）；Consumes=Rosen `ISession::SetSessionLabelAndIcon` 3 参数版本（跨仓先行就绪）。

**只读上下文**

| 路径 | 读取目的 |
|------|----------|
| `interfaces/kits/native/ability/ability_runtime/ability_context.h:482-491` | 接口契约 |
| `interfaces/kits/native/ability/ability_runtime/ability_context_impl.h:317-323` | 实现声明 |
| `frameworks/native/ability/ability_runtime/ability_context_impl.cpp:1149-1164` | 实现入口 |
| `frameworks/native/ability/ability_runtime/ability_context_impl.cpp:1691-1704` | 错误码映射 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| 修改 | `interfaces/kits/native/ability/ability_runtime/ability_context.h` | 签名新增 `const std::string& groupId`；Doxygen `@param groupId` |
| 修改 | `interfaces/kits/native/ability/ability_runtime/ability_context_impl.h` | 同步签名变更 |
| 修改 | `frameworks/native/ability/ability_runtime/ability_context_impl.cpp` | 实现新增 groupId 参数；`SetSessionLabelAndIcon(label, icon, groupId)` |

**禁止修改文件**

| 文件/路径 | 原因 |
|-----------|------|
| window_manager 仓 `ISession` 接口 | 跨仓依赖，窗口侧已先行 |

**Steps**

- [x] Step 1: 修改 `ability_context.h` 签名，新增 `const std::string& groupId` 必选参数
- [x] Step 2: 同步修改 `ability_context_impl.h` 签名
- [x] Step 3: 修改 `ability_context_impl.cpp` 实现，`SetSessionLabelAndIcon(label, icon, groupId)` 传 3 参数
- [x] Step 4: 确认 `TransferSetAbilityInstanceInfoErr` 无需变更（AC5）
- [x] 回填 AC 到 Task 追溯 + 代码范围映射 + Actual Result
>>>>>>> Stashed changes

**Anti-Fake Completion**

| Check | Required Evidence |
|-------|-------------------|
<<<<<<< Updated upstream
| AC closed | AC-1.1: `ability_context.h:490` 新增 groupId 参数; `ability_context_impl.cpp:1164` 调用 `SetSessionLabelAndIcon(label, icon, groupId)` — commit `75708f6d89`; AC-1.2: `ability_context_impl.cpp:1153-1154` SceneBoard 检查返回 `ERR_CAPABILITY_NOT_SUPPORT` |
| Scope respected | 实际 diff 仅修改 `ability_context.h`, `ability_context_impl.h`, `ability_context_impl.cpp` — 与 Plan 一致 |
| State lifecycle complete | N/A — 无状态变更 |
| Interface contract matched | C++ SDK 签名 `SetAbilityInstanceInfo(label, icon, groupId)` 与 ISession `SetSessionLabelAndIcon(label, icon, groupId)` 一致 |
| Relevant verification | `SetAbilityInstanceInfo_0100` 和 `TransferSetAbilityInstanceInfoErr_0100` 直接覆盖本 Task |

**Handoff Summary**

| 项 | 内容 |
|----|------|
| 任务描述 | TASK-1：C++ SDK 接口和实现新增 groupId 参数 |
| 允许修改 | `ability_context.h`, `ability_context_impl.h`, `ability_context_impl.cpp` |
| Spec 引用 | `AC-1.1`, `AC-1.2`, `R-1`, `R-3` |
| Design 引用 | `ADR-1`, `ADR-4` |
| 完成规则 | 接口签名含 groupId，实现透传至 SetSessionLabelAndIcon，单测通过 |
=======
| AC closed | AC1: 签名含 groupId; AC2: SetSessionLabelAndIcon 传 3 参数; AC5: 错误码映射不变 |
| Scope respected | 仅修改 ability_context.h / ability_context_impl.h / ability_context_impl.cpp |

**Verification**

| Command / Evidence | Expected Result | Actual Result |
|--------------------|-----------------|---------------|
| `git show c37c143ef4 -- ability_context.h` | 签名含 `const std::string& groupId` | Pass — `ability_context.h:490-491` 含 groupId |
| `git show c37c143ef4 -- ability_context_impl.cpp` | `SetSessionLabelAndIcon(label, icon, groupId)` | Pass — `ability_context_impl.cpp:1164` 传 3 参数 |
| `git show c37c143ef4 -- ability_context_impl.cpp` (错误码映射) | `TransferSetAbilityInstanceInfoErr` 无 diff | Pass — 映射逻辑无变更 |
>>>>>>> Stashed changes

**Review Handoff**

| Reviewer | Input |
|----------|-------|
<<<<<<< Updated upstream
| Spec Compliance | AC-1.1→TASK-1→`ability_context.h:490`+`ability_context_impl.cpp:1164`→commit `75708f6d89`→PASS; AC-1.2→`ability_context_impl.cpp:1153`→PASS |
| Code Quality | 分层合规; 错误码映射复用; 无跨层; 10 files +66/-31 lines |
| Verification | `SetAbilityInstanceInfo_0100` 3 子场景 PASS; `TransferSetAbilityInstanceInfoErr_0100` 5 分支 PASS |

### TASK-2: 绑定层 groupId 解析、校验与 ETS/JS 接口

| 字段 | 内容 |
|------|------|
| 任务目标 | 在 NAPI/ANI/ETS/JS 绑定层新增 groupId 参数解析、长度校验和 ETS 可选参数 |
| AC 映射 | AC-2.1, AC-2.2, AC-3.1 |
| 前置依赖 | TASK-1（C++ SDK 签名已含 groupId） |
| 非目标 | 不在绑定层做业务逻辑 |
| 完成判据 | ETS groupId 可选默认空串；NAPI/ANI 校验 64 字节；超长抛参数错误 |
| 停止条件 | 绑定层参数解析失败 |
| Design 状态/不变量 | N/A |

**状态所有权和生命周期**

| Design Ref | 状态 | Owner | Key / Index | 创建 | 读取/更新 | 清理触发 | 生命周期不变量 |
|------------|------|-------|-------------|------|-----------|----------|----------------|
| N/A | N/A | N/A | N/A | N/A | N/A | N/A | N/A |

**任务间接口（Produces / Consumes）**

| Direction | Contract | Provider / Consumer | Dependency | Compatibility Constraint |
|-----------|----------|---------------------|------------|--------------------------|
| Consumes | `SetAbilityInstanceInfo(label, icon, groupId)` | TASK-2 | TASK-1 | groupId 为 `const std::string&` |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Modify | `frameworks/native/ability/native/ability_runtime/js_ability_context.cpp` | NAPI 绑定 groupId 可选解析与校验 |
| Modify | `interfaces/kits/native/ability/native/ability_runtime/js_ability_context.h` | NAPI 头文件声明 |
| Modify | `frameworks/ets/ani/ui_ability/src/ets_ability_context.cpp` | ANI 绑定 groupId 解析与校验 |
| Modify | `frameworks/ets/ani/ui_ability/include/ets_ability_context.h` | ANI 头文件声明 |
| Modify | `frameworks/ets/ets/application/UIAbilityContext.ets` | ETS 接口 groupId 可选参数 |
| Modify | `frameworks/js/napi/ability_context/ability_context.js` | FA 模型 JS 绑定 groupId 参数 |

**Spec References**

- AC / Rules: `AC-2.1`, `AC-2.2`, `AC-3.1`, `R-2`, `R-4`
- Anchors: `spec.md#验收追溯`, `spec.md#规则定义`

**Design References**

- IDs / Anchors: `ADR-1`, `ADR-2`, `ADR-3`, `design.md#关键设计决策`, `design.md#既有模式复用`

**Required Rules**

| Rule ID | Must / Must Not |
|---------|-----------------|
| OH-ARCH-LAYERING | Must: 绑定层校验后透传至 SDK 层 |
| OH-ARCH-API-LEVEL | Must: ETS groupId 可选，默认空串 |

**Verification**

| 证据类型 | 命令/路径 | Expected Result | Actual Result |
|----------|-----------|-----------------|---------------|
| 编译 | `./build.sh --product-name <product> --build-target ability_runtime` | 编译通过 | Pass（已实现） |
| 静态检查 | groupId 长度校验逻辑存在 | NAPI: `js_ability_context.cpp:3078-3083`; ANI: `ets_ability_context.cpp:810-814` | Pass（已实现） |
=======
| Spec Compliance | AC1/AC2/AC5 覆盖；C++ SDK 签名含 groupId |
| Code Quality | 透传简洁；错误码映射不变；无新增状态 |

### TASK-2: ETS 公共 API 重载 + FA JS 绑定

**目标：** `UIAbilityContext.ets` 新增 `setAbilityInstanceInfo(label, icon, groupId)` 重载；保留原重载内部传 `""`；NAPI AbilityContext 绑定 `ability_context.js` 签名更新。

**AC 映射：** AC3, AC7

**前置依赖：** TASK-1（C++ SDK 签名先行）

**非目标：** NAPI/ANI 绑定层（TASK-3）。

**状态所有权：** 无。

**任务间接口：** Produces=ETS 两个重载（供应用调用）；Consumes=TASK-1 C++ SDK 签名。

**只读上下文**

| 路径 | 读取目的 |
|------|----------|
| `frameworks/ets/ets/application/UIAbilityContext.ets:780-812` | ETS API 入口 |
| `frameworks/js/napi/ability_context/ability_context.js:204` | FA JS 绑定 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| 修改 | `frameworks/ets/ets/application/UIAbilityContext.ets` | native 声明新增 groupId；原重载内部传 `''`；新增 groupId 重载 |
| 修改 | `frameworks/js/napi/ability_context/ability_context.js` | 签名从 `(label, callback)` 变更为 `(label, icon, groupId)` |

**禁止修改文件**

| 文件/路径 | 原因 |
|-----------|------|
| TASK-1 的 C++ 头文件/实现 | 接口契约已定 |

**Steps**

- [x] Step 1: 修改 `UIAbilityContext.ets` 的 `nativeSetAbilityInstanceInfo` 和 `nativeSetAbilityInstanceInfoCheck` native 声明，新增 `groupId: string` 参数
- [x] Step 2: 修改原重载 `setAbilityInstanceInfo(label, icon)` 内部调用传 `''`
- [x] Step 3: 新增重载 `setAbilityInstanceInfo(label, icon, groupId)` 传实际 groupId
- [x] Step 4: 修改 `ability_context.js` 签名为 `(label, icon, groupId)`
- [x] 回填
>>>>>>> Stashed changes

**Anti-Fake Completion**

| Check | Required Evidence |
|-------|-------------------|
<<<<<<< Updated upstream
| AC closed | AC-2.1: ETS `groupId: string = ''` 可选; NAPI `info.argc > ARGC_TWO` 可选 — commit `75708f6d89`; AC-2.2: 空串透传不报错; AC-3.1: NAPI/ANI `groupId.length() > 64` → ThrowInvalidParamError |
| Scope respected | 实际 diff 修改 `js_ability_context.cpp`, `ets_ability_context.cpp/.h`, `UIAbilityContext.ets`, `ability_context.js` — 与 Plan 一致（含必要的头文件更新） |
| State lifecycle complete | N/A — 无状态变更 |
| Interface contract matched | 绑定层调用 `context->SetAbilityInstanceInfo(label, icon, groupId)` 与 TASK-1 Produces 签名一致 |
| Relevant verification | 编译通过; groupId 校验逻辑在源码中确认 |

**Handoff Summary**

| 项 | 内容 |
|----|------|
| 任务描述 | TASK-2：NAPI/ANI/ETS/JS 绑定层 groupId 参数 |
| 允许修改 | `js_ability_context.cpp/.h`, `ets_ability_context.cpp/.h`, `UIAbilityContext.ets`, `ability_context.js` |
| Spec 引用 | `AC-2.1`, `AC-2.2`, `AC-3.1`, `R-2`, `R-4` |
| Design 引用 | `ADR-1`, `ADR-2`, `ADR-3` |
| 完成规则 | ETS 可选默认空串，NAPI/ANI 校验 64 字节，编译通过 |
=======
| AC closed | AC3: ETS 两重载; AC7: FA JS 签名 |
| Scope respected | 仅修改 UIAbilityContext.ets / ability_context.js |

**Verification**

| Command / Evidence | Expected Result | Actual Result |
|--------------------|-----------------|---------------|
| `git show c37c143ef4 -- UIAbilityContext.ets` | 两个重载（:780 原, :797 新） | Pass |
| `git show c37c143ef4 -- ability_context.js` | `setAbilityInstanceInfo(label, icon, groupId)` | Pass |
>>>>>>> Stashed changes

**Review Handoff**

| Reviewer | Input |
|----------|-------|
<<<<<<< Updated upstream
| Spec Compliance | AC-2.1→TASK-2→`UIAbilityContext.ets:780`+`js_ability_context.cpp:3075`→commit `75708f6d89`→PASS; AC-2.2→`UIAbilityContext.ets:780` 默认空串→PASS; AC-3.1→`js_ability_context.cpp:3078`+`ets_ability_context.cpp:810`→PASS |
| Code Quality | 双重校验为防御性设计; NAPI lambda capture by value 线程安全; FA 模型 deprecated 路径 |
| Verification | groupId 可选解析确认; 64 字节校验确认; 编译通过 |

### TASK-3: 单元测试 groupId 场景覆盖

| 字段 | 内容 |
|------|------|
| 任务目标 | 在已有单测中覆盖 groupId 场景 |
| AC 映射 | AC-1.1, AC-1.2 |
| 前置依赖 | TASK-1, TASK-2 |
| 非目标 | 不新增 NAPI/ANI 绑定层测试（由编译保证） |
| 完成判据 | `SetAbilityInstanceInfo_0100` 含 groupId 传入；`TransferSetAbilityInstanceInfoErr_0100` 通过 |
| 停止条件 | 单测编译或运行失败 |
| Design 状态/不变量 | N/A |

**状态所有权和生命周期**

| Design Ref | 状态 | Owner | Key / Index | 创建 | 读取/更新 | 清理触发 | 生命周期不变量 |
|------------|------|-------|-------------|------|-----------|----------|----------------|
| N/A | N/A | N/A | N/A | N/A | N/A | N/A | N/A |

**任务间接口（Produces / Consumes）**

| Direction | Contract | Provider / Consumer | Dependency | Compatibility Constraint |
|-----------|----------|---------------------|------------|--------------------------|
| 无 | 无 | — | — | — |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Modify | `test/unittest/frameworks_kits_ability_ability_runtime_test/ability_context_impl_second_test.cpp` | groupId 场景覆盖 |

**Spec References**

- AC / Rules: `AC-1.1`, `AC-1.2`, `R-1`, `R-3`
- Anchors: `spec.md#验证映射`

**Design References**

- IDs / Anchors: `design.md#代码事实基线`

**Required Rules**

| Rule ID | Must / Must Not |
|---------|-----------------|
| OH-ARCH-ERROR-LOG | Must: 错误码映射测试覆盖所有分支 |

**Verification**

| 证据类型 | 命令/路径 | Expected Result | Actual Result |
|----------|-----------|-----------------|---------------|
| 测试 | `SetAbilityInstanceInfo_0100` | 3 个子场景全部通过（SceneBoard 未启用→2097230；null session→ERR_INVALID_VALUE；有效 session→非 ERR_INVALID_VALUE） | Pass（已实现） |
| 测试 | `TransferSetAbilityInstanceInfoErr_0100` | 5 个错误码映射全部正确 | Pass（已实现） |
=======
| Spec Compliance | AC3/AC7 覆盖；ETS 向后兼容 |
| Code Quality | 重载模式；NAPI 签名修复 |

### TASK-3: NAPI + ANI 绑定 + 单测适配

**目标：** NAPI `JsAbilityContext::OnSetAbilityInstanceInfo` 新增 groupId 可选解析+校验；ANI `EtsAbilityContext` 签名新增 groupId 必选+校验；单测适配新签名。

**AC 映射：** AC4, AC6

**前置依赖：** TASK-1（C++ SDK 签名先行）

**非目标：** ETS API（TASK-2）、C++ SDK（TASK-1）。

**状态所有权：** 无。

**任务间接口：** Produces=NAPI/ANI groupId 解析与校验（供应用调用链）；Consumes=TASK-1 C++ SDK 签名、TASK-2 ETS native 声明。

**只读上下文**

| 路径 | 读取目的 |
|------|----------|
| `frameworks/native/ability/native/ability_runtime/js_ability_context.cpp:3072-3094` | NAPI 入口 |
| `frameworks/ets/ani/ui_ability/include/ets_ability_context.h:163-177` | ANI 接口声明 |
| `frameworks/ets/ani/ui_ability/src/ets_ability_context.cpp:775-815, 2753-2835, 3237-3241` | ANI 实现 |
| `test/unittest/frameworks_kits_ability_ability_runtime_test/ability_context_impl_second_test.cpp:89-104` | 单测入口 |
| `test/unittest/frameworks_kits_ability_ability_runtime_test/ability_context_test.cpp:490` | mock 类 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| 修改 | `frameworks/native/ability/native/ability_runtime/js_ability_context.cpp` | 新增常量 MAX=64；groupId 可选解析+校验；lambda capture |
| 修改 | `frameworks/ets/ani/ui_ability/include/ets_ability_context.h` | 签名新增 groupIdObj（4 处方法） |
| 修改 | `frameworks/ets/ani/ui_ability/src/ets_ability_context.cpp` | 新增常量 MAX=64；Check+OnSet 双重校验；native 注册签名 |
| 修改 | `test/unittest/.../ability_context_impl_second_test.cpp` | 三个子场景传入 `""` |
| 修改 | `test/unittest/.../ability_context_test.cpp` | mock 类签名新增 groupId |

**禁止修改文件**

| 文件/路径 | 原因 |
|-----------|------|
| TASK-1/2 的已修改文件 | 接口契约已定 |

**Steps**

- [x] Step 1: NAPI — 新增 `MAX_ABILITY_INSTANCE_GROUP_ID_LENGTH = 64` 常量
- [x] Step 2: NAPI — 新增 groupId 可选解析逻辑（argc > 2 && type check）
- [x] Step 3: NAPI — lambda capture groupId by value
- [x] Step 4: ANI — header 签名变更（4 处方法 + native 注册签名）
- [x] Step 5: ANI — 实现新增常量 + Check + OnSet 双重校验
- [x] Step 6: 单测 — `ability_context_impl_second_test.cpp` 传入 `""`
- [x] Step 7: 单测 — `ability_context_test.cpp` mock 类签名更新
- [x] 回填
>>>>>>> Stashed changes

**Anti-Fake Completion**

| Check | Required Evidence |
|-------|-------------------|
<<<<<<< Updated upstream
| AC closed | AC-1.1: `SetAbilityInstanceInfo_0100` 传入 groupId 参数（空串）→ 3 子场景 PASS — commit `75708f6d89`; AC-1.2: SceneBoard 未启用→2097230 PASS; `TransferSetAbilityInstanceInfoErr_0100` 5 分支 PASS |
| Scope respected | 实际 diff 修改 `ability_context_impl_second_test.cpp` 和 `ability_context_test.cpp`（mock 签名必须更新）— 与 Plan 一致 |
| State lifecycle complete | N/A — 无状态变更 |
| Interface contract matched | 测试调用 `SetAbilityInstanceInfo(label, icon, "")` 与 TASK-1 签名一致 |
| Relevant verification | `SetAbilityInstanceInfo_0100` 和 `TransferSetAbilityInstanceInfoErr_0100` 直接覆盖本 Task |

**Handoff Summary**

| 项 | 内容 |
|----|------|
| 任务描述 | TASK-3：单测 groupId 场景覆盖 |
| 允许修改 | `ability_context_impl_second_test.cpp` |
| Spec 引用 | `AC-1.1`, `AC-1.2`, `R-1`, `R-3` |
| Design 引用 | `design.md#代码事实基线` |
| 完成规则 | SetAbilityInstanceInfo_0100 和 TransferSetAbilityInstanceInfoErr_0100 通过 |
=======
| AC closed | AC4: NAPI/ANI groupId ≤ 64 校验; AC6: 单测适配新签名 |
| Scope respected | 仅修改 js_ability_context.cpp / ets_ability_context.h/.cpp / 两个测试文件 |

**Verification**

| Command / Evidence | Expected Result | Actual Result |
|--------------------|-----------------|---------------|
| `git show c37c143ef4 -- js_ability_context.cpp` | groupId 可选解析+校验 | Pass — `:3072-3083` |
| `git show c37c143ef4 -- ets_ability_context.cpp` | groupId 必选+双重校验 | Pass — `:810-814, :2828-2834` |
| `git show c37c143ef4 -- ability_context_impl_second_test.cpp` | 传入 `""` | Pass — `:93, :96, :99` |
| `git show c37c143ef4 -- ability_context_test.cpp` | mock 签名含 groupId | Pass — `:490` |
>>>>>>> Stashed changes

**Review Handoff**

| Reviewer | Input |
|----------|-------|
<<<<<<< Updated upstream
| Spec Compliance | AC-1.1→TASK-3→`ability_context_impl_second_test.cpp:89`→commit `75708f6d89`→PASS; AC-1.2→`ability_context_impl_second_test.cpp:92`→PASS |
| Code Quality | 测试覆盖正常/异常/边界; 错误码映射全分支; mock 签名同步更新 |
| Verification | `SetAbilityInstanceInfo_0100` 3 子场景 PASS; `TransferSetAbilityInstanceInfoErr_0100` 5 分支 PASS |

## 代码范围映射

| Task ID | 计划文件/符号 | 实际文件/符号 | 操作 | Commit | Spec Compliance | Code Quality | Verification | 偏差 |
|---------|---------------|---------------|------|--------|-----------------|--------------|--------------|------|
| TASK-1 | `ability_context.h:490`, `ability_context_impl.cpp:1149` | `ability_context.h:490`, `ability_context_impl.cpp:1149`, `ability_context_impl.h:323` | Modify | `75708f6d89` | `evidence/reviews/spec-compliance.md#TASK-1` | `evidence/reviews/code-quality.md#TASK-1` | `evidence/reviews/verification.md#TASK-1` | 无 |
| TASK-2 | `js_ability_context.cpp:3075`, `ets_ability_context.cpp:809`, `UIAbilityContext.ets:780` | `js_ability_context.cpp:3075`, `ets_ability_context.cpp:777`, `UIAbilityContext.ets:780`, `ability_context.js:204`, `ets_ability_context.h:165` | Modify | `75708f6d89` | `evidence/reviews/spec-compliance.md#TASK-2` | `evidence/reviews/code-quality.md#TASK-2` | `evidence/reviews/verification.md#TASK-2` | 无 |
| TASK-3 | `ability_context_impl_second_test.cpp:89` | `ability_context_impl_second_test.cpp:89`, `ability_context_test.cpp:490` | Modify | `75708f6d89` | `evidence/reviews/spec-compliance.md#TASK-3` | `evidence/reviews/code-quality.md#TASK-3` | `evidence/reviews/verification.md#TASK-3` | 无 |

## 实现偏差和开放问题

| Task ID | 类型 | 问题/偏差 | 处理状态 | 回退能力/Owner |
|---------|------|-----------|----------|----------------|
| 无 | — | — | — | — |
=======
| Spec Compliance | AC4/AC6 覆盖；NAPI 可选 + ANI 必选 |
| Code Quality | lambda capture by value；双重校验防御性设计 |

## Review Gates

| Gate | When | Required Evidence | Blocks Next Step |
|------|------|-------------------|------------------|
| Gate-1 | TASK-1 完成后 | C++ 签名含 groupId；SetSessionLabelAndIcon 3 参数；错误码映射不变 | 是 |
| Gate-Final | TASK-3 完成后 | 全部 AC 证据；代码范围映射；Actual Result 回填 | 是 |

## 代码范围映射

| TASK ID | 文件 | 操作 |
|---------|------|------|
| TASK-1 | `interfaces/kits/native/ability/ability_runtime/ability_context.h:490-491` | 修改 |
| TASK-1 | `interfaces/kits/native/ability/ability_runtime/ability_context_impl.h:323` | 修改 |
| TASK-1 | `frameworks/native/ability/ability_runtime/ability_context_impl.cpp:1149-1164` | 修改 |
| TASK-2 | `frameworks/ets/ets/application/UIAbilityContext.ets:173-174, 236, 780-812` | 修改 |
| TASK-2 | `frameworks/js/napi/ability_context/ability_context.js:204-205` | 修改 |
| TASK-3 | `frameworks/native/ability/native/ability_runtime/js_ability_context.cpp:88, 3072-3094` | 修改 |
| TASK-3 | `frameworks/ets/ani/ui_ability/include/ets_ability_context.h:166-177` | 修改 |
| TASK-3 | `frameworks/ets/ani/ui_ability/src/ets_ability_context.cpp:62, 775-815, 2753-2835, 3237-3241` | 修改 |
| TASK-3 | `test/unittest/frameworks_kits_ability_ability_runtime_test/ability_context_impl_second_test.cpp:93-99` | 修改 |
| TASK-3 | `test/unittest/frameworks_kits_ability_ability_runtime_test/ability_context_test.cpp:490` | 修改 |

## 实施完成定义 (DoD)

- [x] TASK-1~TASK-3 所有代码已写入（commit `c37c143ef4`）
- [x] 编译通过
- [x] 单元测试通过（`ability_context_impl_second_test`、`ability_context_test`）
- [x] AC1~AC7 全部通过
- [x] evidence/reviews/ 三份审查完成
>>>>>>> Stashed changes

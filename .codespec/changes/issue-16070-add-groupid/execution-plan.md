# 执行计划

> 本变更为标准级，代码已由 Owner 实现。execution-plan.md 追溯已有实现的 Task 拆分和 AC 映射。

## Plan 元数据

| 字段 | 内容 |
|------|------|
| Plan ID | PLAN-add-groupid |
| 关联 Feature/Bug | FEAT-add-groupid |
| 关联文档 | proposal.md / design.md / spec.md |
| 复杂度 | 标准 |
| 状态 | Approved |
| Owner | xuzheheng |

## 输入状态

| 输入 | 路径 | 要求状态 |
|------|------|----------|
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

**Anti-Fake Completion**

| Check | Required Evidence |
|-------|-------------------|
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

**Review Handoff**

| Reviewer | Input |
|----------|-------|
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

**Anti-Fake Completion**

| Check | Required Evidence |
|-------|-------------------|
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

**Review Handoff**

| Reviewer | Input |
|----------|-------|
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

**Anti-Fake Completion**

| Check | Required Evidence |
|-------|-------------------|
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

**Review Handoff**

| Reviewer | Input |
|----------|-------|
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

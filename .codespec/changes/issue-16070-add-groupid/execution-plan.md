# 执行计划

> 将 Approved Spec 拆成可独立执行、可验证、可审查的 Task。每个 Task 自包含，交接信息不依赖文件路径引用。

## Plan 元数据

| 字段 | 内容 |
|------|------|
| Plan ID | issue-16070-add-groupid |
| 关联 Feature/Bug | issue-16070-add-groupid |
| 关联文档 | proposal.md / design.md / spec.md |
| 复杂度 | 标准 |
| 状态 | Approved |
| PR | https://gitcode.com/openharmony/ability_ability_runtime/pull/20307 |
| Owner | xuzheheng |

## 输入状态

| 输入 | 路径 | 要求状态 |
|------|------|----------|
| Requirement | `proposal.md` | Approved |
| Design | `design.md` | Approved |
| Spec | `spec.md` | Approved |

## AC 到 Task 追溯

| AC | 来源 | Task | 验证方式 | 覆盖？ |
|----|------|------|----------|--------|
| AC-1.1 | spec.md | TASK-1, TASK-2 | 单测 | 是 |
| AC-1.2 | spec.md | TASK-2, TASK-3 | 单测 | 是 |
| AC-1.3 | spec.md | TASK-2 | 单测 | 是 |
| AC-1.4 | spec.md | TASK-3 | 单测 | 是 |
| AC-2.1 | spec.md | TASK-2 | 单测 | 是 |
| AC-3.1 | spec.md | TASK-1 | 单测（回归） | 是 |
| AC-3.2 | spec.md | TASK-1 | 单测（回归） | 是 |
| AC-3.3 | spec.md | TASK-1 | 单测（回归） | 是 |
| AC-3.4 | spec.md | TASK-1 | 单测（回归） | 是 |
| AC-3.5 | spec.md | TASK-1 | 单测（回归） | 是 |

## 首批实现边界

**首批必须实现：** C++ SDK 签名变更（TASK-1），绑定层和 ETS 重载依赖 SDK 签名
**可后置：** 无
**不建议延后：** C++ SDK 签名变更是所有下游的基础，不可延后

## Task 列表

| Task ID | 目标 | 文件范围 | AC 映射 | 前置依赖 | 完成判据 | 验证命令 | 状态 |
|---------|------|----------|---------|----------|----------|----------|------|
| TASK-1 | C++ SDK 签名变更 + groupId 透传 + 错误码映射不变 | `ability_context.h`, `ability_context_impl.h`, `ability_context_impl.cpp` | AC-1.1, AC-3.1~3.5 | 无 | 签名含 groupId；透传至 SetSessionLabelAndIcon；错误码映射不变 | `SetAbilityInstanceInfo_0100` + `TransferSetAbilityInstanceInfoErr_0100` | Done |
| TASK-2 | ETS 公共 API 重载 + NAPI AbilityContext 签名 | `UIAbilityContext.ets`, `ability_context.js` | AC-1.1, AC-1.3, AC-2.1 | TASK-1 | ETS 两重载；原重载传空串；NAPI 签名更新 | 编译通过 | Done |
| TASK-3 | NAPI + ANI 绑定 groupId 解析校验 + 单测适配 | `js_ability_context.cpp`, `ets_ability_context.h`, `ets_ability_context.cpp`, `ability_context_impl_second_test.cpp`, `ability_context_test.cpp` | AC-1.2, AC-1.4 | TASK-1 | NAPI groupId 可选解析+64 字节校验；ANI groupId 必选+64 字节校验；单测传空串 | 编译通过 + 单测 | Done |

## Task 详情

### TASK-1: C++ SDK 签名变更 + groupId 透传

| 字段 | 内容 |
|------|------|
| 任务目标 | `SetAbilityInstanceInfo` 新增 groupId 必选参数，透传至窗口 `SetSessionLabelAndIcon`，错误码映射不变 |
| AC 映射 | AC-1.1, AC-3.1, AC-3.2, AC-3.3, AC-3.4, AC-3.5 |
| 前置依赖 | 无 |
| 非目标 | 不在 C++ 层做 groupId 长度校验（在绑定层） |
| 完成判据 | 签名含 groupId；实现透传 groupId；错误码映射 5 分支不变 |
| 停止条件 | 接口签名变更导致编译失败且无法修复 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Modify | `interfaces/kits/native/ability/ability_runtime/ability_context.h` | 签名新增 `const std::string& groupId`；Doxygen `@param groupId` |
| Modify | `interfaces/kits/native/ability/ability_runtime/ability_context_impl.h` | 同步签名变更 |
| Modify | `frameworks/native/ability/ability_runtime/ability_context_impl.cpp` | 实现透传 groupId 到 `SetSessionLabelAndIcon` |

**Spec Context**

- AC-1.1: WHEN 调用 `setAbilityInstanceInfo(label, icon, groupId)` 传入合法 groupId THEN Promise resolve
- AC-3.1~3.5: 异常路径错误码（801/16000050/16000011/201/401）经错误码映射不变
- BR-1: groupId ≤ 64 字节正常执行
- 架构约束: groupId 透传到窗口 session，不解释语义

**Design Context**

- ADR-2: C++ SDK groupId 为必选参数（无默认值）
- 调用链: AbilityContextImpl → ifaceSession->SetSessionLabelAndIcon(label, icon, groupId)
- 错误码映射 TransferSetAbilityInstanceInfoErr 不变（5 分支 + default 透传）

**Required Rules**

| Rule ID | Must / Must Not |
|---------|-----------------|
| 架构约束 | Must: SDK 层透传 groupId 至 ISession，不跨层 |
| 架构约束 | Must: 复用既有错误码映射，不新增 |
| 架构约束 | Must Not: C++ 层不做 groupId 长度校验 |

**Steps**

- [x] 修改 `ability_context.h` 签名，新增 `const std::string& groupId`
- [x] 同步修改 `ability_context_impl.h`
- [x] 修改 `ability_context_impl.cpp`，`SetSessionLabelAndIcon(label, icon, groupId)` 传 3 参数
- [x] 确认错误码映射 `TransferSetAbilityInstanceInfoErr` 无需变更

**Completion Evidence**

| 证据类型 | 命令/路径 | 结果 |
|----------|-----------|------|
| 单测 | `SetAbilityInstanceInfo_0100`（3 子场景） | PASS |
| 单测 | `TransferSetAbilityInstanceInfoErr_0100`（5 分支） | PASS |
| PR | https://gitcode.com/openharmony/ability_ability_runtime/pull/20307 | TASK-1 实现 |

**Handoff Summary**

| 项 | 内容 |
|----|------|
| 允许修改 | `ability_context.h`, `ability_context_impl.h`, `ability_context_impl.cpp` |
| Spec 摘要 | AC-1.1 正常路径 + AC-3.1~3.5 异常路径错误码不变 |
| Design 摘要 | ADR-2 groupId 必选无默认值；透传至 ISession；错误码映射不变 |
| 完成规则 | 不得修改允许范围外的文件；无 fresh evidence 不得声明完成 |

---

### TASK-2: ETS 公共 API 重载 + NAPI AbilityContext 签名

| 字段 | 内容 |
|------|------|
| 任务目标 | ETS 新增 groupId 必选重载，原重载内部传空串；NAPI AbilityContext 签名更新 |
| AC 映射 | AC-1.1, AC-1.3, AC-2.1 |
| 前置依赖 | TASK-1（C++ SDK 签名先行） |
| 非目标 | NAPI/ANI 绑定层实现（TASK-3） |
| 完成判据 | ETS 两个重载；原重载传 `''`；NAPI 签名从 `(label, callback)` 变更为 `(label, icon, groupId)` |
| 停止条件 | ETS 重载编译失败 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Modify | `frameworks/ets/ets/application/UIAbilityContext.ets` | native 声明新增 groupId；原重载传 `''`；新增 groupId 重载 |
| Modify | `frameworks/js/napi/ability_context/ability_context.js` | 签名从 `(label, callback)` 变更为 `(label, icon, groupId)` |

**Spec Context**

- AC-1.1: WHEN 调用 `setAbilityInstanceInfo(label, icon, groupId)` THEN Promise resolve
- AC-1.3: WHEN groupId 恰好 64 字节 THEN Promise resolve
- AC-2.1: WHEN 调用 `setAbilityInstanceInfo(label, icon)` 不传 groupId THEN Promise resolve，等价于 groupId=""
- BR-4: 原重载不传 groupId 时等价于 groupId=""

**Design Context**

- ADR-1: ETS 采用重载而非默认参数
- 原重载内部传空串实现向后兼容
- NAPI 签名 breaking change（原签名已损坏）

**Required Rules**

| Rule ID | Must / Must Not |
|---------|-----------------|
| 架构约束 | Must: ETS 原重载保留，内部传空串 |
| 架构约束 | Must: NAPI 签名从 `(label, callback)` 变更为 `(label, icon, groupId)` |

**Steps**

- [x] 修改 `UIAbilityContext.ets` native 声明新增 groupId 参数
- [x] 原重载内部传 `''`
- [x] 新增 `setAbilityInstanceInfo(label, icon, groupId)` 重载
- [x] 修改 `ability_context.js` 签名

**Completion Evidence**

| 证据类型 | 命令/路径 | 结果 |
|----------|-----------|------|
| 编译 | `./build.sh --product-name <product> --build-target ability_runtime` | PASS |
| PR | https://gitcode.com/openharmony/ability_ability_runtime/pull/20307 | TASK-2 实现 |

**Handoff Summary**

| 项 | 内容 |
|----|------|
| 允许修改 | `UIAbilityContext.ets`, `ability_context.js` |
| Spec 摘要 | AC-1.1 正常路径 + AC-1.3 边界 + AC-2.1 向后兼容 |
| Design 摘要 | ADR-1 重载模式；原重载传空串；NAPI 签名修复 |
| 完成规则 | 不得修改允许范围外的文件；无 fresh evidence 不得声明完成 |

---

### TASK-3: NAPI + ANI 绑定 groupId 解析校验 + 单测适配

| 字段 | 内容 |
|------|------|
| 任务目标 | NAPI groupId 可选解析+64 字节校验；ANI groupId 必选+64 字节校验；单测适配新签名 |
| AC 映射 | AC-1.2, AC-1.4 |
| 前置依赖 | TASK-1（C++ SDK 签名先行） |
| 非目标 | ETS API（TASK-2）、C++ SDK（TASK-1） |
| 完成判据 | NAPI argc 检测+长度校验；ANI 必选+双重校验；单测传入空串 |
| 停止条件 | 绑定层参数解析编译失败 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Modify | `frameworks/native/ability/native/ability_runtime/js_ability_context.cpp` | 新增常量 MAX=64；groupId 可选解析+校验；lambda capture |
| Modify | `frameworks/ets/ani/ui_ability/include/ets_ability_context.h` | 签名新增 groupIdObj（4 处方法） |
| Modify | `frameworks/ets/ani/ui_ability/src/ets_ability_context.cpp` | 新增常量 MAX=64；Check+OnSet 双重校验；native 注册签名 |
| Modify | `test/unittest/.../ability_context_impl_second_test.cpp` | 三个子场景传入 `""` |
| Modify | `test/unittest/.../ability_context_test.cpp` | mock 类签名新增 groupId |

**Spec Context**

- AC-1.2: WHEN groupId 长度 > 64 字节 THEN 同步抛出 401
- AC-1.4: WHEN NAPI 第三参数为非 string THEN groupId 视为空串，Promise resolve
- BR-2: groupId > 64 字节同步抛出 401
- BR-3: NAPI 非 string 类型降级为空串
- EX-1: 401 同步抛出，不进入 Promise

**Design Context**

- ADR-3: NAPI groupId 可选解析（argc 检测）
- ADR-4: ANI groupId 为必选参数
- ADR-5: 64 字节校验在绑定层强制
- ADR-6: ANI 双重校验（Check + OnSet）

**Required Rules**

| Rule ID | Must / Must Not |
|---------|-----------------|
| 架构约束 | Must: NAPI groupId 可选解析，argc ≤ 2 时默认空串 |
| 架构约束 | Must: ANI groupId 必选 |
| 架构约束 | Must: 超长 64 字节同步抛出 401 |
| 架构约束 | Must Not: 绑定层不调用 C++ 实现当 groupId 超长 |

**Steps**

- [x] NAPI 新增 `MAX_ABILITY_INSTANCE_GROUP_ID_LENGTH = 64`
- [x] NAPI groupId 可选解析（argc > 2 && type check）
- [x] NAPI lambda capture groupId by value
- [x] ANI header 签名变更（4 处方法 + native 注册签名）
- [x] ANI 实现常量 + Check + OnSet 双重校验
- [x] 单测 `ability_context_impl_second_test.cpp` 传入 `""`
- [x] 单测 `ability_context_test.cpp` mock 签名更新

**Completion Evidence**

| 证据类型 | 命令/路径 | 结果 |
|----------|-----------|------|
| 编译 | `./build.sh --product-name <product> --build-target ability_runtime` | PASS |
| 单测 | `ability_context_impl_second_test` | PASS |
| PR | https://gitcode.com/openharmony/ability_ability_runtime/pull/20307 | TASK-3 实现 |

**Handoff Summary**

| 项 | 内容 |
|----|------|
| 允许修改 | `js_ability_context.cpp`, `ets_ability_context.h`, `ets_ability_context.cpp`, `ability_context_impl_second_test.cpp`, `ability_context_test.cpp` |
| Spec 摘要 | AC-1.2 groupId 超长同步抛出 401 + AC-1.4 NAPI 非 string 降级 |
| Design 摘要 | ADR-3 NAPI 可选 + ADR-4 ANI 必选 + ADR-5 64 字节 + ADR-6 双重校验 |
| 完成规则 | 不得修改允许范围外的文件；无 fresh evidence 不得声明完成 |

## Plan 自审清单

- [x] 每个 P0/P1 AC 至少映射到一个 Task
- [x] 每个 Task 文件范围明确
- [x] 每个 Task 明确前置依赖、非目标、完成判据和停止条件
- [x] 每个 Task 有验证命令
- [x] Task 粒度形成能力闭环
- [x] 没有 TBD/TODO/占位符
- [x] 没有要求 Agent 自行寻找未列出的上下文
- [x] 交接信息自包含（Handoff Summary 完整）
- [x] 每个 Task 验证在完成时立即执行并记录证据
- [x] 超 3000 行阈值的 Task 已拆分或已说明不拆分理由

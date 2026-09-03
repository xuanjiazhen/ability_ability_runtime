# 架构设计

> 本变更为标准级，代码已由 Owner 实现。design.md 记录已验证的代码事实、架构约束和设计决策。

## 设计元数据

| 字段 | 内容 |
|------|------|
| Design ID | DESIGN-add-groupid |
| 关联需求 | `.codespec/changes/issue-16070-add-groupid/proposal.md` |
| 关联 Epic | 无 |
| 目标 Feature | FEAT-add-groupid |
| 复杂度 | 标准 |
| 目标版本 | OpenHarmony-6.0-Release (API 26.1.0+) |
| Owner | xuzheheng |
| 状态 | Approved |

## 需求基线

| 项 | 补充说明 |
|----|----------|
| groupId 为可选参数 | ETS 层默认空串 `''`，C++ SDK 层为必填（无默认值），绑定层处理可选性 |
| 不涉及窗口侧修改 | `ISession::SetSessionLabelAndIcon` 已支持 groupId 第三参数 |
| 代码已实现 | 本次 design 为追溯性文档，记录已有实现的设计决策 |

## 上下文和现状

### 涉及仓和模块

| 仓库 | 补充架构说明 |
|------|-------------|
| ability_runtime | 本仓涵盖公共 SDK 接口（interfaces/kits/）、框架实现（frameworks/）、绑定层（NAPI/ANI/ETS/JS）。服务层（services/）不涉及此变更。groupId 通过 `ISession` IPC 接口透传至窗口管理器（Rosen），窗口侧已支持。 |

### 代码事实基线

| 代码引用（文件:行/符号） | 已验证事实 | 关联架构规则 | 对本设计的约束 | 验证来源/可信度 |
|---|---|---|---|---|
| `ability_context.h:490 / AbilityContext::SetAbilityInstanceInfo` | C++ 公共 SDK 纯虚接口，签名 `(label, icon, groupId)`，第三参数 `const std::string& groupId` | OH-ARCH-API-LEVEL | groupId 为 Public API 参数，签名变更需兼容性评估 | 源码，高 |
| `ability_context_impl.cpp:1149-1186 / AbilityContextImpl::SetAbilityInstanceInfo` | 实现流程：SceneBoard 检查 → ISession 获取 → `SetSessionLabelAndIcon(label, icon, groupId)` → 错误码转换 → 成功时回调 `SetMissionLabel/SetMissionIcon` | OH-ARCH-LAYERING | 实现层不校验 groupId 长度，仅透传至窗口；长度校验在绑定层 | 源码，高 |
| `ability_context_impl.cpp:1691-1704 / TransferSetAbilityInstanceInfoErr` | 错误码映射：`WS_OK→ERR_OK`, `WS_ERROR_INVALID_PERMISSION→CHECK_PERMISSION_FAILED`, `WS_ERROR_SET_SESSION_LABEL_FAILED→INVALID_PARAMETERS_ERR`, `WS_ERROR_DEVICE_NOT_SUPPORT→ERR_CAPABILITY_NOT_SUPPORT` | OH-ARCH-ERROR-LOG | 错误码转换必须保持映射一致，不得新增映射项 | 源码，高 |
| `js_ability_context.cpp:88 / MAX_ABILITY_INSTANCE_GROUP_ID_LENGTH` | NAPI 绑定层定义 `constexpr size_t = 64` | OH-ARCH-API-LEVEL | groupId 长度上限 64 字节，NAPI 层校验 | 源码，高 |
| `js_ability_context.cpp:3075-3083 / OnSetAbilityInstanceInfo` | NAPI 绑定层：groupId 可选（`info.argc > ARGC_TWO && CheckTypeForNapiValue(napi_string)`），超长时 `ThrowInvalidParamError` | OH-ARCH-LAYERING | NAPI 层 groupId 为可选参数，类型检查为 napi_string | 源码，高 |
| `ets_ability_context.cpp:62 / MAX_ABILITY_INSTANCE_GROUP_ID_LENGTH` | ANI 绑定层定义 `constexpr size_t = 64` | OH-ARCH-API-LEVEL | ANI 层 groupId 长度上限 64 字节，与 NAPI 一致 | 源码，高 |
| `ets_ability_context.cpp:809-814 / SetAbilityInstanceInfoCheck` | ANI 绑定层：groupId 必须解析成功且 ≤64 字节，否则 `ThrowInvalidParamError` | OH-ARCH-LAYERING | ANI 层 groupId 校验在 Check 函数中，先于异步执行 | 源码，高 |
| `UIAbilityContext.ets:780 / setAbilityInstanceInfo` | ETS 接口：`groupId: string = ''`（可选参数，默认空串） | OH-ARCH-API-LEVEL | ETS 层 groupId 可选，向后兼容 | 源码，高 |
| `ability_context.js:204 / setAbilityInstanceInfo` | FA 模型 JS 绑定：`setAbilityInstanceInfo(label, icon, groupId)` | OH-ARCH-API-LEVEL | FA 模型 groupId 为第三参数 | 源码，高 |

### 调用链层级分析

| 层 | 模块 | 职责 | 修改类型 |
|----|------|------|----------|
| ETS 接口层 | `UIAbilityContext.ets` | Stage 模型公共 ETS API 声明 | 修改（新增可选参数） |
| ANI 绑定层 | `ets_ability_context.cpp` | ArkTS 原生绑定：参数解析、校验、异步调度 | 修改（新增 groupId 解析与校验） |
| NAPI 绑定层 | `js_ability_context.cpp` | FA 模型 JS 原生绑定：参数解析、校验、异步调度 | 修改（新增 groupId 解析与校验） |
| JS 绑定层 | `ability_context.js` | FA 模型 JS 接口声明 | 修改（新增 groupId 参数） |
| C++ SDK 接口层 | `ability_context.h` | 公共 C++ 纯虚接口声明 | 修改（新增第三参数） |
| C++ SDK 实现层 | `ability_context_impl.cpp` | 实现：SceneBoard 检查 → ISession 透传 → 错误码转换 | 修改（新增 groupId 透传） |
| 窗口 IPC 层 | `ISession`（跨仓，Rosen） | 窗口侧接收 groupId | 不修改（已支持） |

**检查项：**
- [x] 调用链每一层都已覆盖（从 ETS/JS 到 C++ SDK 到窗口 IPC）
- [x] 每层职责边界清晰，无跨层违规调用
- [x] 每层修改类型明确

### 适用架构规则

| Rule ID | 适用原因 | 设计结论 | 验证方式 |
|---------|----------|----------|----------|
| OH-ARCH-LAYERING | ETS/JS → NAPI/ANI → C++ SDK → ISession IPC | 调用方向：应用层 → 框架层 → IPC → 窗口服务。无反向依赖 | 代码评审/依赖检查 |
| OH-ARCH-API-LEVEL | 涉及 Public API 签名变更 | groupId 为 Public 参数，标注 `@since 26.1.0`；ETS 可选保兼容 | API 评审/XTS |
| OH-ARCH-ERROR-LOG | 涉及错误码映射 | 复用已有 `TransferSetAbilityInstanceInfoErr` 映射，不新增错误码 | 单测/hilog |
| OH-ARCH-IPC-SAF | 涉及 ISession IPC 透传 | groupId 通过已有 IPC 通道透传，无新增 IPC 接口 | 集成测试 |

### 既有模式复用

| 模式 | 参考代码（文件:行/符号） | 适用架构规则 | 必须复用的约定 | 本设计的复用方式 | 偏差及理由 | 关联 Task |
|------|---|---|---|---|---|---|
| 错误码转换模式 | `ability_context_impl.cpp:1691 / TransferSetAbilityInstanceInfoErr` | OH-ARCH-ERROR-LOG | WSError→ability 错误码映射表 | 直接复用，不新增映射项 | 无 | TASK-1 |
| 绑定层参数校验模式 | `js_ability_context.cpp:3075 / groupId 可选解析` | OH-ARCH-LAYERING | argc 检查 + 类型检查 + 长度校验 | NAPI 和 ANI 绑定层采用相同模式 | 无 | TASK-2 |

## 不涉及项承接

| 维度 | 设计结论 |
|------|----------|
| 兼容性 | ETS 层 groupId 可选（默认空串），向后兼容；C++ SDK 层 groupId 必填，已有调用方需适配 |
| API/SDK | Public API 签名变更，groupId 标注 `@since 26.1.0` |

## 关键设计决策

| 决策 ID | 问题 | 推荐方案 | 探索过的替代方案 | 取舍理由 | 影响 |
|---------|------|----------|-----------------|------|------|
| ADR-1 | groupId 在哪一层做长度校验 | 在 NAPI/ANI 绑定层校验 | 备选1: 在 C++ SDK 层校验（放弃：SDK 层为纯透传，不应承担参数校验职责）；备选2: 在窗口侧校验（放弃：窗口侧无法区分空串和超长，错误归因不清晰） | 绑定层是最接近应用的层，能提供精确的错误信息；SDK 层保持透传职责单一 | C++ SDK 层不校验，依赖绑定层 |
| ADR-2 | groupId 在 ETS 中是否可选 | 可选，默认空串 `''` | 备选1: 必填（放弃：破坏向后兼容）；备选2: 使用 undefined 哨兵值（放弃：增加复杂度） | 空串表示"不指定分组"，语义清晰；向后兼容 | ETS 调用方不传 groupId 时行为不变 |
| ADR-3 | groupId 长度上限设为多少 | 64 字节 (`MAX_ABILITY_INSTANCE_GROUP_ID_LENGTH`) | 备选1: 32 字节（放弃：可能不够用）；备选2: 128 字节（放弃：过长可能影响 IPC 性能） | 64 字节与 label 长度限制一致，合理且一致 | 超过 64 字节返回参数错误 |
| ADR-4 | C++ SDK 层 groupId 是否给默认值 | 不给默认值（必填） | 备选1: 给默认空串（放弃：C++ 接口为纯虚，默认值需在实现层，破坏接口一致性） | 保持接口签名一致性，C++ 调用方显式传参 | C++ 调用方需适配新参数 |

## 后续 Task 拆分

| Task ID | 目标 | 受影响文件 | 依赖 |
|---------|------|------------|------|
| TASK-1 | C++ SDK 接口与实现层 groupId 参数 | `ability_context.h`, `ability_context_impl.h`, `ability_context_impl.cpp` | design.md + spec.md Approved |
| TASK-2 | 绑定层（NAPI/ANI/ETS/JS）groupId 参数解析与校验 | `js_ability_context.cpp`, `ets_ability_context.cpp`, `UIAbilityContext.ets`, `ability_context.js` | TASK-1 完成 |
| TASK-3 | 单元测试 groupId 场景覆盖 | `ability_context_impl_second_test.cpp` | TASK-1, TASK-2 完成 |

## API 签名、Kit 与权限

### 变更/废弃 API

| 原有 API | 变更类型 | 新 API | 迁移说明 |
|----------|----------|--------|----------|
| `SetAbilityInstanceInfo(label, icon)` | 变更（新增参数） | `SetAbilityInstanceInfo(label, icon, groupId)` | C++ 调用方需补传 groupId 或空串 |
| `setAbilityInstanceInfo(label, icon)` (ETS) | 变更（新增可选参数） | `setAbilityInstanceInfo(label, icon, groupId = '')` | 向后兼容，无需迁移 |

## 构建系统影响

### BUILD.gn 变更

无新增源文件，所有修改均为已有文件内变更。BUILD.gn 无需更新。

### bundle.json 变更

无新增依赖或部件。bundle.json 无需更新。

## 风险和开放问题

| 项 | 类型 | 影响 | 处理方式 | Owner |
|----|------|------|----------|-------|
| C++ SDK 层缺 groupId 长度校验 | 技术 | 低 | 绑定层已做 64 字节限制；SDK 层为透传，设计如此 | xuzheheng |

## 设计审批

- [x] 需求基线已确认，设计覆盖 P0/P1 AC
- [x] 不涉及项已承接，N/A 和展开项都有结论
- [x] 涉及仓和模块职责清楚
- [x] 调用链层级分析完整，每层覆盖到位
- [x] 涉及既有实现时，代码事实基线使用文件:行/符号引用，且每项事实已转化为设计约束
- [x] 已检索并记录既有模式复用
- [x] 适用架构规则已识别并形成设计结论
- [x] 分层和子系统边界合规
- [x] API 变更有签名、权限、错误码和兼容性说明
- [x] BUILD.gn/bundle.json 影响明确（无影响）
- [x] 设计输出和后续 Task 拆分明确
- [x] 关键设计决策有理由和影响说明
- [x] 风险和开放问题有 Owner

**结论:** 通过

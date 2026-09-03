# 特性规格

> 固定用户可见行为和验收标准。

## 概述

| 属性 | 值 |
|------|-----|
| 特性名称 | setAbilityInstanceInfo 新增 groupId 可选参数并透传至窗口 |
| 特性编号 | FEAT-add-groupid |
| 所属 Epic | 无 |
| 优先级 | P1 |
| 目标版本 | OpenHarmony-6.0-Release (API 26.1.0+) |
| SIG 归属 | SIG_Ability |
| 状态 | Approved |
| 复杂度 | 标准 |

## 本次变更范围（Delta）

| 类型 | 内容 | 说明 |
|------|------|------|
| MODIFIED | `AbilityContext::SetAbilityInstanceInfo` 新增第三参数 `groupId` | C++ Public API 签名变更，新增可选参数 |
| MODIFIED | `setAbilityInstanceInfo` (ETS) 新增可选参数 `groupId` | ETS Public API 签名变更，默认空串，向后兼容 |
| MODIFIED | NAPI/ANI 绑定层 groupId 参数解析与长度校验 | 绑定层新增 groupId 解析，超长返回错误 |

## 输入文档

| 文档 | 路径 | 状态 |
|------|------|------|
| Requirement | `.codespec/changes/issue-16070-add-groupid/proposal.md` | Approved |

## 用户故事

### US-1: 传入 groupId 实现实例分组

**作为** 应用开发者,
**我想要** 在调用 `setAbilityInstanceInfo` 时传入 groupId,
**以便** 窗口侧能基于分组标识管理 ability 实例.

**验收标准：**

| AC编号 | 验收标准 | 可观察表面 | 类型 |
|--------|----------|--------------|------|
| AC-1.1 | WHEN 应用调用 `setAbilityInstanceInfo(label, icon, groupId)` 传入非空 groupId THEN `SetAbilityInstanceInfo` 返回 `ERR_OK`（0），Promise resolve | Public API | 正常 |
| AC-1.2 | WHEN 设备未启用 SceneBoard THEN `setAbilityInstanceInfo` 返回错误码 `2097230`（ERR_CAPABILITY_NOT_SUPPORT），Promise reject | Public API | 异常 |

### US-2: groupId 可选保持向后兼容

**作为** 应用开发者,
**我想要** groupId 为可选参数，不传时行为不变,
**以便** 已有代码无需修改.

**验收标准：**

| AC编号 | 验收标准 | 可观察表面 | 类型 |
|--------|----------|--------------|------|
| AC-2.1 | WHEN 应用调用 `setAbilityInstanceInfo(label, icon)` 不传 groupId（ETS 默认空串） THEN `SetAbilityInstanceInfo` 正常执行返回 `ERR_OK`，行为与变更前一致 | Public API | 边界 |
| AC-2.2 | WHEN 应用传入 groupId 为空串 THEN `SetAbilityInstanceInfo` 正常执行返回 `ERR_OK` | Public API | 边界 |

### US-3: groupId 超长错误处理

**作为** 应用开发者,
**我想要** groupId 超长时得到明确错误,
**以便** 避免传入非法值.

**验收标准：**

| AC编号 | 验收标准 | 可观察表面 | 类型 |
|--------|----------|--------------|------|
| AC-3.1 | WHEN 应用传入 groupId 长度超过 64 字节 THEN 绑定层抛出参数错误（NAPI: ThrowInvalidParamError; ANI: ThrowInvalidParamError），不向窗口侧透传 | Public API | 异常 |

## 验收追溯

| AC | 关联规则 | 可观察表面 |
|----|----------|--------------|
| AC-1.1 | R-1 | Public API 返回值 |
| AC-1.2 | R-3 | Public API 错误码 |
| AC-2.1 | R-2 | Public API 返回值 |
| AC-2.2 | R-2 | Public API 返回值 |
| AC-3.1 | R-4 | Public API 异常抛出 |

## 规则定义

| 规则ID | 类型 | 触发条件 | 预期行为 | 边界/约束 | 关联AC |
|--------|------|----------|----------|-----------|--------|
| R-1 | 行为 | label 非空、icon 有效、groupId 非空且 ≤64 字节、SceneBoard 已启用、session 有效 | 返回 `ERR_OK`（0），Promise resolve | groupId 最大 64 字节 | AC-1.1 |
| R-2 | 边界 | groupId 未传入或为空串 | 返回 `ERR_OK`（0），行为与无 groupId 时一致 | ETS 默认 `''` | AC-2.1, AC-2.2 |
| R-3 | 异常 | SceneBoard 未启用 | 返回 `2097230`（ERR_CAPABILITY_NOT_SUPPORT），Promise reject | — | AC-1.2 |
| R-4 | 异常 | groupId 长度 > 64 字节 | 绑定层抛出参数错误，不透传至窗口 | 上限 64 字节（`MAX_ABILITY_INSTANCE_GROUP_ID_LENGTH`） | AC-3.1 |

## 验证映射

| 编号 | 对应规格项 | 测试入口 | 验证方式 | Red 条件（实现前失败信号） | 通过标准 |
|------|------------|----------|----------|---------------------------|----------|
| VM-1 | R-1 / AC-1.1 | `test/unittest/.../ability_context_impl_second_test.cpp::SetAbilityInstanceInfo_0100` | 单测 | 接口无 groupId 参数，编译失败（参数数量不匹配） | 编译通过 + 返回 `ERR_OK` |
| VM-2 | R-2 / AC-2.1, AC-2.2 | `test/unittest/.../ability_context_impl_second_test.cpp::SetAbilityInstanceInfo_0100`（空 groupId 场景） | 单测 | 无 groupId 参数重载，空串场景无法测试 | 传入空串 groupId 返回 `ERR_OK` |
| VM-3 | R-3 / AC-1.2 | `test/unittest/.../ability_context_impl_second_test.cpp`（SceneBoard 未启用场景） | 单测 | `ERR_CAPABILITY_NOT_SUPPORT` 未定义或值不一致 | 返回 `2097230` |
| VM-4 | R-4 / AC-3.1 | NAPI/ANI 绑定层单测 | 单测 | groupId 长度校验逻辑不存在，超长值不报错 | 超长 groupId 抛出参数错误 |

## API 变更分析

### 新增 API

> N/A：本次变更为已有 API 新增参数，无新增 API。

### 变更/废弃 API

| API 名称 | 当前开放级别 | 目标开放级别 | 变更类型 | 影响场景 | 迁移指引 | 关联 AC |
|----------|--------------|--------------|----------|----------|----------|---------|
| `AbilityContext::SetAbilityInstanceInfo` | Public | Public | 变更（新增参数） | C++ 调用方需适配第三参数；不传时编译报错（C++ 无默认值） | 新增参数为必填，已有调用方需补传 groupId 或空串 | AC-1.1 |
| `setAbilityInstanceInfo` (ETS) | Public | Public | 变更（新增可选参数） | ETS 调用方不传 groupId 时行为不变（默认空串） | 向后兼容，无需迁移 | AC-2.1 |

### API 与错误码事实

| 事实项 | 事实类型 | 开放级别 | 精确签名/数值 | 触发条件 | 对外行为 | 事实来源（文件:行/符号或 New 依据） | 关联 AC |
|--------|----------|----------|---------------|----------|----------|--------------------------------------|---------|
| `AbilityContext::SetAbilityInstanceInfo` | Existing API | Public | `virtual ErrCode SetAbilityInstanceInfo(const std::string& label, std::shared_ptr<OHOS::Media::PixelMap> icon, const std::string& groupId) = 0;` | label 非空、icon 有效、groupId ≤64 字节 | 返回 `ErrCode` | `interfaces/kits/native/ability/ability_runtime/ability_context.h:490` | AC-1.1 |
| `setAbilityInstanceInfo` (ETS) | Existing API | Public | `setAbilityInstanceInfo(label: string, icon: image.PixelMap, groupId: string = ''): Promise<void>` | label 非空、icon 有效 | Promise\<void\> resolve/reject | `frameworks/ets/ets/application/UIAbilityContext.ets:780` | AC-1.1, AC-2.1 |
| `ERR_CAPABILITY_NOT_SUPPORT` | Existing Error Code | Public | `2097230` | SceneBoard 未启用 | Promise reject with code 2097230 | `interfaces/inner_api/ability_manager/include/ability_manager_errors.h:415` | AC-1.2 |
| `INVALID_PARAMETERS_ERR` | Existing Error Code | Public | `29360128` | groupId 长度 > 64 字节（绑定层校验） | NAPI: ThrowInvalidParamError; ANI: ThrowInvalidParamError | `interfaces/inner_api/ability_manager/include/ability_manager_errors.h:1325` | AC-3.1 |
| `CHECK_PERMISSION_FAILED` | Existing Error Code | Public | `2097177` | 窗口侧权限校验失败 | Promise reject with code 2097177 | `interfaces/inner_api/ability_manager/include/ability_manager_errors.h:149` | AC-1.2 |
| `MAX_ABILITY_INSTANCE_GROUP_ID_LENGTH` | New API | Public | `64` | groupId 长度上限 | 超过此值触发参数错误 | `New: proposal.md AC-3.1; frameworks/native/ability/native/ability_runtime/js_ability_context.cpp:88; frameworks/ets/ani/ui_ability/src/ets_ability_context.cpp:62` | AC-3.1 |

## 接口规格

### setAbilityInstanceInfo

| 属性 | 值 |
|------|-----|
| 函数签名 (C++) | `ErrCode AbilityContext::SetAbilityInstanceInfo(const std::string& label, std::shared_ptr<OHOS::Media::PixelMap> icon, const std::string& groupId)` |
| 函数签名 (ETS) | `setAbilityInstanceInfo(label: string, icon: image.PixelMap, groupId: string = ''): Promise<void>` |
| 返回值 | C++: `ErrCode`（0 = 成功）; ETS: `Promise<void>` |
| 开放级别 | Public |
| 错误码 | `ERR_OK`(0), `ERR_CAPABILITY_NOT_SUPPORT`(2097230), `CHECK_PERMISSION_FAILED`(2097177), `INVALID_PARAMETERS_ERR`(29360128) |
| 关联 AC | AC-1.1, AC-1.2, AC-2.1, AC-3.1 |

**参数约束**

| 参数 | 类型 | 必填 | 默认值 | 约束条件 |
|------|------|------|--------|---------|
| label | string | 是 | — | 非空；空串触发参数错误 |
| icon | PixelMap | 是 | — | 有效的 PixelMap 对象；null 触发参数错误 |
| groupId | string | 否（ETS 默认空串） | `''` | 长度 ≤ 64 字节；超长触发参数错误 |

**行为场景**

| # | 触发条件 | 可观察表面 | 预期行为 | 关联 AC |
|---|----------|--------------|----------|---------|
| 1 | label 非空、icon 有效、groupId 非空 ≤64 字节、SceneBoard 启用 | Public API | 返回 `ERR_OK`，Promise resolve | AC-1.1 |
| 2 | groupId 未传入或空串 | Public API | 返回 `ERR_OK`，行为不变 | AC-2.1, AC-2.2 |
| 3 | SceneBoard 未启用 | Public API | 返回 `2097230`，Promise reject | AC-1.2 |
| 4 | groupId 长度 > 64 字节 | Public API | 绑定层抛出参数错误，不透传 | AC-3.1 |

## 兼容性声明

- **已有 API 行为变更:** 是 — `SetAbilityInstanceInfo` C++ 签名新增第三参数 `groupId`（必填，无默认值）；ETS 新增可选参数 `groupId: string = ''`（向后兼容）
- **配置文件格式变更:** 否
- **数据存储格式变更:** 否
- **最低支持版本:** OpenHarmony 6.0 (API 26.1.0+)
- **API 版本号策略:** groupId 参数标注 `@since 26.1.0`

## 架构约束

| 关键约束 | 约束说明 | 影响 AC |
|----------|----------|---------|
| groupId 透传至窗口 | 实现层通过 `ifaceSession->SetSessionLabelAndIcon(label, icon, groupId)` 透传，窗口侧已支持 | AC-1.1 |
| 绑定层长度校验 | NAPI/ANI 绑定层校验 groupId ≤ 64 字节，C++ SDK 层不校验（透传） | AC-3.1 |
| SceneBoard 前置条件 | 未启用 SceneBoard 时返回 `ERR_CAPABILITY_NOT_SUPPORT`，不向窗口侧透传 | AC-1.2 |

## 非功能性需求

| 类型 | 指标/阈值 | 验证方式 | 证据 |
|------|-----------|----------|------|
| 可测试性 | groupId 参数可通过单测验证传递与校验 | 单测 | `test/unittest/.../ability_context_impl_second_test.cpp` |

## 多设备适配声明

| 设备类型 | 行为差异 | 规格/约束 | 验证方式 | 证据 |
|----------|----------|-----------|----------|------|
| 全设备 | 无差异 | groupId 透传行为一致 | 单测 | — |

## 全局特性影响

| 特性 | 适用？ | 结论 | 关联场景 |
|------|--------|------|----------|
| 无障碍 | 否 | groupId 为技术标识，不影响无障碍 | — |
| 大字体 | 否 | 无影响 | — |
| 深色模式 | 否 | 无影响 | — |
| 多窗口/分屏 | 否 | 无影响 | — |
| 多用户 | 否 | 无影响 | — |
| 版本升级 | 是 | 新增参数向后兼容（ETS 可选），C++ 需适配 | — |
| 生态兼容 | 是 | ETS 向后兼容；C++ SDK 新增必填参数需适配 | — |

## context-references

```yaml
context-queries:
  - repo: "openharmony/ability_ability_runtime"
    query: "SetAbilityInstanceInfo implementation and groupId parameter flow"
```

**关键文档：** `interfaces/kits/native/ability/ability_runtime/ability_context.h`; `frameworks/native/ability/ability_runtime/ability_context_impl.cpp`

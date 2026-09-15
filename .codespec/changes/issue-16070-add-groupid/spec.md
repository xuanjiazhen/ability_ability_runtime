# 特性规格

> 固化用户可见行为和验收标准。

> **不应包含：** InnerKit 接口定义、内部实现流程、中间件调用链路、框架层类名/方法名、内部性能指标。这些内容属于 design.md。Spec 只描述用户可见行为、API 契约和验收标准。

## 概述

| 属性 | 值 |
|------|-----|
| 特性名称 | setAbilityInstanceInfo 新增必选 groupId 参数重载 |
| 特性编号 | issue-16070-add-groupid |
| 所属 Epic | 无 |
| 优先级 | P1 |
| 目标版本 | 引用 manifest.target_release |
| SIG 归属 | AppFwk SIG |
| 状态 | Approved |
| 复杂度 | 标准 |

## 本次变更范围（Delta）

| 类型 | 内容 | 说明 |
|------|------|------|
| ADDED | `setAbilityInstanceInfo(label, icon, groupId)` 新重载 | groupId 必选，用于 Ability 实例分组 |
| MODIFIED | `setAbilityInstanceInfo(label, icon)` 原重载 | 行为等价于 groupId=""，向后兼容 |
| MODIFIED | C++ Inner API `SetAbilityInstanceInfo` 签名 | 新增 groupId 必选参数（无默认值），breaking change |
| MODIFIED | NAPI `setAbilityInstanceInfo` 签名 | 从 `(label, callback)` 变更为 `(label, icon, groupId)`，breaking change（原签名已损坏） |

## 输入文档

| 文档 | 路径 | 状态 |
|------|------|------|
| Requirement | `proposal.md` | Approved |

> 需求基线、不涉及项、受影响子系统与仓库详见 proposal.md，本文档不重复摘录。design.md 与本文档并行产出，互不依赖。

## 用户故事

### US-1: 应用设置 Ability 实例分组

**作为** 应用开发者, 调用 `setAbilityInstanceInfo(label, icon, groupId)` 将 Ability 实例归入指定分组,
**以便** 窗口管理侧可按分组管理多实例（如分屏组、悬浮窗组）.

**验收标准：**

- **AC-1.1:** WHEN 设备支持 SceneBoard 且当前 Ability 拥有有效 session，调用 `setAbilityInstanceInfo(label, icon, groupId)` 传入合法 groupId（≤ 64 字节） THEN Promise resolve
- **AC-1.2:** WHEN 传入 groupId 长度 > 64 字节 THEN 同步抛出错误码 401，不执行设置操作
- **AC-1.3:** WHEN 传入 groupId 长度恰好 64 字节 THEN Promise resolve
- **AC-1.4:** WHEN 应用通过 NAPI 调用且第三参数为非 string 类型 THEN groupId 视为空串，Promise resolve

### US-2: 向后兼容

**作为** 已有应用, 继续使用 `setAbilityInstanceInfo(label, icon)` 而不传 groupId,
**以便** 不修改已有代码即可继续正常运行.

**验收标准：**

- **AC-2.1:** WHEN 调用 `setAbilityInstanceInfo(label, icon)` 不传 groupId，且设备支持 SceneBoard 且 session 有效 THEN Promise resolve，行为等价于 groupId=""

### US-3: 异常场景错误码

**作为** 应用开发者, 在设备不支持或 session 无效时获得明确的错误码,
**以便** 正确处理异常.

**验收标准：**

- **AC-3.1:** WHEN 设备不支持 SceneBoard，调用任一重载 THEN Promise reject，错误码 801
- **AC-3.2:** WHEN session token 为空或已失效，调用含 groupId 重载且设备支持 SceneBoard THEN Promise reject，错误码 16000050
- **AC-3.3:** WHEN context 为空或已释放 THEN Promise reject，错误码 16000011
- **AC-3.4:** WHEN 窗口侧返回权限校验失败 THEN Promise reject，错误码 201
- **AC-3.5:** WHEN 窗口侧返回设置失败 THEN Promise reject，错误码 401

## 验收追溯

| AC | 关联规则 | 关联 Task | 验证方式 | 证据 |
|----|----------|-----------|----------|------|
| AC-1.1 | BR-1, FR-1 | TASK-1, TASK-2 | 单测 | `[路径]` |
| AC-1.2 | BR-2, EX-1 | TASK-2, TASK-3 | 单测 | `[路径]` |
| AC-1.3 | BR-1 | TASK-2 | 单测 | `[路径]` |
| AC-1.4 | BR-3 | TASK-3 | 单测 | `[路径]` |
| AC-2.1 | BR-4 | TASK-2 | 单测 | `[路径]` |
| AC-3.1 | EX-2 | — | 单测（回归） | `[路径]` |
| AC-3.2 | EX-3 | — | 单测（回归） | `[路径]` |
| AC-3.3 | EX-4 | — | 单测（回归） | `[路径]` |
| AC-3.4 | EX-5 | — | 单测（回归） | `[路径]` |
| AC-3.5 | EX-6 | — | 单测（回归） | `[路径]` |

## 业务规则

| 编号 | 规则描述 | 约束条件 | 关联 AC |
|------|----------|----------|---------|
| BR-1 | groupId 为 string 且长度 ≤ 64 字节时正常执行 | 长度上限 64 字节 | AC-1.1, AC-1.3 |
| BR-2 | groupId 长度 > 64 字节时同步抛出 401 | 调用后立即抛出，不进入 Promise | AC-1.2 |
| BR-3 | NAPI 第三参数为非 string 类型时 groupId 视为空串 | 仅 NAPI 路径；ANI 路径 groupId 必选 | AC-1.4 |
| BR-4 | 原重载不传 groupId 时行为等价于 groupId="" | 向后兼容 | AC-2.1 |

## 功能规则

| 编号 | 规则描述 | 触发条件 | 作用对象 | 关联 AC |
|------|----------|----------|----------|---------|
| FR-1 | 设置 Ability 实例的 label、icon 和 groupId 到窗口 session | 调用 `setAbilityInstanceInfo` 且参数合法 | 当前 Ability 的窗口 session | AC-1.1 |

## 异常/豁免规则

| 编号 | 异常码/枚举 | 规则描述 | 触发条件 | 超时阈值 | 处理结果 | 关联 AC |
|------|------------|----------|----------|----------|----------|---------|
| EX-1 | 401 (ERROR_CODE_INVALID_PARAM) | groupId 超长，同步抛出 | groupId 长度 > 64 字节 | N/A | 同步抛出 401，不执行设置 | AC-1.2 |
| EX-2 | 801 (ERROR_CODE_CAPABILITY_NOT_SUPPORT) | 设备不支持 SceneBoard | SceneBoard 未启用 | N/A | Promise reject 801 | AC-3.1 |
| EX-3 | 16000050 (ERROR_CODE_INNER) | session 无效 | session token 为空或已失效 | N/A | Promise reject 16000050 | AC-3.2 |
| EX-4 | 16000011 (ERROR_CODE_INVALID_CONTEXT) | context 无效 | context 为空或已释放 | N/A | Promise reject 16000011 | AC-3.3 |
| EX-5 | 201 (ERROR_CODE_PERMISSION_DENIED) | 窗口侧权限校验失败 | 窗口侧返回权限失败 | N/A | Promise reject 201 | AC-3.4 |
| EX-6 | 401 (ERROR_CODE_INVALID_PARAM) | 窗口侧设置失败 | 窗口侧返回设置失败 | N/A | Promise reject 401 | AC-3.5 |

## 恢复契约

| 编号 | 触发条件 | 恢复策略 | 恢复结果 | 约束 |
|------|----------|----------|----------|------|
| RC-1 | 同步抛出 401（groupId 超长） | 调用方修正 groupId 长度后重新调用 | 正常执行 | groupId ≤ 64 字节 |
| RC-2 | Promise reject（设备/session/context/权限异常） | 调用方根据错误码处理异常 | 调用方自行决定重试或降级 | N/A |

## 验证映射

| 编号 | 对应规格项 | 验证方式 | 验证重点 |
|------|------------|----------|----------|
| VM-1 | FR-1 / AC-1.1 | 单测 | 有效 session + 非空 groupId → Promise resolve |
| VM-2 | BR-2 / AC-1.2 | 单测 | groupId 65 字节 → 同步抛出 401 |
| VM-3 | BR-1 / AC-1.3 | 单测 | groupId 恰好 64 字节 → Promise resolve |
| VM-4 | BR-3 / AC-1.4 | 单测 | 第三参数为 number → groupId 降级为 ""，Promise resolve |
| VM-5 | BR-4 / AC-2.1 | 单测 | 不传 groupId → Promise resolve |
| VM-6 | EX-2 / AC-3.1 | 单测 | SceneBoard 禁用 → Promise reject 801 |
| VM-7 | EX-3 / AC-3.2 | 单测 | session 为空 → Promise reject 16000050 |
| VM-8 | EX-4 / AC-3.3 | 单测 | context 已释放 → Promise reject 16000011 |
| VM-9 | EX-5 / AC-3.4 | 单测 | 窗口侧权限失败 → Promise reject 201 |
| VM-10 | EX-6 / AC-3.5 | 单测 | 窗口侧设置失败 → Promise reject 401 |

## API 变更分析

### 新增 API

| API 名称 | 开放范围 | 入参概要 | 返回值 | 错误码范围 | 功能描述 | 关联 AC |
|----------|----------|----------|--------|------------|----------|---------|
| `setAbilityInstanceInfo(label, icon, groupId)` | Public | label: string, icon: PixelMap, groupId: string | Promise\<void\> | 0, 401, 201, 801, 16000011, 16000050 | 设置 Ability 实例 label、icon 和 groupId | AC-1.1~1.4, AC-3.1~3.5 |

### 变更/废弃 API

| API 名称 | 变更类型 | 影响场景 | 迁移指引 | 关联 AC |
|----------|----------|----------|----------|---------|
| `setAbilityInstanceInfo(label, icon)` | 变更（行为） | 原重载内部行为等价于 groupId="" | 无需迁移，已有调用零修改可正常运行 | AC-2.1 |
| C++ `SetAbilityInstanceInfo(label, icon, groupId)` | 变更（签名） | C++ Inner API 消费者编译失败 | 显式传入 groupId 或 "" | AC-1.1 |
| NAPI `setAbilityInstanceInfo(label, icon, groupId)` | 变更（签名） | 原签名 `(label, callback)` 已损坏（callback 被错误解析为 icon） | 传入 `(label, icon, groupId)` | AC-1.1 |

> API 签名、d.ts 位置、权限要求等实现细节见 design.md。

## 兼容性声明

- **已有 API 行为变更:** 是。ETS 原重载行为等价于 groupId=""（向后兼容）；C++ Inner API 和 NAPI AbilityContext 绑定为 breaking change
- **配置文件格式变更:** 否
- **数据存储格式变更:** 否
- **最低支持版本:** 引用 manifest.target_release
- **API 版本号策略:** 新增重载沿用原 API 的 @since 标注

## 架构约束

| 关键约束 | 约束说明 | 影响 AC |
|----------|----------|---------|
| groupId 透传到窗口 session | ability_runtime 仅透传 groupId 到窗口侧，不解释语义 | AC-1.1 |
| groupId 长度校验在绑定层 | NAPI/ANI 绑定层校验 ≤ 64 字节，超长同步抛出 401 | AC-1.2 |
| 错误码不新增 | 复用既有错误码，不为 groupId 引入专属错误码 | AC-1.2, AC-3.1~3.5 |

## 非功能性需求

> N/A。proposal.md 不涉及项确认：性能、功耗、内存、安全均不涉及。

## 多设备适配声明

无差异。groupId 参数行为在所有设备类型上一致。

## 全局特性影响

| 特性 | 适用？ | 结论 | 关联场景 |
|------|--------|------|----------|
| 无障碍 | 否 | groupId 为技术标识符，无 UI 文案 | — |
| 大字体 | 否 | 无 UI 变更 | — |
| 深色模式 | 否 | 无 UI 变更 | — |
| 多窗口/分屏 | 否 | groupId 用于实例分组，但分组行为由窗口侧实现 | — |
| 多用户 | 否 | groupId 无用户隔离语义 | — |
| 版本升级 | 否 | 无数据格式变更 | — |
| 生态兼容 | 是 | NAPI 签名 breaking change，原签名已损坏 | NAPI `setAbilityInstanceInfo` |

## Spec 自审清单

- [x] 无"待定""TBD""TODO"等占位符
- [x] 所有 AC 使用 WHEN/THEN 格式，可独立测试
- [x] 范围边界明确（做什么/不做什么清晰）
- [x] 无语义模糊表述（"快速""稳定""尽可能"等）
- [x] AC 与业务规则/异常规则/恢复契约交叉一致

## context-references

```yaml
context-queries:
  - repo: "openharmony/ability_ability_runtime"
    query: "setAbilityInstanceInfo 接口签名和错误码映射"
```

**关键文档：** proposal.md, design.md

# Design: setAbilityInstanceInfo 新增必选 groupId 参数重载

> PR: https://gitcode.com/openharmony/ability_ability_runtime/pull/20307

## 1. 架构上下文

### 1.1 现有调用链（变更前）

```
应用 (ETS/JS)
  ↓ setAbilityInstanceInfo(label, icon)
UIAbilityContext.ets / ability_context.js
  ↓ nativeSetAbilityInstanceInfoCheck(label, icon)
  ↓ nativeSetAbilityInstanceInfo(label, icon, callback)
ANI: EtsAbilityContext::SetAbilityInstanceInfo(label, icon, callback)
NAPI: JsAbilityContext::OnSetAbilityInstanceInfo(label, icon)
  ↓
AbilityContextImpl::SetAbilityInstanceInfo(label, icon)
  ↓ ifaceSession->SetSessionLabelAndIcon(label, icon)     ← 2 参数
Rosen ISession
```

### 1.2 变更后调用链

```
应用 (ETS)
  ├─ setAbilityInstanceInfo(label, icon)                   ← 原重载，兼容
  │   ↓ nativeSetAbilityInstanceInfoCheck(label, icon, '')
  │   ↓ nativeSetAbilityInstanceInfo(label, icon, '', callback)
  └─ setAbilityInstanceInfo(label, icon, groupId)          ← 新重载
      ↓ nativeSetAbilityInstanceInfoCheck(label, icon, groupId)
      ↓ nativeSetAbilityInstanceInfo(label, icon, groupId, callback)

ANI: EtsAbilityContext::SetAbilityInstanceInfo(label, icon, groupId, callback)
  ├─ SetAbilityInstanceInfoCheck: 校验 groupId ≤ 64 字节
  └─ OnSetAbilityInstanceInfo: 再次校验 groupId ≤ 64 字节（防御性双重校验）
      ↓ context->SetAbilityInstanceInfo(label, icon, groupId)

NAPI: JsAbilityContext::OnSetAbilityInstanceInfo(label, icon, [groupId])
  ├─ argc > 2 && argv[2] is string → 解析 groupId
  ├─ argc ≤ 2 || argv[2] not string → groupId = ""（默认）
  └─ 校验 groupId.length() ≤ 64
      ↓ context->SetAbilityInstanceInfo(label, icon, groupId)

AbilityContextImpl::SetAbilityInstanceInfo(label, icon, groupId)
  ├─ SceneBoard 未启用 → ERR_CAPABILITY_NOT_SUPPORT
  ├─ session 为空 → ERR_INVALID_VALUE
  └─ ifaceSession->SetSessionLabelAndIcon(label, icon, groupId)  ← 3 参数
      ↓ TransferSetAbilityInstanceInfoErr(errCode)
Rosen ISession
```

### 1.3 关键约束
- 窗口侧 `SetSessionLabelAndIcon` 已支持 3 参数版本（跨仓先行）
- `TransferSetAbilityInstanceInfoErr` 映射逻辑不变
- ETS 原重载保留，内部传空串实现向后兼容
- C++ SDK 签名为 breaking change（groupId 无默认值）

### 1.4 行为规则

#### groupId 参数校验

| 条件 | 系统行为 |
|------|----------|
| groupId 为 string 且长度 ≤ 64 字节 | 正常执行 |
| groupId 为 string 且长度 > 64 字节 | 抛出参数错误（Invalid groupId），不执行设置操作 |
| groupId 未提供（ETS 原重载 / NAPI argc ≤ 2） | groupId 视为空串 ""，正常执行（不分组） |
| groupId 为非 string 类型（仅 NAPI 可选路径） | groupId 视为空串 ""，正常执行 |

#### 执行结果

| 条件 | 返回值 / Promise 状态 |
|------|----------------------|
| 设备支持 SceneBoard 且 session 有效 | resolve(undefined) / ERR_OK |
| 设备不支持 SceneBoard | reject(ERR_CAPABILITY_NOT_SUPPORT, 2097230) / ERR_CAPABILITY_NOT_SUPPORT |
| session 无效（token 为空） | reject(ERR_INVALID_VALUE) / ERR_INVALID_VALUE |
| 窗口侧权限校验失败 | reject(CHECK_PERMISSION_FAILED) / CHECK_PERMISSION_FAILED |
| 窗口侧设置 label 失败 | reject(INVALID_PARAMETERS_ERR) / INVALID_PARAMETERS_ERR |
| 窗口侧设备不支持 | reject(ERR_CAPABILITY_NOT_SUPPORT) / ERR_CAPABILITY_NOT_SUPPORT |

#### 兼容性规则

- ETS 原重载 `setAbilityInstanceInfo(label, icon)` 行为等价于 groupId=""，已有应用零修改可正常运行
- C++ Inner API 签名为 breaking change（groupId 无默认值），C++ 消费者必须显式传入 groupId 或 ""
- NAPI AbilityContext 绑定签名为 breaking change（从 `(label, callback)` 变更为 `(label, icon, groupId)`，原签名已损坏）
- 错误码映射逻辑不变，无新增错误码

---

## 2. 设计决策 (ADR)

### ADR-1: ETS 采用重载而非默认参数

**决策**：在 `UIAbilityContext.ets` 新增独立的 `setAbilityInstanceInfo(label, icon, groupId)` 重载方法，保留原 `setAbilityInstanceInfo(label, icon)` 重载。

**理由**：
- ETS/ArkTS 方法重载是公共 API 的标准扩展方式
- 保留原重载实现 100% 向后兼容，已有应用零修改即可正常运行
- 原重载内部传 `""` 等价于不分组，语义清晰

**替代方案**：使用默认参数 `groupId: string = ''` — ArkTS 对默认参数的公共 API 兼容性约束比重载更复杂，重载更明确。

### ADR-2: C++ SDK groupId 为必选参数（无默认值）

**决策**：`AbilityContext::SetAbilityInstanceInfo` 签名变更为 `SetAbilityInstanceInfo(label, icon, groupId)`，groupId 为 `const std::string&` 无默认值。

**理由**：
- C++ Inner API 不需要与 ETS 公共 API 保持相同的兼容性约束
- 必选参数强制所有 C++ 消费者显式传入 groupId（或显式传 `""`），避免遗漏
- C++ 消费者数量有限且可控，breaking 影响面可接受

**替代方案**：`groupId = ""` 默认值 — 会掩盖遗漏意图，不强制消费者显式表达。

### ADR-3: NAPI groupId 可选解析（argc 检测）

**决策**：NAPI 绑定层通过 `info.argc > ARGC_TWO && CheckTypeForNapiValue(env, info.argv[INDEX_TWO], napi_string)` 检测 groupId 是否提供。未提供时 groupId 默认 `""`。

**理由**：
- NAPI 绑定层需同时支持新旧两种 ETS 调用方式（有/无 groupId）
- argc 检测是 NAPI 标准的可选参数处理模式，与既有代码一致
- 当 argc ≤ 2 或 argv[2] 非 string 时，安全降级为 `""`

### ADR-4: ANI groupId 为必选参数

**决策**：ANI 绑定层 `SetAbilityInstanceInfo` 和 `SetAbilityInstanceInfoCheck` 签名新增 `ani_string groupIdObj` 必选参数。ANI native 注册签名新增 `C{std.core.String}`。

**理由**：
- ANI 绑定层签名由 ETS 重载直接映射，新重载的 groupId 为必选
- 原重载调用走 ANI 层的原有签名路径（ETS 编译器按参数数量分派到不同 native 函数）
- ANI 层不做 argc 可选检测（与 NAPI 不同），因为 ETS 重载在编译期已确定

### ADR-5: groupId 长度校验 64 字节，在绑定层强制

**决策**：定义 `MAX_ABILITY_INSTANCE_GROUP_ID_LENGTH = 64`，在 NAPI 和 ANI 绑定层强制校验。超长抛参数错误，不调用 C++ 实现。

**理由**：
- 64 字节与 docs 文档约束一致
- 绑定层校验拦截无效输入，避免传递到 C++/窗口层
- C++ SDK 层不做长度校验（Inner API 消费者自行保证；绑定层是应用侧入口）

**替代方案**：C++ 层也做校验 — 增加重复检查，Inner API 消费者应自行保证参数合法性。

### ADR-6: ANI 双重校验（Check + OnSet）

**决策**：ANI 绑定层在 `SetAbilityInstanceInfoCheck`（同步前置校验）和 `OnSetAbilityInstanceInfo`（异步执行前）两处都做 groupId 解析与长度校验。

**理由**：
- `SetAbilityInstanceInfoCheck` 为同步快速失败入口，在 taskpool 提交前拦截
- `OnSetAbilityInstanceInfo` 为异步执行前的防御性再次校验，防止 Check 与 OnSet 间 groupId 被篡改（虽然实际不可能，但防御性设计）
- 与既有 label/icon 校验模式一致（二者也在两处校验）

---

## 3. 不涉及项确认

| 项 | 说明 |
|----|------|
| 修改 Rosen `ISession::SetSessionLabelAndIcon` 接口 | 不涉及。窗口侧已先行支持 3 参数版本 |
| 修改 `TransferSetAbilityInstanceInfoErr` 映射 | 不涉及。沿用现有 5 个 WSError 映射 |
| 新增 groupId 专属错误码 | 不涉及。groupId 校验失败复用 `ThrowInvalidParamError` |
| 修改 hisysevent.yaml | 不涉及。沿用现有事件定义和日志 tag |
| 修改 IPC Stub/Proxy | 不涉及。`SetSessionLabelAndIcon` 为跨仓 Rosen IPC |
| 修改 BUILD.gn | 不涉及。无新增源文件 |
| 新增 Want 常量 | 不涉及。groupId 为 API 参数，非 Want 参数 |

---

## 4. 修改物清单

### 4.1 C++ SDK 接口（interfaces/kits/native）

| 文件 | 类型 | 修改内容 |
|------|------|----------|
| `ability/ability_runtime/ability_context.h:490` | 签名变更 | `SetAbilityInstanceInfo` 新增 `const std::string& groupId` 必选参数；Doxygen 注释新增 `@param groupId` |
| `ability/ability_runtime/ability_context_impl.h:323` | 签名变更 | 同上 |

### 4.2 C++ 实现（frameworks/native/ability）

| 文件 | 类型 | 修改内容 |
|------|------|----------|
| `ability_runtime/ability_context_impl.cpp:1149-1164` | 实现变更 | `SetAbilityInstanceInfo` 新增 groupId 参数；`SetSessionLabelAndIcon(label, icon, groupId)` 传 3 参数 |

### 4.3 NAPI 绑定（frameworks/native/ability/native）

| 文件 | 类型 | 修改内容 |
|------|------|----------|
| `ability_runtime/js_ability_context.cpp:88` | 新增常量 | `MAX_ABILITY_INSTANCE_GROUP_ID_LENGTH = 64` |
| `ability_runtime/js_ability_context.cpp:3072-3083` | 实现扩展 | 新增 groupId 可选解析（argc 检测）+ 长度校验；lambda capture groupId by value |
| `ability_runtime/js_ability_context.cpp:3094` | 调用变更 | `context->SetAbilityInstanceInfo(label, icon, groupId)` |

### 4.4 ANI 绑定（frameworks/ets/ani/ui_ability）

| 文件 | 类型 | 修改内容 |
|------|------|----------|
| `include/ets_ability_context.h:166-177` | 签名变更 | `SetAbilityInstanceInfo` 新增 `ani_string groupIdObj`；`SetAbilityInstanceInfoCheck` 新增 `ani_string groupIdObj`；`OnSetAbilityInstanceInfo` 新增 `ani_string groupIdObj`；`OnSetAbilityInstanceInfoInner` 新增 `std::string& groupId` |
| `src/ets_ability_context.cpp:62` | 新增常量 | `MAX_ABILITY_INSTANCE_GROUP_ID_LENGTH = 64` |
| `src/ets_ability_context.cpp:775-815` | 签名变更 | `SetAbilityInstanceInfo` / `SetAbilityInstanceInfoCheck` 签名新增 groupId；Check 中新增 groupId 解析与校验 |
| `src/ets_ability_context.cpp:2753-2835` | 签名变更 | `OnSetAbilityInstanceInfo` / `OnSetAbilityInstanceInfoInner` 新增 groupId；OnSet 中新增 groupId 解析与校验；lambda capture groupId |
| `src/ets_ability_context.cpp:3237-3241` | 签名变更 | ANI native 注册签名 `nativeSetAbilityInstanceInfo` 和 `nativeSetAbilityInstanceInfoCheck` 新增 `C{std.core.String}` |

### 4.5 ETS 公共 API（frameworks/ets/ets）

| 文件 | 类型 | 修改内容 |
|------|------|----------|
| `application/UIAbilityContext.ets:173-174` | native 声明变更 | `nativeSetAbilityInstanceInfo` 新增 `groupId: string` 参数 |
| `application/UIAbilityContext.ets:236` | native 声明变更 | `nativeSetAbilityInstanceInfoCheck` 新增 `groupId: string` 参数 |
| `application/UIAbilityContext.ets:780-812` | 新增重载 | 保留原重载（内部传 `''`）；新增 `setAbilityInstanceInfo(label, icon, groupId)` 重载 |

### 4.6 Stage 模型 NAPI AbilityContext 绑定（frameworks/js/napi）

| 文件 | 类型 | 修改内容 |
|------|------|----------|
| `ability_context/ability_context.js:204-205` | 签名变更 | `setAbilityInstanceInfo(label, icon, groupId)` 从 `(label, callback)` 变更 |

### 4.7 单元测试

| 文件 | 类型 | 修改内容 |
|------|------|----------|
| `ability_context_impl_second_test.cpp:90-104` | 适配 | `SetAbilityInstanceInfo_0100` 三个子场景传入 `""` 作为 groupId |
| `ability_context_test.cpp:490` | 适配 | mock 类 `SetAbilityInstanceInfo` 签名新增 `const std::string& groupId` |

---

## 5. 架构图

```
                ┌─────────────────────────────────────────────────┐
                │  应用 (ETS)                                      │
                │                                                  │
                │  setAbilityInstanceInfo(label, icon)             │ ← 原重载
                │  setAbilityInstanceInfo(label, icon, groupId)     │ ← 新重载
                └────────────────────┬────────────────────────────┘
                                     ↓
         ┌───────────────────────────┴───────────────────────────┐
         ↓ (ETS/ArkTS)                                          ↓ (JS/FA)
  ┌──────────────────────┐                          ┌────────────────────┐
  │  ANI 绑定层          │                          │  NAPI 绑定层       │
  │  EtsAbilityContext   │                          │  JsAbilityContext  │
  │                      │                          │                    │
  │  groupId 必选        │                          │  groupId 可选      │
  │  校验 ≤ 64 字节      │                          │  校验 ≤ 64 字节    │
  │  (Check + OnSet     │                          │  (argc 检测)       │
  │   双重校验)          │                          │                    │
  └──────────┬───────────┘                          └──────────┬─────────┘
             │                                                 │
             └───────────────────────┬────────────────────────┘
                                     ↓
                    ┌────────────────────────────────┐
                    │  C++ SDK (Inner API)            │
                    │  AbilityContextImpl::           │
                    │    SetAbilityInstanceInfo(      │
                    │      label, icon, groupId)     │
                    │  groupId 必选（无默认值）       │
                    └────────────────┬───────────────┘
                                     ↓
                    ┌────────────────────────────────┐
                    │  Rosen ISession (跨仓)          │
                    │  SetSessionLabelAndIcon(        │
                    │    label, icon, groupId)        │
                    │  (窗口侧已先行支持)             │
                    └────────────────────────────────┘
```

---

## 6. 状态归属与不变量

- **Ownership**：无新增状态。groupId 为参数透传，不存储、不缓存。
- **Lifecycle**：groupId 生命周期与单次 API 调用绑定。NAPI/ANI lambda 按 value 捕获 groupId（线程安全）。
- **Concurrency**：无共享可变状态。groupId 为 immutable string，lambda capture by value。
- **Capacity**：groupId 最大 64 字节，在绑定层强制。

---

## DFX 设计

未命中知识库，不涉及。沿用现有 `TAG_LOGD(AAFwkTag::CONTEXT, "SetAbilityInstanceInfo")` 和 `TAG_LOGI(AAFwkTag::CONTEXT, "SetSessionLabelAndIcon")` 日志。无 hisysevent.yaml 变更。

## 7. 关键风险与缓解

| 风险 | 缓解措施 |
|------|----------|
| C++ SDK breaking change（groupId 无默认值） | C++ Inner API 消费者数量有限且可控；需同步更新所有调用方（包括 mock 类） |
| NAPI AbilityContext 绑定 breaking change | 原签名 `(label, callback)` 已损坏（callback 被错误解析为 icon），本次变更是修复 |
| groupId 校验在 NAPI/ANI 层但不 C++ 层 | C++ Inner API 消费者应自行保证参数合法性；绑定层是应用侧入口已拦截 |
| 窗口侧 groupId 返回新 WSError 未映射 | `TransferSetAbilityInstanceInfoErr` default 分支 `static_cast<int32_t>` 透传原始值 |

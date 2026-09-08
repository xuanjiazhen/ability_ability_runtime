# 给 SA 提供指定 tokenId 执行意图的 inner 接口 — AMS 侧需求分析

## 1. 功能概述

### 1.1 背景

卡片（Form）当前已支持 `router` 事件，FMS（Form Manager Service）通过调用 AMS 的 `StartAbilityWithSpecifyTokenId` 接口拉起指定 UIAbility。核心调用方式为：

```
FMS → AMS::StartAbilityWithSpecifyTokenId(want, callerToken, specifyTokenId, userId)
```

- `want`：被拉起的 bundle/module/ability 三元组 + 自定义参数
- `callerToken`：调用方 token（卡片场景为大桌面的 callerToken），用于前后台校验
- `specifyTokenId`：指定的 tokenId，用于拉起时的权限校验
- `userId`：用户 ID

### 1.2 需求

卡片新增支持 **UIAbility 前台意图执行事件**（InsightIntent Execute），FMS 需要调用 AMS 新增的 inner API `ExecuteIntentWithSpecifyTokenId`，将会话中用户触发的意图（Intent）以指定 tokenId 的身份执行，拉起目标 UIAbility 并携带卡片自定义参数。

### 1.3 核心流程对比

| 事件类型 | 已有接口 | 新增接口 |
|---------|---------|---------|
| Card router 事件 | `StartAbilityWithSpecifyTokenId` | — |
| Card 意图执行事件 | — | `ExecuteIntentWithSpecifyTokenId` |

两接口的调用约定一致：
- 仅允许 FOUNDATION_UID 进程（即 FMS）调用
- 调用方传入 `specifyTokenId` 作为权限校验的身份
- 调用方传入 `callerToken` 作为前后台校验的凭证

---

## 2. 实现方案

### 2.1 整体架构

```
FMS (foundation 进程)
    │
    │ ExecuteIntentWithSpecifyTokenId(key, hostClient, param, wantParams, specifyTokenId, callerToken)
    ▼
AbilityManagerClient (客户端代理)
    │
    │ IPC (EXECUTE_INTENT_WITH_SPECIFY_TOKEN_ID = 1147)
    ▼
AbilityManagerStub (反序列化)
    │
    ▼
AbilityManagerService.ExecuteIntentWithSpecifyTokenId()
    │
    ├── 1. 校验调用方 UID == FOUNDATION_UID
    ├── 2. 合并 FMS 传入的 wantParams 到 insightIntentParam_（过滤 ohos.insightIntent.* 系统 key）
    ├── 3. CheckCrossUserPermission()
    ├── 4. 提取 Intent GenericInfo，判断 ignoreAbilityName
    ├── 5. CheckAndUpdateParam(key, hostClient, param, ..., specifyTokenId)
    │      └── 内部使用 specifyTokenId 做权限校验 CheckCallerPermission
    ├── 6. 构造 ExecuteIntentCommonOptions(specifyTokenId, callerToken, wantParams)
    └── 7. ExecuteIntentCommon(hostClient, param, callerBundleName, options)
           │
           └── UI_ABILITY_FOREGROUND 分支
                ├── ExpandCardFormIdentityToWantTop() — 将卡片 formId 参数展开到 want 顶层
                └── StartAbilityWithInsightIntent(want, userId, ... specifyTokenId, callerToken)
                     └── StartAbilityInner(startAbilityWrapParam)
                          └── 当 specifyTokenId > 0 && callerToken != nullptr
                              └── UpdateCallerInfoFromToken() — 从 callerToken 更新调用者信息
```

### 2.2 关键变更点

#### 2.2.1 IPC 协议层

| 文件 | 变更 |
|------|------|
| `AbilityManagerInterfaceCode` | 新增 `EXECUTE_INTENT_WITH_SPECIFY_TOKEN_ID = 1147` |
| `AbilityManagerInterface` | 新增纯虚函数 `ExecuteIntentWithSpecifyTokenId()` |
| `AbilityManagerProxy` | 序列化 6 个参数（key, hostClient, param, wantParams, specifyTokenId, callerToken），发送 IPC |
| `AbilityManagerStub` | 新增 `ExecuteIntentWithSpecifyTokenIdInner()` — 反序列化 + 调用 Service |
| `AbilityManagerClient` | 新增客户端封装方法 |
| `AbilityManagerService` | 声明 + 实现 `ExecuteIntentWithSpecifyTokenId()` |

#### 2.2.2 服务层

| 文件 | 变更 |
|------|------|
| `AbilityManagerService` | 新增 `ExecuteIntentWithSpecifyTokenId()` 实现 |
| `AbilityManagerService` | `StartAbilityWithInsightIntent()` 新增 `specifyTokenId`、`callerToken` 参数 |
| `AbilityManagerService` | `StartAbilityInner()` 中 `specifyTokenId` 参数类型从 `uint32_t` 调整为 `uint64_t` 对齐 |
| `AbilityManagerService` | `ExecuteIntentCommon()` 中新增 `ExpandCardFormIdentityToWantTop` 闭包 |
| `ExtractInsightIntentProfile` | `ExecuteIntentCommonOptions` 新增 `specifyTokenId`、`callerToken`、`wantParams` 字段 |
| `InsightIntentExecuteManager` | `CheckCallerPermission()` 中 `specifiedFullTokenId` 参数 `static_cast<uint32_t>` 适配 |

#### 2.2.3 数据流增强

```
FMS 传入 wantParams (含卡片参数)
    │
    ├── 过滤 ohos.insightIntent.* 前缀的系统 key
    ├── 合并到 InsightIntentExecuteParam.insightIntentParam_
    │
    └── ExecuteIntentCommon 中 ExpandCardFormIdentityToWantTop
        ├── 将 wantParams 中的 "ohos.extra.param.key.form_identity" 展开到 want 顶层
        └── 将 insightIntentParam_ 中的 "formID" 展开到 want 顶层
```

### 2.3 安全约束

| 约束 | 说明 |
|------|------|
| UID 校验 | 仅 FOUNDATION_UID 可调用，拒绝其他进程 |
| 权限校验 | 使用 `specifyTokenId` 作为 `CheckCallerPermission` 的身份参数 |
| 跨用户校验 | `CheckCrossUserPermission` 确保跨用户场景合规 |
| 系统 key 过滤 | 合并 wantParams 时过滤 `ohos.insightIntent.*` 前缀，防止注入 |

---

## 3. 接口设计

### 3.1 IPC 接口定义

```cpp
// AbilityManagerInterfaceCode 新增
EXECUTE_INTENT_WITH_SPECIFY_TOKEN_ID = 1147,
```

### 3.2 接口声明

```cpp
/**
 * @brief Execute intent with specified token id.
 * @param key The key of intent executing client.
 * @param hostClient The host client carrying execute-done callback context.
 * @param param The Intent execute param.
 * @param wantParams The raw want params, merged into insightIntentParam_.
 * @param specifyTokenId The specified token id for permission checking.
 * @param callerToken The caller ability token.
 * @return Returns ERR_OK on success, others on failure.
 */
virtual int32_t ExecuteIntentWithSpecifyTokenId(uint64_t key, const sptr<IRemoteObject> &hostClient,
    const InsightIntentExecuteParam &param, const WantParams &wantParams,
    uint64_t specifyTokenId, const sptr<IRemoteObject> &callerToken) = 0;
```

### 3.3 参数说明

| 参数 | 类型 | 说明 |
|------|------|------|
| `key` | uint64_t | 意图执行客户端的回调 key |
| `hostClient` | sptr\<IRemoteObject\> | 持有 execute-done 回调上下文的宿主客户端 |
| `param` | InsightIntentExecuteParam | 意图执行参数（bundleName、intentName、executeMode、userId 等） |
| `wantParams` | WantParams | FMS 传入的原始 want 参数，合并到 insightIntentParam_ |
| `specifyTokenId` | uint64_t | 指定的 tokenId，用于权限校验（区别于 caller 的 token） |
| `callerToken` | sptr\<IRemoteObject\> | 调用方 ability token，用于前后台校验和 caller 信息更新 |

### 3.4 返回值

| 返回值 | 含义 |
|--------|------|
| ERR_OK | 意图执行成功触发 |
| CHECK_PERMISSION_FAILED | 非 FOUNDATION_UID 调用或无权限 |
| ERR_INVALID_VALUE | 参数无效 |
| 其他 | 透传 `StartAbilityInner` / `ExecuteIntentCommon` 的失败码 |

### 3.5 与已有接口的对比

| 对比维度 | `StartAbilityWithSpecifyTokenId` | `ExecuteIntentWithSpecifyTokenId` |
|---------|--------------------------------|----------------------------------|
| 用途 | 直接拉起 UIAbility（router 事件） | 执行意图后拉起 UIAbility（intent 事件） |
| 调用方 | FMS (FOUNDATION_UID) | FMS (FOUNDATION_UID) |
| 安全校验 | UID 校验 + specifyTokenId 权限 | UID 校验 + specifyTokenId 权限 |
| 额外参数 | — | wantParams（卡片自定义参数合并） |
| 执行链路 | StartAbilityInner | ExecuteIntentCommon → StartAbilityWithInsightIntent |
| Intent 处理 | 无 | 完整的 InsightIntent 提取、校验、生成 Want 流程 |

---

## 4. 可测试性设计

### 4.1 单元测试分层

| 测试层次 | 测试内容 | 测试方法 |
|---------|---------|---------|
| Stub 反序列化 | `ExecuteIntentWithSpecifyTokenIdInner` 参数解析 | Mock Stub + 验证反序列化正确性 |
| Proxy 序列化 | 6 个参数的 IPC 写入 | Mock Proxy + 验证 MessageParcel 写入 |
| Service 逻辑 | `ExecuteIntentWithSpecifyTokenId` 完整流程 | Mock 依赖 + 验证各分支 |
| ExecuteIntentCommon | `ExpandCardFormIdentityToWantTop` 展开逻辑 | 验证 want 顶层参数 |
| 权限校验 | UID 校验、specifyTokenId 权限 | Mock IPCSkeleton + PermissionVerification |

### 4.2 测试用例设计

#### 4.2.1 正常流程

| 用例编号 | 场景 | 验证点 |
|---------|------|-------|
| TC-001 | 卡片前台意图执行（UI_ABILITY_FOREGROUND） | intent 正确执行，UIAbility 被拉起，want 携带 formId |
| TC-002 | 卡片前台意图执行（含 wantParams 合并） | wantParams 正确合并到 insightIntentParam_，ohos.insightIntent.* 前缀被过滤 |
| TC-003 | 卡片前台意图执行（callerToken 非空） | callerToken 被传入 StartAbilityInner，UpdateCallerInfoFromToken 被调用 |
| TC-004 | 卡片前台意图执行（specifyTokenId 生效） | 权限校验使用 specifyTokenId 而非 callerToken 的 tokenId |

#### 4.2.2 异常流程

| 用例编号 | 场景 | 验证点 |
|---------|------|-------|
| TC-005 | 调用方 UID ≠ FOUNDATION_UID | 返回 CHECK_PERMISSION_FAILED |
| TC-006 | hostClient 为 nullptr | Stub 返回 ERR_INVALID_VALUE |
| TC-007 | param 反序列化失败 | Stub 返回 ERR_INVALID_VALUE |
| TC-008 | 跨用户权限校验失败 | 返回 CheckCrossUserPermission 失败码 |
| TC-009 | CheckAndUpdateParam 失败 | 返回对应失败码，intent 不执行 |
| TC-010 | StartAbilityWithInsightIntent 失败 | 返回对应失败码，intent 被 Remove |

#### 4.2.3 边界场景

| 用例编号 | 场景 | 验证点 |
|---------|------|-------|
| TC-011 | wantParams 为空 | 直接跳过合并逻辑，不崩溃 |
| TC-012 | specifyTokenId = 0 | 退化为原有权限校验逻辑（走 specifiedFullTokenId） |
| TC-013 | callerToken 为 nullptr | callerInfo 更新走默认分支（UpdateCallerInfo） |
| TC-014 | 卡片 formId 参数不全 | ExpandCardFormIdentityToWantTop 不崩溃，want 无额外参数 |

### 4.3 Mock 化要求

所有已有测试 mock 文件已通过 `MOCK_METHOD6` 添加桩（`test/mock/` 下 10+ 个 mock 文件均更新），确保已有单测不因新增纯虚函数而编译失败。

### 4.4 验证命令

```bash
# 编译验证
./build.sh --product-name <product> --build-target ability_runtime

# 相关单测
./build.sh --product-name <product> --build-target ability_manager_service_first_test
./build.sh --product-name <product> --build-target ability_manager_stub_test
```

---

## 附录：变更文件清单

| 类别 | 文件 | 变更类型 |
|------|------|---------|
| IPC 接口定义 | `interfaces/inner_api/ability_manager/include/ability_manager_interface.h` | 新增纯虚函数 |
| IPC 协议码 | `interfaces/inner_api/ability_manager/include/ability_manager_ipc_interface_code.h` | 新增枚举值 |
| 客户端代理 | `interfaces/inner_api/ability_manager/include/ability_manager_client.h` | 新增声明 |
| Proxy 实现 | `services/abilitymgr/src/ability_manager_proxy.cpp` | 新增序列化 |
| Proxy 声明 | `services/abilitymgr/include/ability_manager_proxy.h` | 新增声明 |
| Stub 实现 | `services/abilitymgr/src/ability_manager_stub.cpp` | 新增反序列化 + 路由 |
| Stub 声明 | `services/abilitymgr/include/ability_manager_stub.h` | 新增 Inner 方法 |
| 核心服务 | `services/abilitymgr/src/ability_manager_service.cpp` | 新增实现 + 修改下游函数 |
| 服务声明 | `services/abilitymgr/include/ability_manager_service.h` | 新增声明 + 修改参数 |
| 客户端实现 | `services/abilitymgr/src/ability_manager_client.cpp` | 新增实现 |
| 数据结构 | `services/abilitymgr/include/insight_intent/extract_insight_intent_profile.h` | ExecuteIntentCommonOptions 扩展 |
| Execute 管理 | `services/abilitymgr/src/insight_intent/insight_intent_execute_manager.cpp` | 类型适配 |
| Mock 文件 | `test/mock/` 下 10+ 个文件 | 新增 MOCK_METHOD6 桩 |
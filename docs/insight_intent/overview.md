# InsightIntent 意图框架介绍

> 最后更新：2026-08-10
> 适用范围：`ability_ability_runtime` 仓库 `services/abilitymgr/{insight_intent,skill}`、`cli_tool_framework`、相关 `interfaces/` 与 `frameworks/`

本文档面向意图框架的维护者和上游贡献者，描述：
1. 意图框架的能力边界
2. 对外接口规范（应用侧 + SA/系统侧）
3. 对内接口规范（模块间契约）

---

## 1. 意图框架能做什么

### 1.1 定位

InsightIntent 是 OpenHarmony 的**系统级意图标准体系**，连接应用内的业务功能与系统调度。它解决三个问题：

| 问题 | 没有意图框架时 | 有意图框架后 |
|---|---|---|
| 系统如何知道应用提供了哪些可被调度的功能 | 各 SA 各自维护白名单，散乱 | 应用通过装饰器/配置声明，系统统一收集注册 |
| 系统如何用统一方式跨应用调度业务功能 | Want 隐式启动 + 包名硬编码 | 按 `bundleName + intentName` 调度，框架完成匹配、权限校验、生命周期管理 |
| 业务功能的元信息（输入/输出 schema、描述、图标）如何对外暴露 | 文档约定，运行时不可见 | 元信息存 KVStore，运行时可查询、可校验 |

### 1.2 核心能力

#### A. 意图声明（应用侧）

应用开发者通过两种方式声明意图：

- **装饰器模式**（推荐）：在 ArkTS 源码里用 `@InsightIntentLink`、`@InsightIntentPage`、`@InsightIntentEntry`、`@InsightIntentFunctionMethod`、`@InsightIntentForm` 装饰器标注类/方法
- **配置模式**：在 `module.json5` 同级目录的 `insight_intent.json` 配置文件里静态声明

声明信息经编译期处理 + 运行期解析后注册到系统。

#### B. 意图执行（系统侧 → 应用侧）

系统或第三方应用按 `(bundleName, moduleName, intentName)` 三元组触发意图，框架负责：

1. 从 KVStore / BundleMgr 查询意图元信息（目标 ability、schema、executeMode）
2. 校验调用方权限（基于 callerTokenId）
3. 按 `executeMode` 调度目标：
   - `UIABILITY_FOREGROUND`：拉起 UIAbility 到前台
   - `UIABILITY_BACKGROUND`：拉起后台 UIAbility
   - `UIEXTENSION_ABILITY`：拉起 UIExtension
   - `SERVICE_EXTENSION_ABILITY`：拉起 ServiceExtension
4. 通过 `InsightIntentExecuteCallback` 异步回传执行结果

#### C. 意图查询（系统侧 / 应用侧）

- 系统侧（SA）：通过 `AbilityManagerClient::GetInsightIntentInfoByBundleName` / `...ByIntentName` 查询
- 应用侧：通过 `InsightIntentContext` 在执行意图时获取上下文，或主动查询自身声明的意图

#### D. Skill 执行（基于意图的高阶能力）

`SkillExecute` 是意图的"代码级执行"模式，用于 AI/Agent 场景：

- 调用方传入 `(bundleName, moduleName, skillName, functionName, skillArgs)`
- 系统拉起目标进程，调用应用内指定的 ArkTS 函数（不是 ability）
- 通过 `ISkillExecuteCallback` 异步回传结果
- 支持超时管理、调用方死亡清理、目标进程死亡清理

#### E. Function 注册与查询（CliTool 体系）

将 InsightIntent 元信息以 `FunctionInfo` 形式注册到 `CliToolManagerService` 的 KVStore，供其他 SA（如 Agent 框架）查询和调用。

- 注册接口：`RegisterFunction` / `BatchRegisterFunctions` / `ResetNamespaceFunctions`
- 查询接口：`GetFunctionInfo` / `GetAllFunctions`
- 注销接口：`UnregisterFunction` / `UnregisterIntentFunctionsByNamespace`

### 1.3 不做什么（边界）

- **不做意图理解/NLP**：意图名是字符串字面量，框架不做语义解析
- **不做跨设备调度**：跨设备由 DMS 处理，框架只提供 `ExecuteIntentForDistributed` 入口
- **不做应用内导航**：UIAbility 内部的页面跳转由应用自己实现
- **不替代 Want**：意图执行底层仍然基于 Want，意图只是声明 + 元信息层

---

## 2. 架构概览

```
                          ┌─────────────────────────────────────────────────┐
                          │              应用进程（声明方 + 执行方）             │
                          │                                                 │
                          │  @InsightIntent* 装饰器  ←→  insight_intent.json5│
                          │           │                                     │
                          │           ▼                                     │
                          │  InsightIntentExecutor（应用侧执行入口）           │
                          │  InsightIntentContext（应用侧上下文）             │
                          └────────┬───────────────────────────┬────────────┘
                                   │ ExecuteIntent             │ Execute callback
                                   ▼                           ▲
┌──────────────────────────────────────────────────────────────────────────┐
│                          AbilityManagerService（系统调度方）                │
│                                                                          │
│   ExecuteIntent  GetInsightIntentInfo  ExecuteInAppSkill  ExecuteSkillDone│
│                        │                                                 │
│       ┌────────────────┼─────────────────┬──────────────────┐            │
│       ▼                ▼                 ▼                  ▼            │
│  InsightIntent     InsightIntent     SkillExecute      InsightIntent      │
│  ExecuteManager    EventMgr          Manager           DbCache           │
│  (执行调度)        (注册/卸载)        (代码级执行)      (元信息缓存)        │
│       │                │                 │                  │            │
│       └────────────────┴─────────────────┴──────────────────┘            │
│                        │                                                 │
│                        ▼                                                 │
│                CliTool::UnregisterInsightIntentFunctions / Reset...       │
│                        │                                                 │
└────────────────────────┼─────────────────────────────────────────────────┘
                         │ IPC
                         ▼
┌──────────────────────────────────────────────────────────────────────────┐
│              CliToolManagerService（Function 注册中心）                    │
│                                                                          │
│   RegisterFunction / BatchRegisterFunctions / ResetNamespaceFunctions    │
│   GetAllFunctions / GetFunctionInfo / UnregisterFunction                 │
│                                                                          │
│   CliFunctionDataManager ←→ KVStore (cli_functions_db)                   │
└──────────────────────────────────────────────────────────────────────────┘
```

### 数据流

#### 注册流（应用安装/更新）

```
BundleMgr COMMON_EVENT_BUNDLE_SCAN_FINISHED
   → InsightIntentSysEventReceiver::LoadInsightIntentInfos
   → 读 module.json5 的 insight_intent.json5
   → InsightIntentDbCache::SaveInsightIntentTotalInfo（写 RDB）
   → CliTool::BatchRegisterInsightIntentFunctions（开机批量注册）
       → CliToolManagerService → KVStore
```

#### 卸载流（应用卸载）

```
AbilityBundleEventCallback::OnBundleStateChanged
   → InsightIntentEventMgr::DeleteInsightIntentEvent
       → InsightIntentDbCache::DeleteInsightIntentTotalInfo（删 RDB）
       → CliTool::UnregisterInsightIntentFunctions（删 KVStore）
```

#### 更新流（应用版本更新）

```
AbilityBundleEventCallback::OnBundleUpdated
   → InsightIntentEventMgr::UpdateInsightIntentEvent
   → 重新解析 profile → 更新 RDB
   → CliTool::ResetInsightIntentFunctions（原子 clear + write）
```

#### 执行流（系统触发意图）

```
SA: AbilityManagerClient::ExecuteIntent(key, callerToken, param)
   → AbilityManagerService::ExecuteIntent
   → InsightIntentExecuteManager::CheckAndUpdateParam
   → 拉 UIAbility/ServiceExtension
   → 应用进程：InsightIntentExecutor::HandleExecuteIntent
   → InsightIntentExecuteCallback::OnExecuteDone
   → InsightIntentExecuteManager::ExecuteIntentDone
```

---

## 3. 对外接口规范

### 3.1 应用开发者接口

#### 3.1.1 装饰器声明（ArkTS / ETS）

定义位置：`frameworks/js/napi/insight_intent/insight_intent_decorator/`

| 装饰器 | 用途 | 对应 InsightIntentType |
|---|---|---|
| `@InsightIntentLink` | 声明深链接意图 | `DECOR_LINK` |
| `@InsightIntentPage` | 声明页面导航意图 | `DECOR_PAGE` |
| `@InsightIntentEntry` | 声明入口意图（带 executeMode） | `DECOR_ENTRY` |
| `@InsightIntentFunctionMethod` | 声明函数式意图 | `DECOR_FUNC` |
| `@InsightIntentForm` | 声明卡片意图 | `DECOR_FORM` |

#### 3.1.2 InsightIntentContext

定义位置：`interfaces/inner_api/insight_intent/insight_intent_context/insight_intent_context.h`

应用在执行意图时通过该 Context 拉起同应用内的其他 ability：

```cpp
class InsightIntentContext final {
public:
    ErrCode StartAbilityByInsightIntent(const AAFwk::Want &want);
    std::string GetBundleName() const;
    int32_t GetCurrentWindowMode() const;
    // ...
};
```

#### 3.1.3 InsightIntentExecutor

定义位置：`interfaces/kits/native/ability/native/insight_intent_executor/`

应用进程内的执行入口，应用通过实现该抽象类响应意图执行：

```cpp
class InsightIntentExecutor {
public:
    virtual bool Init(const InsightIntentExecutorInfo& intentInfo) = 0;
    virtual bool HandleExecuteIntent(...) = 0;
    // ...
};
```

#### 3.1.4 配置模式（insight_intent.json5）

当不便使用装饰器时，可在模块配置里静态声明，schema 与装饰器一致：

```json5
{
  "intentName": "string",
  "moduleName": "string",
  "uiAbilityIntentInfo": { "abilityName": "string", "supportExecuteMode": [...] },
  "serviceExtensionIntentInfo": { ... },
  "inputParams": [...],
  "outputParams": [...]
}
```

### 3.2 SA / 系统侧接口（AbilityManagerClient）

定义位置：`interfaces/inner_api/ability_manager/include/ability_manager_interface.h`

所有 SA 调用走 `AbilityManagerClient::GetInstance()`，关键接口：

#### 3.2.1 意图执行

```cpp
// 完整参数版本（通用）
int32_t ExecuteIntent(uint64_t key,
                      const sptr<IRemoteObject> &callerToken,
                      const InsightIntentExecuteParam &param);

// 函数式调用（简化）
int32_t ExecuteIntentByFunctionCall(uint64_t key,
                                    const sptr<IRemoteObject> &callerToken,
                                    const std::string &bundleName,
                                    const std::string &intentName,
                                    const WantParams &wantParam);

// 分布式场景入口
int32_t ExecuteIntentForDistributed(const Want &want,
                                    const std::string &srcDeviceId,
                                    uint64_t requestCode,
                                    uint64_t specifiedFullTokenId = 0);

// 应用内 skill 代码执行（旧）
int32_t ExecuteInAppSkill(const std::string &bundleName,
                          const std::string &moduleName,
                          const std::string &skillName,
                          const std::string &arkTSPath = "",
                          const std::string &funcName = "",
                          const std::shared_ptr<AAFwk::WantParams> &skillArgs = nullptr,
                          const sptr<ISkillExecuteCallback> &callback = nullptr);

// 应用内 skill 代码执行（新，SA-to-SA 推荐）
int32_t ExecuteInAppSkillWithTokenId(const AppExecFwk::SkillExecuteRequest &request,
                                     const sptr<ISkillExecuteCallback> &callback);

// Skill 执行完成回调（应用侧 → 系统）
int32_t ExecuteSkillDone(const sptr<IRemoteObject> &token,
                         const std::string &requestCode,
                         int32_t resultCode,
                         const AppExecFwk::SkillExecuteResult &result);

// Skill 类型查询
int32_t QuerySkillType(const std::string &bundleName,
                       const std::string &moduleName,
                       const std::string &skillName,
                       int32_t &skillType);
```

#### 3.2.2 意图查询

```cpp
// 按 bundleName 查询全部意图
int32_t GetInsightIntentInfoByBundleName(GetInsightIntentFlag flag,
                                         const std::string &bundleName,
                                         std::vector<InsightIntentInfoForQuery> &infos,
                                         int32_t userId = DEFAULT_INVAL_VALUE);

// 按 (bundleName, moduleName, intentName) 精确查询
int32_t GetInsightIntentInfoByIntentName(GetInsightIntentFlag flag,
                                         const std::string &bundleName,
                                         const std::string &moduleName,
                                         const std::string &intentName,
                                         InsightIntentInfoForQuery &info,
                                         int32_t userId = DEFAULT_INVAL_VALUE);
```

`flag` 取值见 `insight_intent_constant.h::GetInsightIntentFlag`：

| 值 | 含义 |
|---|---|
| `GET_FULL_INSIGHT_INTENT = 1` | 完整意图元信息 |
| `GET_SUMMARY_INSIGHT_INTENT = 2` | 摘要（不含 schema） |
| `GET_ENTITY_INFO = 4` | 仅实体信息 |
| `GET_FULL_INSIGHT_INTENT_ENTITY = 5` | 完整 + 实体 |
| `GET_SUMMARY_INSIGHT_INTENT_ENTITY = 6` | 摘要 + 实体 |

### 3.3 CliTool Function 接口（SA 调用 CliToolManagerService）

定义位置：`cli_tool_framework/interfaces/cli_tool/ICliToolManager.idl`

通过 `CliToolMGRClient::GetInstance()` 调用，所有写接口要求 `FOUNDATION_UID + TOKEN_NATIVE`：

| 接口 | 用途 | 主要调用方 |
|---|---|---|
| `RegisterFunction(FunctionInfo)` | 单条注册 | 应用调试场景 |
| `BatchRegisterFunctions(FunctionsRawData, &count)` | 开机批量注册 | `InsightIntentSysEventReceiver` |
| `ResetNamespaceFunctions(ns, funcs, &count)` | 单 bundle 更新原子替换 | `InsightIntentEventMgr::UpdateInsightIntentEvent` |
| `UnregisterFunction(ns, name)` | 单条注销 | 极少使用 |
| `UnregisterIntentFunctionsByNamespace(ns)` | 按 bundle 全部注销 | `InsightIntentEventMgr::DeleteInsightIntentEvent` |
| `GetFunctionInfo(ns, name, &out)` | 单条查询 | Agent 框架 |
| `GetAllFunctions(&out)` | 全量查询 | Agent 框架 |

#### Function 注册数据结构

```cpp
// FunctionInfo 关键字段
struct FunctionInfo : public Parcelable {
    std::string functionName;        // intentName
    std::string functionNamespace;   // bundleName
    std::string description;
    std::string inputSchema;         // JSON schema
    std::string outputSchema;        // JSON schema
    FunctionType functionType;       // 必须为 INTENT_FUNCTION
    std::string version;             // bundleInfo.versionCode
};
```

### 3.4 应用侧异步回调

定义位置：`interfaces/inner_api/ability_manager/include/insight_intent/insight_intent_execute_callback_*.h`

```cpp
class IInsightIntentExecuteCallback : public IRemoteBroker {
public:
    virtual void OnExecuteDone(uint64_t intentId, int32_t resultCode,
                               const InsightIntentExecuteResult &result) = 0;
};
```

`SkillExecute` 用独立 callback：

```cpp
class ISkillExecuteCallback : public IRemoteBroker {
public:
    virtual void OnExecuteDone(const std::string &requestCode, int32_t resultCode,
                               const AppExecFwk::SkillExecuteResult &result) = 0;
};
```

---

## 4. 对内接口规范

对内接口指 `services/abilitymgr` 内部各 Manager 之间的契约，外部不应直接调用。

### 4.1 模块清单

| 模块 | 头文件 | 职责 |
|---|---|---|
| `InsightIntentEventMgr` | `services/abilitymgr/include/insight_intent/insight_intent_event_mgr.h` | 响应 bundle 安装/更新/卸载事件，维护 DB 与 KVStore 一致性 |
| `InsightIntentSysEventReceiver` | `.../insight_intent_sys_event_receiver.h` | 响应系统事件（开机扫描完成、用户切换、用户删除），开机批量注册 |
| `InsightIntentDbCache` | `.../insight_intent_db_cache.h` | RDB 缓存层，提供元信息的 CRUD |
| `InsightIntentExecuteManager` | `.../insight_intent_execute_manager.h` | 执行调度，连接 callerToken 与目标 ability |
| `InsightIntentExecuteCallbackStub/Proxy` | `.../insight_intent_execute_callback_*.h` | IPC 回调封装 |
| `SkillExecuteManager` | `.../skill/skill_execute_manager.h` | Skill 代码级执行调度 |
| `SkillExecuteCallbackStub/Proxy` | `.../skill/skill_execute_callback_*.h` | IPC 回调封装 |
| `function_call_convert` | `.../function_call_convert.h` | InsightIntent 元信息 → FunctionInfo 转换 + CliTool 调用封装 |

### 4.2 关键接口签名

#### InsightIntentEventMgr

```cpp
class InsightIntentEventMgr {
public:
    // 应用更新（重新解析 profile + Reset functions）
    void UpdateInsightIntentEvent(const AppExecFwk::ElementName &elementName, int32_t userId);
    // 应用卸载（删 DB + Unregister functions）
    void DeleteInsightIntentEvent(const AppExecFwk::ElementName &elementName,
                                  int32_t userId, int32_t appIndex);
    // 内部 helper：删 DB，不动服务端 function（清理由调用方决定）
    void DeleteInsightIntent(const std::string &bundleName,
                             const std::string &moduleName, int32_t userId);
    // 订阅 COMMON_EVENT_BUNDLE_SCAN_FINISHED 等系统事件
    void SubscribeSysEventReceiver();
};
```

#### InsightIntentDbCache

```cpp
class InsightIntentDbCache {
public:
    int32_t InitInsightIntentCache(int32_t userId);

    // 查询
    void GetInsightIntentInfoByName(const std::string &bundleName, int32_t userId,
                                    std::vector<ExtractInsightIntentInfo> &infos);
    void GetConfigInsightIntentInfoByName(const std::string &bundleName, int32_t userId,
                                          std::vector<InsightIntentInfo> &infos);
    void GetAllInsightIntentInfoForRegister(int32_t userId,
                                            std::vector<ExtractInsightIntentInfo> &infos,
                                            std::vector<InsightIntentInfo> &configInfos);

    // 写入
    int32_t SaveInsightIntentTotalInfo(const std::string &bundleName, const std::string &moduleName,
                                       int32_t userId, uint32_t versionCode,
                                       const ExtractInsightIntentProfileInfoVec &infos,
                                       const std::vector<InsightIntentInfo> &configInfos);

    // 删除
    int32_t DeleteInsightIntentTotalInfo(const std::string &bundleName,
                                         const std::string &moduleName, int32_t userId);
    int32_t DeleteInsightIntentByUserId(int32_t userId);

    // 状态查询
    bool HasInsightIntentByName(uint32_t versionCode, const std::string &bundleName, int32_t userId);
    bool HasBundleCache(const std::string &bundleName);
    bool IsCacheInitialized(int32_t userId);

    // 持久化
    void BackupRdb();
};
```

#### InsightIntentExecuteManager

```cpp
class InsightIntentExecuteManager {
public:
    // 入口：参数校验 + 调度
    int32_t CheckAndUpdateParam(uint64_t key, const sptr<IRemoteObject> &callerToken,
                                InsightIntentExecuteParam &param, Want &want);
    int32_t CheckAndUpdateWant(Want &want, AppExecFwk::ExecuteMode executeMode,
                               int32_t userId, uint64_t specifiedFullTokenId);

    // 执行生命周期
    int32_t ExecuteIntentDone(uint64_t intentId, int32_t resultCode,
                              const InsightIntentExecuteResult &result);
    int32_t RemoveExecuteIntent(uint64_t intentId);

    // 分布式 + 异常处理
    int32_t GetDistributedInfo(uint64_t intentId, std::string &srcDeviceId, int32_t &errCode);
    int32_t RemoteDied(uint64_t intentId);

    // 调用方信息
    int32_t GetBundleName(uint64_t intentId, std::string &bundleName) const;
    int32_t GetCallerBundleName(uint64_t intentId, std::string &callerBundleName) const;

    // 豁免名单
    void SetIntentExemptionInfo(int32_t uid);
    bool CheckIntentIsExemption(int32_t uid);
};
```

#### SkillExecuteManager

```cpp
class SkillExecuteManager {
public:
    // 生成 Want + 解析目标类型
    int32_t GenerateSkillWant(const AppExecFwk::SkillInfo &skillInfo, Want &want,
                              int32_t userId, const std::string &requestCode,
                              AppExecFwk::ExtensionAbilityType &targetType,
                              const std::string &scriptPath = "",
                              const std::string &functionName = "",
                              const std::shared_ptr<AAFwk::WantParams> &skillArgs = nullptr);

    // BundleMgr 查询 + 权限
    int32_t QuerySkillInfo(const std::string &bundleName, const std::string &moduleName,
                           const std::string &skillName, int32_t userId,
                           AppExecFwk::SkillInfo &skillInfo);
    int32_t CheckSkillPermission(const AppExecFwk::SkillInfo &skillInfo,
                                 uint64_t callerTokenId);

    // 执行生命周期 + 超时
    int32_t ExecuteSkillDone(const std::string &requestCode, int32_t resultCode,
                             const AppExecFwk::SkillExecuteResult &result);
    void OnTimeout(int64_t requestCodeSeq);
    void OnLaunchCompleted(const std::string &requestCode);
    void OnLaunchFailed(const std::string &requestCode, int32_t errCode);
    void OnTargetProcessDied(const std::string &bundleName, pid_t pid);
};
```

#### function_call_convert（CliTool 调用封装）

```cpp
namespace CliTool {

// 单 bundle 注册（旧）
bool RegisterInsightIntentFunctions(
    const std::vector<ExtractInsightIntentInfo> &intentInfos,
    const std::vector<InsightIntentInfo> &configInfos,
    const std::string &bundleName, uint32_t versionCode);

// 开机批量注册（跨 bundle 一次性写入）
bool BatchRegisterInsightIntentFunctions(
    const std::vector<ExtractInsightIntentInfo> &intentInfos,
    const std::vector<InsightIntentInfo> &configInfos,
    const std::unordered_map<std::string, uint32_t> &bundleVersionMap,
    int32_t &successCount);

// 单 bundle 更新（原子替换，替代 Unregister + BatchRegister）
bool ResetInsightIntentFunctions(
    const std::vector<ExtractInsightIntentInfo> &intentInfos,
    const std::vector<InsightIntentInfo> &configInfos,
    const std::string &bundleName, uint32_t versionCode,
    int32_t &successCount);

// 注销
bool UnregisterInsightIntentFunctions(const std::string &bundleName);

// 元信息过滤工具
class IntentFilterUtil {
public:
    void FilterConfig(std::vector<InsightIntentInfo> &configInfos);
    void FilterGeneric(std::vector<ExtractInsightIntentInfo> &intentInfos);
};

} // namespace CliTool
```

---

## 5. 接口契约规范

### 5.1 命名约定

| 元素 | 规范 | 示例 |
|---|---|---|
| 类名 | `InsightIntent*` / `Skill*` + 名词，PascalCase | `InsightIntentExecuteManager` |
| 方法名 | 动词开头，camelCase | `ExecuteIntent`、`GetInsightIntentInfoByBundleName` |
| 常量 | `INSIGHT_INTENT_*` 前缀，UPPER_SNAKE | `INSIGHT_INTENT_INTENT_NAME` |
| IDL 接口 | `I*` 前缀 | `ICliToolManager`、`IInsightIntentExecuteCallback` |
| 服务端 Stub 类 | `*Stub` 后缀 | `SkillExecuteCallbackStub` |
| 客户端 Proxy 类 | `*Proxy` 后缀 | `SkillExecuteCallbackProxy` |

### 5.2 参数传递规范

| 类型 | 规则 |
|---|---|
| `std::string` 输入 | `const std::string &`（避免拷贝） |
| `std::string` 输出 | `std::string &`（不返回引用） |
| 容器输入 | `const std::vector<T> &` |
| 容器输出 | `std::vector<T> &` |
| IPC 远程对象 | `const sptr<IXxx> &` |
| 大对象出参（避免拷贝） | `std::shared_ptr<T> &` |
| 简单类型（int32_t/bool/uint64_t） | 值传递 |
| 枚举 | 值传递（确保大小 ≤ int32_t） |

### 5.3 错误码规范

- 返回值统一用 `int32_t` 或 `ErrCode`，禁止 `bool` 表示错误（除非语义就是 yes/no）
- 成功：`ERR_OK (= 0)`
- 通用错误：用 `ability_manager_errors.h` 定义的 `ERR_*` 常量
- InsightIntent 专属错误：`ERR_INSIGHT_INTENT_*` 前缀
- SkillExecute 错误：`AppExecFwk::SkillExecuteResult::errCode` 字段
- 禁止用 `>= 0` 判断成功，必须显式 `== ERR_OK`

### 5.4 线程模型

| 入口 | 默认线程 | 是否需要异步化 |
|---|---|---|
| IPC Stub（`OnRemoteRequest`） | IPC 线程 | 重操作必须抛 ffrt |
| `CommonEventSubscriber::OnReceiveEvent` | COMMON 事件线程 | 重操作必须抛 ffrt |
| `AbilityBundleEventCallback` | bundle callback 线程 | 重操作必须抛 ffrt |
| 应用内 ffrt 任务 | ffrt worker | — |

**异步化要求**：任何调用 `CliTool::ResetInsightIntentFunctions` / `UnregisterInsightIntentFunctions`（跨 SA IPC）或 `InsightIntentDbCache::*`（DB I/O）的入口，必须用 `ffrt::submit` 包裹整体工作，确保调用方线程立即返回。

**顺序敏感场景**：`UpdateInsightIntentEvent` 路径里，`ResetInsightIntentFunctions`（原子 clear+write）必须发生在所有 `SaveInsightIntentTotalInfo` 之后，否则会清空刚写入的数据。

### 5.5 IPC 数据结构规范

所有需要 IPC 传递的结构体必须：
- 继承 `Parcelable`
- 实现 `Marshalling(Parcel &)` 和 `Unmarshalling(Parcel &)`
- 提供 `ReadFromParcel(Parcel &)`
- 包含 `INSIGHT_INTENT_*` 或 `SKILL_*` 前缀的字段需在 `insight_intent_constant.h` 统一定义字符串常量

### 5.6 权限校验规范

所有写操作（注册/注销 function、执行 skill）必须：
1. `IPCSkeleton::GetCallingUid() == FOUNDATION_UID`（仅 foundation SA 可调）
2. `AccessTokenKit::GetTokenTypeFlag(callerToken) == TOKEN_NATIVE`（排除应用/HAP 调用）
3. 业务侧权限（如 `ExecuteInAppSkillWithTokenId` 内的 `CheckSkillPermission`）独立校验

读操作（查询 function、查询 intent info）允许应用通过 SDK 调用，但需校验对应 `PERMISSION_*` 权限。

---

## 6. 相关文档与代码索引

### 6.1 关键代码入口

| 关注点 | 入口文件 |
|---|---|
| 系统服务端调度 | `services/abilitymgr/src/insight_intent/insight_intent_execute_manager.cpp` |
| 注册/卸载流程 | `services/abilitymgr/src/insight_intent/insight_intent_event_mgr.cpp` |
| 开机批量注册 | `services/abilitymgr/src/insight_intent/insight_intent_sys_event_receiver.cpp` |
| Skill 代码执行 | `services/abilitymgr/src/skill/skill_execute_manager.cpp` |
| Function 元信息转换 | `services/abilitymgr/src/insight_intent/function_call_convert.cpp` |
| Function 注册中心 | `cli_tool_framework/services/climgr/src/cli_tool_manager_service.cpp` |
| Function KVStore | `cli_tool_framework/services/climgr/src/cli_function_data_manager.cpp` |

### 6.2 对外接口定义

| 关注点 | 文件 |
|---|---|
| AMS 主接口 | `interfaces/inner_api/ability_manager/include/ability_manager_interface.h` |
| InsightIntent 数据结构 | `interfaces/inner_api/ability_manager/include/insight_intent/*.h` |
| Skill 数据结构 | `interfaces/inner_api/ability_manager/include/skill/*.h` |
| CliTool IDL | `cli_tool_framework/interfaces/cli_tool/ICliToolManager.idl` |
| 常量定义 | `interfaces/inner_api/ability_manager/include/insight_intent/insight_intent_constant.h` |

### 6.3 应用侧 SDK

| 关注点 | 文件 |
|---|---|
| 应用执行入口 | `interfaces/kits/native/ability/native/insight_intent_executor/` |
| ArkTS 装饰器 | `frameworks/js/napi/insight_intent/insight_intent_decorator/` |
| ArkTS 执行驱动 | `frameworks/js/napi/insight_intent/insight_intent_driver/` |
| ETS ANI 绑定 | `frameworks/ets/ani/insight_intent/` |

---

## 7. 术语表

| 术语 | 含义 |
|---|---|
| **InsightIntent** | 系统级意图，应用通过装饰器/配置声明的可调度业务功能 |
| **Skill** | 基于意图的代码级执行模式，定位 AI/Agent 场景，直接调用 ArkTS 函数 |
| **Function** | InsightIntent 元信息在 CliTool KVStore 里的存储形式，供其他 SA 查询 |
| **executeMode** | 意图执行的目标 ability 类型 + 模式（前台/后台 UIAbility、Extension） |
| **IntentType** | 装饰器类型（Link/Page/Entry/Function/Form） |
| **callerTokenId** | 调用方的 HAP Token ID，用于权限校验和 caller 身份追踪 |
| **requestCode** | Skill 执行的全局唯一序列号，用于追踪单次执行 |
| **namespace** | CliTool KVStore 里的 function 分组键，对 InsightIntent 等于 bundleName |

# Skill Execution Framework

本文档描述 Skill 执行框架的完整传递路径，供其他 Agent 阅读理解。

## 概述

Skill 是应用内业务功能的执行单元，支持在目标应用的 UIAbility 或 ServiceExtension 中执行指定的 ArkTS 函数。框架提供两种入口：JS API 和 aa 命令行。

### Skill 类型

| 类型 | 值 | 说明 |
|------|----|------|
| APP_SKILL | 0 | 应用内 Skill，在目标应用进程中执行 |
| INDEPENDENT_SKILL | -1 | 独立 Skill，预留拓展，当前不支持 |

当前仅支持 APP_SKILL（应用内 Skill）。调用时会先通过 `QuerySkillType` IPC 查询类型，独立 Skill 返回 `ERR_SKILL_TYPE_INDEPENDENT_NOT_SUPPORTED (35700012)`。

---

## 两个入口

### 入口 1：JS API

```
cliManager.executeSkill({
    cmd: "ohos-arktsscript",
    bundleName: "com.example",
    moduleName: "entry",
    skillName: "mySkill",
    arkTSPath: "",          // 可选
    funcName: "myFunc",     // 可选
    arg1: 123,              // 任意类型参数
    arg2: "hello"
})
```

### 入口 2：aa 命令行

```bash
aa execSkill -b com.example -m entry -s mySkill [-p arkTSPath] [-f funcName] [-a arg1] [-a arg2]
```

aa 命令的 `-a` 参数仅支持 string，按位置序号转为 WantParams（key 为 "0", "1", ...）。

---

## 核心数据结构

### SkillExecuteParam

定义文件：`interfaces/inner_api/ability_manager/include/skill/skill_execute_param.h`

```
SkillExecuteParam {
    bundleName_   : string              // 目标应用包名
    moduleName_   : string              // 目标模块名
    skillName_    : string              // Skill 名称
    arkTSPath_    : string              // ABC 路径（可选）
    funcName_     : string              // 导出函数名（可选）
    skillArgs_    : shared_ptr<WantParams>  // 调用参数，支持多类型
    srcEntries_   : vector<string>      // 源码入口列表
    requestCode_  : int64_t             // 请求编号
}
```

### SkillExecuteResult

定义文件：`interfaces/inner_api/ability_manager/include/skill/skill_execute_result.h`

```
SkillExecuteResult {
    code    : int32_t               // 结果码
    result  : shared_ptr<WantParams> // 返回数据
    uris    : vector<string>         // URI 列表
    flags   : uint32_t              // 标志位
}
```

### Want 参数 Key

所有 Skill 参数通过 Want 的 WantParams 传递，使用以下 key：

| 常量名 | Key 值 | 类型 | 说明 |
|--------|--------|------|------|
| `SKILL_EXECUTE_PARAM_BUNDLE_NAME` | `ohos.skill.executeParam.bundleName` | String | 目标包名 |
| `SKILL_EXECUTE_PARAM_MODULE_NAME` | `ohos.skill.executeParam.moduleName` | String | 目标模块 |
| `SKILL_EXECUTE_PARAM_SKILL_NAME` | `ohos.skill.executeParam.skillName` | String | Skill 名称 |
| `SKILL_EXECUTE_PARAM_ARKTS_PATH` | `ohos.skill.executeParam.arkTSPath` | String | ABC 路径 |
| `SKILL_EXECUTE_PARAM_FUNC_NAME` | `ohos.skill.executeParam.funcName` | String | 函数名 |
| `SKILL_EXECUTE_PARAM_ARGS_KEYS` | `ohos.skill.executeParam.argsKeys` | String | 参数 key 列表（分号分隔） |
| `SKILL_EXECUTE_PARAM_ARGS_PREFIX` | `ohos.skill.executeParam.args.<key>` | 多类型 | 各参数值 |
| `SKILL_EXECUTE_PARAM_SRC_ENTRIES_COUNT` | `ohos.skill.executeParam.srcEntriesCount` | String | srcEntries 数量 |
| `SKILL_EXECUTE_PARAM_SRC_ENTRY_PREFIX` | `ohos.skill.executeParam.srcEntry.<i>` | String | srcEntry |
| `SKILL_EXECUTE_PARAM_REQUEST_CODE` | `ohos.skill.executeParam.requestCode` | String | 请求编号 |

### skillArgs 多类型支持

`skillArgs_` 是 `WantParams`（本质是 `std::map<string, sptr<IInterface>>`），支持以下类型：

| JS 类型 | napi 类型 | WantParams 包装类 | 示例 |
|---------|-----------|-------------------|------|
| string | napi_string | `AAFwk::String` | `"hello"` |
| number (整数) | napi_number | `AAFwk::Integer` | `123` |
| number (浮点) | napi_number | `AAFwk::Float` | `3.14` |
| boolean | napi_boolean | `AAFwk::Boolean` | `true` |
| bigint | napi_bigint | `AAFwk::Long` | `999n` |

---

## IPC 接口

### IPC 枚举码

定义文件：`interfaces/inner_api/ability_manager/include/ability_manager_ipc_interface_code.h`

| 枚举名 | 值 | 说明 |
|--------|-----|------|
| `EXECUTE_IN_APP_SKILL` | 6169 | 执行应用内 Skill |
| `EXECUTE_IN_APP_SKILL_DONE` | 6170 | Skill 执行完成回调 |
| `QUERY_SKILL_TYPE` | 6171 | 查询 Skill 类型 |

### 调度器 IPC 码

| 枚举名 | 说明 |
|--------|------|
| `SCHEDULE_EXECUTE_SKILL` | AMS → App，执行 Skill（通过 AbilitySchedulerProxy/Stub） |

---

## 完整传递路径

### 阶段 1：JS API 入口解析

```
js_cli_manager.cpp::OnExecuteSkill
│
├─ 校验 cmd == "ohos-arktsscript"
├─ 提取 bundleName / moduleName / skillName（必填）
├─ 提取 arkTSPath / funcName（可选）
├─ ExtractSkillArgs(): 遍历 JS 对象属性
│  跳过保留 key（cmd, bundleName, moduleName, skillName, arkTSPath, funcName）
│  按 napi 类型装箱为 WantParams:
│    napi_string  → String::Box(str)
│    napi_number  → Integer::Box(int) 或 Float::Box(float)
│    napi_boolean → Boolean::Box(bool)
│    napi_bigint  → Long::Box(int64)
│  结果: skillArgs = WantParams { "arg1": Integer(123), "arg2": String("hello") }
│
└─ [异步线程] NapiAsyncTask::ExecuteCallback
   ├─ ① QuerySkillType IPC → AMS 查询 Skill 类型
   │  ├─ skillType == -1 → 返回 ERR_SKILL_TYPE_INDEPENDENT_NOT_SUPPORTED
   │  └─ skillType == 0  → 继续
   └─ ② ExecuteInAppSkill IPC → 进入阶段 2
```

### 阶段 1-alt：aa 命令行入口解析

```
ability_command.cpp::RunAsExecSkillCommand
│
├─ getopt 解析: -b bundleName -m moduleName -s skillName [-p path] [-f func] [-a argv...]
├─ argv 转 WantParams: key 为位置序号 "0", "1", ...，值全为 String 类型
└─ ExecuteInAppSkill IPC → 进入阶段 2
   注意: aa 路径无 QuerySkillType 检查
```

### 阶段 2：ExecuteInAppSkill IPC（Client → AMS）

```
AbilityManagerClient::ExecuteInAppSkill(b, m, s, path, func, args)
│
├─ [Proxy 序列化]
│  WriteInterfaceToken()
│  WriteString16(bundleName) + WriteString16(moduleName) + WriteString16(skillName)
│  WriteString16(arkTSPath) + WriteString16(funcName)
│  WriteParcelable(skillArgs)  ← WantParams 整体序列化
│
├─ SendRequest(EXECUTE_IN_APP_SKILL = 6169)
│
└─ [Stub 反序列化]
   ReadString16×5 → bundleName / moduleName / skillName / arkTSPath / funcName
   ReadParcelable<WantParams> → skillArgs
   → 调用 AMS::ExecuteInAppSkill
```

### 阶段 3：AMS 服务端处理

```
AMS::ExecuteInAppSkill (ability_manager_service.cpp:14331)
│
├─ ① QuerySkillInfo — 查询 bundle framework 获取 SkillInfo 配置
│  SkillExecuteManager::QuerySkillInfo()
│  → SkillManagerProxy::GetSkillInfo(bundleName, moduleName, skillName, flags, userId)
│  → 返回 SkillInfo { bundleName, moduleName, skillName, abilityName, type,
│                     srcEntries, permissions }
│
├─ ② CheckSkillPermission — 校验调用者权限
│  遍历 skillInfo.permissions，逐个 VerifyCallingPermission
│
├─ ③ CreateExecuteRecord — 创建执行记录，分配唯一 requestCode
│
├─ ④ GenerateSkillWant — 构造目标 Want
│  ├─ abilityName: 优先用 skillInfo.abilityName，否则 ResolveDefaultAbilityName
│  ├─ ResolveTargetType: 检查目标是 UIAbility 还是 ServiceExtension
│  ├─ want.SetElementName(bundleName, abilityName, moduleName)
│  └─ SkillExecuteParam::WriteToWant: 将参数写入 Want
│     ├─ 写入元数据: bundleName / moduleName / skillName / arkTSPath / funcName
│     ├─ 写入 skillArgs: 遍历 WantParams，每个 key 加 "args." 前缀
│     │  如 "arg1" → "ohos.skill.executeParam.args.arg1": Integer(123)
│     │  同时记录 argsKeys = "arg1;arg2"
│     ├─ 写入 srcEntries: srcEntry.0, srcEntry.1, ...
│     └─ 写入 requestCode
│
└─ ⑤ 按目标类型分发
   ├─ targetType == SERVICE → StartExtensionAbilityWithSkill → 阶段 4-ext
   └─ targetType == 其他   → StartAbilityByCallWithSkill → 阶段 4-ui
```

### 阶段 4-ui：UIAbility 分发（热启动 / 冷启动）

```
AMS::StartAbilityByCallWithSkill (ability_manager_service.cpp:14380)
│
├─ 创建 AbilityBackgroundConnection
├─ GenerateAbilityRequest → 解析 Want 中的 AbilityInfo
│
├─ IsAbilityStarted(abilityRequest, targetRecord, userId)
│  │
│  ├─ true → 热启动路径 ───────────────────────────────┐
│  │  targetRecord->GetScheduler()->ExecuteSkill(want)   │
│  │                                                     │
│  └─ false → 冷启动路径                                │
│     StartAbilityByCall(want, connect, callerToken)     │
│     → 进程孵化 → 生命周期推进                          │
│     → STARTED_NEW → FOREGROUND_NEW → BACKGROUND_NEW   │
│     → 在 BACKGROUND_NEW 状态检测到 IsSkillExecute      │
│     → HandleExecuteSkill(want, onlyExecuteSkill=false) │
│                                                        │
│  两条路径最终汇合 ↓                                    ↓
└─ 目标 App 进程收到 ExecuteSkill
```

### 阶段 5：IPC 到目标 App 进程

```
[系统侧] AbilitySchedulerProxy::ExecuteSkill (ability_scheduler_proxy.cpp:1151)
│  ├─ 异步 IPC: MessageOption(TF_ASYNC)
│  ├─ 序列化 Want: data.WriteParcelable(&want)
│  └─ SendTransactCmd(SCHEDULE_EXECUTE_SKILL)
│
│  Binder 驱动
│
[App侧] AbilitySchedulerStub::ExecuteSkillInner (ability_scheduler_stub.cpp:729)
   ├─ 反序列化: want = data.ReadParcelable<Want>()
   └─ ExecuteSkill(*want) → UIAbilityThread::ExecuteSkill
```

### 阶段 6：App 进程线程调度

```
UIAbilityThread::ExecuteSkill (ui_ability_thread.cpp:697)
│
├─ 检查 abilityImpl_ 和 abilityHandler_ 非空
├─ 构造 lambda 任务（弱引用防悬空，want 按值捕获）
├─ PostTask 到主线程事件队列
└─ → UIAbilityImpl::HandleExecuteSkill(want, onlyExecuteSkill)
     │  热启动: onlyExecuteSkill = true（不触发 Background）
     │  冷启动: onlyExecuteSkill = false（执行完调 Background）
     │
     ├─ SkillExecuteParam::GenerateFromWant(want, param)
     │  从 Want 的 WantParams 中提取:
     │  ├─ bundleName / moduleName / skillName / arkTSPath / funcName
     │  ├─ requestCode
     │  ├─ skillArgs: 读取 argsKeys，按 ";" 分割遍历
     │  │  从 "ohos.skill.executeParam.args.<key>" 提取，类型不变
     │  └─ srcEntries
     │
     └─ ability_->ExecuteSkill(want, param) → JsUIAbility::ExecuteSkill
```

### 阶段 7：JS 执行

```
JsUIAbility::ExecuteSkill (js_ui_ability.cpp:2516)
│
├─ ① 获取 napi_env
│
├─ ② 加载 ArkTS 模块
│  modulePath = moduleName + "/ets/modules.abc"
│  jsRuntime_.LoadModule() — 首次加载 ABC，后续返回缓存
│  失败 → ReportSkillDone(requestCode, -1)
│
├─ ③ 查找导出函数
│  GetPropertyValueByPropertyName(env, jsObj, funcName, napi_function)
│  找不到 → ReportSkillDone(requestCode, -1)
│
├─ ④ 构造 JS 参数 1: scriptInfo
│  CreateArkTSScriptInfo(env, requestCode)
│  → { requestCode: BigInt, context: UIAbilityContext }
│
├─ ⑤ 构造 JS 参数 2: skillArgs
│  WrapWantParams(env, *param->skillArgs_)
│  → { "arg1": 123, "arg2": "hello", "arg3": true, "arg4": 999n }
│  类型映射:
│    String  → napi_string
│    Integer → napi_number
│    Float   → napi_number
│    Boolean → napi_boolean
│    Long    → napi_bigint
│
├─ ⑥ 调用目标 JS 函数
│  napi_call_function(env, jsObj, method, argc=2, [scriptInfo, skillArgs])
│
│  用户 JS 函数签名:
│  function myFunc(scriptInfo: { requestCode: bigint, context }, args: object)
│
└─ ⑦ 等待异步回调（不阻塞）
```

### 阶段 8：结果回调

```
用户 JS 完成后调用:
cliManager.completeArkTSScriptInApp(requestCode, resultCode, result)
│
js_cli_manager.cpp::OnCompleteArkTSScriptInApp
│
├─ ParseRequestCode: 解析 requestCode (number / bigint)
├─ UnwrapSkillExecuteResult: 提取 { code, result, uris, flags }
│
└─ [异步线程] AbilityManagerClient::ExecuteSkillDone(requestCode, resultCode, result)
   │
   IPC: EXECUTE_IN_APP_SKILL_DONE (6170)
   │  序列化: WriteInt64(requestCode) + WriteParcelable(SkillExecuteResult)
   │
   └─ AMS::ExecuteSkillDone (ability_manager_service.cpp:14428)
      SkillExecuteManager::ExecuteSkillDone(requestCode, resultCode, result)
      ├─ 查找 requestCode 对应的 SkillExecuteRecord
      ├─ 更新 state = COMPLETED
      └─ 记录 resultCode 和 result
```

---

## 数据流转全景

```
JS API 调用:
{ cmd: "ohos-arktsscript", bundleName: "com.x", skillName: "s", arg1: 123, arg2: "hi" }
       │
       ↓ ExtractSkillArgs (js_cli_manager.cpp)
WantParams: { "arg1": Integer(123), "arg2": String("hi") }
       │
       ↓ IPC: EXECUTE_IN_APP_SKILL (WriteParcelable)
[Client → AMS] skillArgs 透传
       │
       ↓ SkillExecuteParam::WriteToWant (skill_execute_param.cpp)
Want.WantParams: {
  "ohos.skill.executeParam.bundleName": String("com.x"),
  "ohos.skill.executeParam.skillName":  String("s"),
  "ohos.skill.executeParam.argsKeys":   String("arg1;arg2"),
  "ohos.skill.executeParam.args.arg1":  Integer(123),
  "ohos.skill.executeParam.args.arg2":  String("hi"),
  "ohos.skill.executeParam.requestCode": String("42")
}
       │
       ↓ IPC: SCHEDULE_EXECUTE_SKILL (Want 整体序列化)
[AMS → App] WantParams 类型不变
       │
       ↓ SkillExecuteParam::GenerateFromWant (skill_execute_param.cpp)
param.skillArgs_ = WantParams { "arg1": Integer(123), "arg2": String("hi") }
       │
       ↓ WrapWantParams (napi_common_want.cpp)
JS 对象: { arg1: 123, arg2: "hi" }
       │
       ↓ napi_call_function
目标 JS 函数: myFunc(scriptInfo, args)
```

---

## 关键文件索引

| 文件 | 职责 |
|------|------|
| `cli_tool_framework/.../js_cli_manager.cpp` | JS API 入口，参数解析，类型判断 |
| `cli_tool_framework/.../cli_error_code.h` | 错误码定义，含 ERR_SKILL_TYPE_INDEPENDENT_NOT_SUPPORTED |
| `tools/aa/src/ability_command.cpp` | aa 命令行入口 |
| `interfaces/inner_api/.../skill/skill_execute_param.h/.cpp` | Skill 参数数据结构，Want 序列化/反序列化 |
| `interfaces/inner_api/.../skill/skill_execute_result.h/.cpp` | Skill 执行结果数据结构 |
| `interfaces/inner_api/.../ability_manager_ipc_interface_code.h` | IPC 枚举码定义 |
| `interfaces/inner_api/.../ability_manager_interface.h` | IPC 接口虚方法声明 |
| `interfaces/inner_api/.../ability_manager_client.h/.cpp` | Client 侧 IPC 调用 |
| `services/abilitymgr/.../ability_manager_proxy.h/.cpp` | Proxy 序列化 |
| `services/abilitymgr/.../ability_manager_stub.h/.cpp` | Stub 反序列化与路由 |
| `services/abilitymgr/.../ability_manager_service.h/.cpp` | AMS 服务端核心逻辑 |
| `services/abilitymgr/.../skill/skill_execute_manager.h/.cpp` | Skill 执行管理器（查询、权限、Want 生成） |
| `services/abilitymgr/.../ability_scheduler_proxy.cpp` | App 调度器 Proxy（AMS→App IPC） |
| `services/abilitymgr/.../ability_scheduler_stub.cpp` | App 调度器 Stub（App 侧接收） |
| `frameworks/native/ability/native/ui_ability_thread.cpp` | UIAbility 线程调度 |
| `frameworks/native/ability/native/ui_ability_impl.cpp` | UIAbility 实现，生命周期中 Skill 检测 |
| `frameworks/native/ability/native/ability_runtime/js_ui_ability.cpp` | JS 执行层，LoadModule + napi_call_function |

---

## 热启动 vs 冷启动对比

| | 热启动 | 冷启动 |
|---|---|---|
| **触发条件** | `IsAbilityStarted == true` | `IsAbilityStarted == false` |
| **IPC 方式** | 直接 `SCHEDULE_EXECUTE_SKILL` | `StartAbilityByCall` → 生命周期推进 |
| **执行时机** | 主线程 PostTask 立即执行 | 生命周期到达 `BACKGROUND_NEW` 时 |
| **onlyExecuteSkill** | `true`，不调 Background() | `false`，执行完调 Background() |
| **Ability 状态** | 保持原状态不变 | 前台 → 执行 Skill → 回到后台 |
| **参数提取** | `GenerateFromWant` — 相同 | `GenerateFromWant` — 相同 |
| **JS 执行** | `JsUIAbility::ExecuteSkill` — 相同 | `JsUIAbility::ExecuteSkill` — 相同 |

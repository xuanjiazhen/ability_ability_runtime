# 意图框架同步到 FunctionCall 实现

## 接口说明

提供三个核心接口（位于 `function_call_convert.h/cpp`）：

1. **装饰器类转换** — `ConvertFromExtractProfile(profileInfos, functions)`
   - 读取 `@InsightIntentFunctionMethod` 装饰器条目
   - 字段完整（functionName、parameters、result 均为 JSON Schema）

2. **配置类转换** — `ConvertFromConfigIntent(configInfos, functions)`
   - 读取 `insightIntents` 配置条目
   - 从 `intentName`、`inputParams`、`outputParams` 推导
   - `inputSchema` = `{"type":"object","properties":{"param":{"type":"string"}}}`

3. **注册到 FunctionCall** — `RegisterInsightIntentFunctions(profileInfos, configInfos)`
   - 内部 = `ConvertFromExtractProfile` + `ConvertFromConfigIntent` + `AddInsightIntentOptions` + `CliToolMGRClient::RegisterFunction`（IPC）

---

## 文件清单

### 新增文件

| 文件 | 说明 |
|------|------|
| `services/abilitymgr/include/insight_intent/function_call_convert.h` | 转换 + 注册函数声明 |
| `services/abilitymgr/src/insight_intent/function_call_convert.cpp` | 实现 |

### 修改文件

| 文件 | 修改内容 |
|------|----------|
| `services/abilitymgr/src/insight_intent/insight_intent_event_mgr.cpp` | PACKAGE_ADDED/CHANGED: `UpdateInsightIntentEvent` 中调 `RegisterInsightIntentFunctions(infos, configIntentInfos)` |
| `services/abilitymgr/src/insight_intent/insight_intent_sys_event_receiver.cpp` | 启动扫描: `SaveInsightIntentInfos` 中调 `RegisterInsightIntentFunctions(infos, configIntentInfos)` |
| `services/abilitymgr/abilitymgr.gni` | 添加 `function_call_convert.cpp` 到源码列表 |
| `services/abilitymgr/BUILD.gn` | 添加 cli_tool include_dirs + `cli_tool_client` deps |

---

## 数据流

```
触发事件                    hook 位置                             数据流向
─────────────────────────────────────────────────────────────────────────────
PACKAGE_ADDED         →  ability_bundle_event_callback.cpp:93
                          → UpdateInsightIntentEvent()
                            → SaveInsightIntentTotalInfo()         Intent RDB
                            → RegisterInsightIntentFunctions()     FunctionCall KVStore

PACKAGE_CHANGED       →  ability_bundle_event_callback.cpp:113
                          → UpdateInsightIntentEvent()            同上

BUNDLE_SCAN_FINISHED  →  insight_intent_sys_event_receiver.cpp:230
                          → LoadInsightIntentInfos()
                            → SaveInsightIntentInfos()
                              → SaveInsightIntentTotalInfo()
                              → RegisterInsightIntentFunctions()

USER_SWITCHED         →  insight_intent_sys_event_receiver.cpp:232
                          → LoadInsightIntentInfos()              同上
```

---

## 字段映射

### 装饰器类 (extractInsightIntents)

```
JSON 字段                                  FunctionInfo
────────────────────────────────────────────────────────────────
functionName        ──────────────────────  functionName
bundleName                                   bundleName
displayDescription  ──────────────────────  description
parameters (JSON obj → string)  ──────────  inputSchema （完整 JSON Schema）
result (JSON obj → string)      ──────────  outputSchema（完整 JSON Schema）
                                              functionType = INTENT_FUNCTION
```

### 配置类 (insightIntents)

```
JSON 字段                                  FunctionInfo
────────────────────────────────────────────────────────────────
intentName          ──────────────────────  functionName
bundleName                                   bundleName
displayDescription  ──────────────────────  description
inputParams         ──────────────────────  inputSchema（推导, 所有类型标 string）
outputParams        ──────────────────────  outputSchema（推导, 所有类型标 string）
                                              functionType = INTENT_FUNCTION
```

---

---

## ExecuteIntentByFunctionCall 运行时 options 解析

### 新增改动

在 `ability_manager_service.cpp` 中：

| 位置 | 内容 |
|------|------|
| Line 117 | 新增 `#include "array_wrapper.h"`（IArray 操作） |
| Lines 14206-14257 | 新增静态辅助函数 `ApplyInsightIntentOptionsFromWantParam` |
| Line 14318 | 在 param 构造后调用该函数 |

### 逻辑

`ExecuteIntentByFunctionCall` 从 `wantParam` 中读取 `ohos.insightIntent.options`（嵌套 WantParams）：

```
ohos.insightIntent.options (WantParams) {
    executeMode: string          → param->executeMode_ (int32)
    uris:        string[]        → param->uris_ (vector<string>)
    flags:       string          → param->flags_ (int32)
}
```

- **executeMode**：支持命名模式（`UI_ABILITY_FOREGROUND`/`BACKGROUND`/`UI_EXTENSION_ABILITY`/`SERVICE_EXTENSION_ABILITY`）+ 数字回退（`strtol`）
- **uris**：通过 `IArray::Query` + `Array::IsStringArray` 校验 + `Array::ForEach` 迭代提取
- **flags**：通过 `strtol` 解析（base=0 自动检测进制，支持 `0x` 十六进制）
- 若 `ohos.insightIntent.options` 不存在则直接返回，不做任何覆盖

---

## 未处理场景

| 场景 | 原因 |
|------|------|
| PACKAGE_REMOVED | FunctionCall 暂无删除接口 |
| USER_REMOVED | KVStore 为 EL1，不区分用户 |

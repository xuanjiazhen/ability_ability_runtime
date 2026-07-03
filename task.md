# Insight Intent 数据库读写异常防护任务清单

## 背景

`insight_intent.db` 的读写路径涉及大量 JSON 反序列化（nlohmann::json）和 RDB API 调用，多个关键路径缺少 try-catch 兜底。一旦触发 C++ 异常，会沿调用栈传播到 ffrt task / IPC worker 顶层，最终触发 `std::terminate()` → `abort()` → **AMS（foundation）进程崩溃重启**。

**触发数据来源**：

1. 数据库里历史记录的 JSON 字段类型与当前代码定义不匹配（升级场景）
2. 应用 `extract_insight_intent.json` / `insight_intent.json` 配置错误
3. IPC 跨进程入参被恶意构造
4. 极端大对象 / 深嵌套 JSON 触发内存或递归限制

**当前代码已有防护**：

- `nlohmann::json::parse(str, nullptr, false)` + `is_discarded()` 拦截**语法层**错误
- `find() != end()` 拦截字段缺失
- 部分位置 `is_string()/is_array()/is_object()` 顶层类型检查
- 手动错误码 `g_parseResult` / `g_extraParseResult` 标记 BMSJsonUtil 内部已知错误

**当前防护覆盖不到的盲区**：

- 类型检查粒度不够（`is_number()` 不能区分 int32 / int64，数值溢出仍抛异常）
- 嵌套递归 `from_json` 失败传播
- `get<T>()` 默认抛异常，没有 `allow_exceptions=false` 等价机制
- `dump()` 在深嵌套 / 极大对象场景可能抛 `out_of_range` / `bad_alloc`

---

## 修复优先级总览

| 优先级 | 范围 | 触发场景 | 后果 |
|--------|------|---------|------|
| **P0 必修** | R1-R7, I1-I2 | DB 数据字段类型不匹配 / IPC 恶意入参 | AMS 崩溃（开机必崩） |
| **P1 建议** | I3 | `executeMode` 配置成非数组 | AMS 崩溃（配置错误场景） |
| **P2 可选** | dump 系列点 | 极大 JSON / 深嵌套 | 罕见场景崩溃 |
| **P3 忽略** | W1/W3, D1-D9 | bad_alloc / system_error 理论风险 | 现实概率极低 |

---

## 类别 W：写库路径风险点

**调用链**：业务 → `InsightIntentDbCache::SaveInsightIntentTotalInfo` → `InsightRdbStorageMgr::SaveStorageInsightIntentData` → `InsightIntentRdbDataMgr::InsertData` → RDB API

### W1：`ToJson` 隐式 to_json 转换 ⭐⭐

- **位置**：`extract_insight_intent_profile.cpp:694`
- **触发代码**：`nlohmann::json subJsonObject = info;`（由 `ToJson` 内部触发）
- **调用入口**：`insight_intent_rdb_storage_mgr.cpp:225, 240` 的 `ExtractInsightIntentProfile::ToJson` / `InsightIntentProfile::ToJson`
- **触发场景**：内存里的 `ExtractInsightIntentProfileInfo` / `InsightIntentInfo` 结构体字段异常
- **可能异常**：`std::bad_alloc`、to_json 实现内部抛
- **当前防护**：无
- **防护方案**：在 `ToJson` 函数内部包 try-catch，或调用点包 try-catch

### W2：写库前的 `jsonObject.dump()` ⭐⭐⭐

- **位置**：`insight_intent_rdb_storage_mgr.cpp:229, 244`
- **触发代码**：`InsertData(key, jsonObject.dump())`
- **触发场景**：极大 JSON 对象、深嵌套（>1000 层）
- **可能异常**：
  - `nlohmann::json::out_of_range.407`（深嵌套）
  - `std::bad_alloc`（超大字符串）
- **当前防护**：无
- **防护方案**：用 SafeDump 工具包装（见统一方案 2）

### W3：第二个 `ToJson` 调用 ⭐⭐

- **位置**：`insight_intent_rdb_storage_mgr.cpp:240`
- **触发代码**：`InsightIntentProfile::ToJson(configInfo, jsonObject)`
- **细节**：同 W1，走配置类 Intent 的 to_json 路径

### W4：第二个写库前 `dump()` ⭐⭐⭐

- **位置**：`insight_intent_rdb_storage_mgr.cpp:244`
- **细节**：同 W2，配置类 Intent 写入

### W5：`InsertWithConflictResolution` ⭐⭐

- **位置**：`insight_intent_rdb_data_mgr.cpp:336`
- **触发代码**：`rdbStore->InsertWithConflictResolution(...)`
- **触发场景**：SQLite 内部错误、磁盘满、文件锁冲突
- **可能异常**：通常返回错误码（不抛），但极少数场景可能抛 `std::system_error`
- **当前防护**：有 `E_SQLITE_CORRUPT` 重试 + Restore 兜底；有 `InsertWithRetry` 3 次重试
- **防护方案**：保持现状，已被重试机制覆盖

### W6：`ValuesBucket::PutString` ⭐

- **位置**：`insight_intent_rdb_data_mgr.cpp:107-108`
- **触发代码**：`valuesBucket.PutString(INTENT_KEY, key); valuesBucket.PutString(INTENT_VALUE, value);`
- **可能异常**：`std::bad_alloc`（理论存在）
- **防护方案**：无需额外防护

---

## 类别 R：读库路径风险点（最危险） ⭐⭐⭐⭐⭐

**调用链**：业务 → `InsightIntentDbCache::LoadInsightIntentInfos/GetAllInsightIntentInfo/...` → `InsightRdbStorageMgr::LoadXxx/Transform/TransformConfigIntent` → `InsightIntentRdbDataMgr::QueryXxx` → RDB API → **Transform/TransformTo 反序列化**

### R1：`get<ExtractInsightIntentProfileInfoVec>()` ⭐⭐⭐⭐⭐

- **位置**：`extract_insight_intent_profile.cpp:673`
- **触发代码**：`intentInfos = jsonObject.get<ExtractInsightIntentProfileInfoVec>();`
- **触发场景**：DB 里存的 JSON 字段类型与 C++ 结构体定义不匹配
  - 如 `intentVersion` 期望 string，DB 里是 number
  - 如 `executeMode` 期望 array，DB 里是 object
  - 如 `entities` 期望 array，DB 里是 string
- **可能异常**：
  - `nlohmann::json::type_error.302`（类型不匹配）
  - `nlohmann::json::out_of_range.401`（数值溢出，如 number 转 int32 时超 INT32_MAX）
  - `nlohmann::json::invalid_iterator.207`
- **当前防护**：`is_discarded()` 拦截语法错误，但**不能拦截 get 的类型转换错误**
- **调用上下文**：ffrt task（开机 `LoadInsightIntentInfos` 或 IPC `UpdateInsightIntentEvent`）
- **防护方案**：try-catch 包装（见统一方案 1）

### R2：配置类 Intent 的 `get<InsightIntentProfileInfoVec>()` ⭐⭐⭐⭐⭐

- **位置**：`insight_intent_profile.cpp:422`
- **触发代码**：`profileInfos = jsonObject.get<InsightIntentProfileInfoVec>();`
- **细节**：同 R1，走配置类 Intent 的反序列化路径
- **额外问题**：line 416 有 `TAG_LOGD(..., "jsonObject : %{public}s", jsonObject.dump().c_str())`，**debug 日志里的 dump 在大对象场景也会抛**

### R3：单条 Intent 读取触发 R1 ⭐⭐⭐⭐⭐

- **位置**：`insight_intent_rdb_storage_mgr.cpp:131`
- **触发代码**：`ExtractInsightIntentProfile::TransformTo(value.begin()->second, profileInfos)`
- **细节**：内部触发 R1；`LoadInsightIntentInfo` 单条查询的入口

### R4：单条配置 Intent 读取触发 R2 ⭐⭐⭐⭐⭐

- **位置**：`insight_intent_rdb_storage_mgr.cpp:159`
- **触发代码**：`InsightIntentProfile::TransformTo(value.begin()->second, configIntentInfos)`
- **细节**：内部触发 R2

### R5：批量循环里触发 R1 ⭐⭐⭐⭐⭐

- **位置**：`insight_intent_rdb_storage_mgr.cpp:190`（`Transform()` 第一次循环）
- **触发代码**：
  ```cpp
  for (const auto &item : value) {
      ExtractInsightIntentProfileInfoVec profileInfos;
      if (!ExtractInsightIntentProfile::TransformTo(item.second, profileInfos)) {
          ...
      }
  }
  ```
- **关键风险**：**只要 DB 里任意一条记录的 JSON 损坏，整个加载流程崩溃**
- **触发场景**：开机时 `InitInsightIntentCache` → `LoadInsightIntentInfos` → `Transform`
- **防护方案**：在 R1 修复后自动覆盖；额外可在循环里加 try-catch 让单条损坏不影响其他

### R6：批量循环里触发 R2 ⭐⭐⭐⭐⭐

- **位置**：`insight_intent_rdb_storage_mgr.cpp:205`（`Transform()` 第二次循环）
- **细节**：同 R5，走配置类 Intent 路径

### R7：`TransformConfigIntent` 循环触发 R2 ⭐⭐⭐⭐⭐

- **位置**：`insight_intent_rdb_storage_mgr.cpp:175`
- **触发代码**：
  ```cpp
  for (const auto &item : value) {
      std::vector<InsightIntentInfo> configIntentInfos;
      if (!InsightIntentProfile::TransformTo(item.second, configIntentInfos)) { ... }
  }
  ```
- **细节**：同 R5

### R8：日志里的 `dump()` ⭐⭐⭐

- **位置**：`insight_intent_profile.cpp:416`
- **触发代码**：`TAG_LOGD(AAFwkTag::INTENT, "jsonObject : %{public}s", jsonObject.dump().c_str())`
- **触发场景**：debug 日志在每次解析时都触发，大对象场景必现
- **可能异常**：`out_of_range.407` / `bad_alloc`
- **讽刺点**：日志本用于排查问题，反而成为新的崩溃源
- **防护方案**：改用 SafeDump 并限制长度（见统一方案 2）

### R9：循环里的 `param.dump()` ⭐⭐⭐

- **位置**：`insight_intent_profile.cpp:192, 220`
- **触发代码**：`insightIntentInfo.inputParams.emplace_back(param.dump());`
- **细节**：在 `ProcessIntputParams` / `ProcessOutputParams` 的循环里

### R10：单点 `dump()` ⭐⭐⭐

- **位置**：`insight_intent_profile.cpp:332`
- **触发代码**：`insightIntentInfo.cfgEntities = jsonObject[INSIGHT_INTENT_ENTITES].dump();`
- **已有防护**：`is_object()` 检查（line 331）✅ 防止类型错误，但**不能防止 dump 自身抛异常**

### R11：entity parameters dump ⭐⭐⭐

- **位置**：`extract_insight_intent_profile.cpp:181, 380, 389`
- **触发代码**：`entityInfo.parameters = jsonObject[KEY].dump();`
- **已有防护**：`find != end` + `is_object()` 检查 ✅，但 dump 自身无保护

### R12：`QueryByStep` 与 ResultSet 遍历 ⭐⭐

- **位置**：`insight_intent_rdb_data_mgr.cpp:218, 251, 288`（QueryByStep）；`224-230, 258-273, 295-310`（遍历）
- **当前防护**：
  - `absSharedResultSet == nullptr` 检查 ✅
  - `GoToFirstRow` / `GetString` 检查返回码 ✅
  - `ScopeGuard` 保证 Close ✅
- **防护方案**：保持现状，已被多重检查覆盖

### R13：`SplitStringBySlash` 内部赋值 ⭐

- **位置**：`insight_intent_rdb_storage_mgr.cpp:60`
- **可能异常**：`bad_alloc`（理论）
- **防护方案**：无需额外防护

### R14：`bundleVersionMap[res[BUNDLE_NAME]] = res[VERSION]` ✅

- **位置**：`insight_intent_rdb_storage_mgr.cpp:64`
- **当前防护**：line 61 `if (res.size() < INTENT_KEY_LENGTH) continue;` ✅
- **防护方案**：已安全，无需修改

---

## 类别 I：IPC 反序列化风险点

**调用链**：跨进程 IPC → Parcel → `ReadFromParcel` / `Unmarshalling` → `get<T>`

### I1：单条 IPC 反序列化 ⭐⭐⭐⭐⭐

- **位置**：`insight_intent_info_for_query.cpp:701`
- **触发代码**：`*this = jsonObject.get<InsightIntentInfoForQuery>();`
- **触发场景**：IPC 入参被恶意构造，或上游序列化数据损坏
- **可能异常**：`type_error` / `out_of_range`
- **当前防护**：`is_discarded()` 拦截 parse 阶段错误，但**不拦截 get**
- **调用上下文**：IPC worker 线程，顶层可能有 catch(...)，但功能会失效
- **防护方案**：try-catch 包装（见统一方案 1）

### I2：批量 IPC 反序列化 ⭐⭐⭐⭐⭐

- **位置**：`insight_intent_info_for_query.cpp:810`
- **触发代码**：
  ```cpp
  for (const auto &item : jsonArray) {
      InsightIntentInfoForQuery info = item.get<InsightIntentInfoForQuery>();
  }
  ```
- **额外风险**：循环中途抛异常，已构造的 `infos` 处于半构造状态，即使上层 catch 也会导致**状态污染**
- **防护方案**：try-catch 包装每个元素；或失败时清空 infos

### I3：`modeArray` 未检查 `is_array()` ⭐⭐⭐⭐

- **位置**：`insight_intent_info_for_query.cpp:120-135`（EntryInfoForQuery）和 `250-265`（UIAbilityIntentInfoForQuery）
- **触发代码**：
  ```cpp
  if (jsonObject.find(INSIGHT_INTENT_EXECUTE_MODE) != jsonObjectEnd) {
      const auto &modeArray = jsonObject[INSIGHT_INTENT_EXECUTE_MODE];
      for (const auto &modeStr : modeArray) {  // ← 没检查 is_array
  ```
- **触发场景**：`executeMode` 字段配置成 string / number / object（非数组）
- **可能异常**：`nlohmann::json::type_error.305`（"cannot use iterator with non-array/object"）
- **当前防护**：`find != end` 只验证存在性，**未验证类型**
- **讽刺点**：同文件 line 357/385 的 `inputParams/outputParams` 都有 `is_array()` 检查，说明这是**遗漏而非设计**
- **防护方案**：补 is_array 判断（见统一方案 3）

### I4：`modeStr.get<std::string>()` ✅

- **位置**：`insight_intent_info_for_query.cpp:127, 257`
- **当前防护**：line 123/253 `if (!modeStr.is_string()) continue;` ✅

### I5：`jsonObject.at(KEY)` for input/output params ✅

- **位置**：`insight_intent_info_for_query.cpp:356, 384`
- **当前防护**：line 352/380 `find == end` 前置检查 ✅

### I6：`jsonObject.at(KEY).is_object()` for cfgEntities ✅

- **位置**：`insight_intent_info_for_query.cpp:555`
- **当前防护**：line 554 `find != end` 前置检查 ✅

### I7：`parse(paramStr, nullptr, false)` ✅

- **位置**：`insight_intent_info_for_query.cpp:610, 621`
- **当前防护**：`is_discarded()` 检查 ✅

### I8：循环里的 `param.dump()` ⭐⭐

- **位置**：`insight_intent_info_for_query.cpp:371, 399`
- **细节**：同 R9

### I9：`isConfigIt->get<bool>()` ✅

- **位置**：`insight_intent_info_for_query.cpp:410`
- **当前防护**：line 409 `isConfigIt->is_boolean()` ✅

---

## 类别 D：RDB 框架风险点 ⭐⭐

RDB API 通常**返回错误码不抛异常**，但极端场景理论存在。

| # | 位置 | 触发场景 | 当前防护 |
|---|------|---------|---------|
| D1 | `insight_intent_rdb_data_mgr.cpp:56-60` | `RdbHelper::GetRdbStore` 初始化失败 | 返回 nullptr 已检查 |
| D2 | `insight_intent_rdb_data_mgr.cpp:70, 135, 164, 193, 340` | `rdbStore_->Restore` 物理恢复 | 返回错误码已检查 |
| D3 | `insight_intent_rdb_data_mgr.cpp:324` | `rdbStore_->Backup` 物理备份 | 返回错误码已检查 |
| D4 | `insight_intent_rdb_data_mgr.cpp:87` | `rdbStore->ExecuteSql` 建表 | 返回错误码已检查 |
| D5 | `insight_intent_rdb_data_mgr.cpp:132, 140, 161, 169, 190, 198` | Update / Delete | 返回错误码 + Restore 重试 |
| D6 | `insight_intent_rdb_data_mgr.cpp:218, 251, 288` | QueryByStep 返回 nullptr | 已检查 nullptr ✅ |
| D7 | `insight_intent_rdb_data_mgr.cpp:224, 258, 295` | GoToFirstRow | 已检查返回码 ✅ |
| D8 | `insight_intent_rdb_data_mgr.cpp:230, 261, 267, 298, 304` | GetString | 已检查返回码 ✅ |
| D9 | `insight_intent_rdb_data_mgr.cpp:352` | `std::this_thread::sleep_for` | 信号中断时理论抛 system_error，概率极低 |

**防护方案**：D1-D8 保持现状，已被既有错误码机制覆盖。D9 理论风险可忽略。

---

## 统一防护方案

### 方案 1：安全 JSON 反序列化包装（覆盖 R1-R7、I1-I2）

**新增文件**：`services/abilitymgr/include/insight_intent/intent_json_safe_get.h`

```cpp
#ifndef OHOS_ABILITY_RUNTIME_INTENT_JSON_SAFE_GET_H
#define OHOS_ABILITY_RUNTIME_INTENT_JSON_SAFE_GET_H

#include <exception>
#include "hilog_tag_wrapper.h"
#include "nlohmann/json.hpp"

namespace OHOS {
namespace AbilityRuntime {

template<typename T>
bool SafeJsonGet(const nlohmann::json &jsonObject, T &out, const char *tag)
{
    try {
        out = jsonObject.get<T>();
        return true;
    } catch (const nlohmann::json::exception &e) {
        TAG_LOGE(AAFwkTag::INTENT, "%{public}s json exception: %{public}s", tag, e.what());
        return false;
    } catch (const std::exception &e) {
        TAG_LOGE(AAFwkTag::INTENT, "%{public}s std exception: %{public}s", tag, e.what());
        return false;
    } catch (...) {
        TAG_LOGE(AAFwkTag::INTENT, "%{public}s unknown exception", tag);
        return false;
    }
}

} // namespace AbilityRuntime
} // namespace OHOS
#endif
```

**应用点**（共 4 处）：

1. `extract_insight_intent_profile.cpp:673`
   ```cpp
   // Before
   intentInfos = jsonObject.get<ExtractInsightIntentProfileInfoVec>();
   // After
   if (!SafeJsonGet(jsonObject, intentInfos, "ExtractVec")) {
       return false;
   }
   ```

2. `insight_intent_profile.cpp:422` — 同上模式

3. `insight_intent_info_for_query.cpp:701`
   ```cpp
   // Before
   *this = jsonObject.get<InsightIntentInfoForQuery>();
   // After
   InsightIntentInfoForQuery tmp;
   if (!SafeJsonGet(jsonObject, tmp, "InsightIntentInfoForQuery")) {
       return false;
   }
   *this = std::move(tmp);
   ```

4. `insight_intent_info_for_query.cpp:810`
   ```cpp
   // Before
   InsightIntentInfoForQuery info = item.get<InsightIntentInfoForQuery>();
   // After
   InsightIntentInfoForQuery info;
   if (!SafeJsonGet(item, info, "VecItem")) {
       continue; // 跳过坏元素，避免整个批量失败
   }
   ```

**级联效果**：修这 4 处后，R3-R7、I1-I2 全部自动安全（因为 TransformTo 内部不再抛）。

### 方案 2：安全 dump 工具（覆盖 R8-R11、I8、W2/W4）

**新增工具函数**（同上文件）：

```cpp
inline std::string SafeDump(const nlohmann::json &jsonObject, size_t maxLen = 0)
{
    try {
        auto s = jsonObject.dump();
        if (maxLen > 0 && s.size() > maxLen) {
            return s.substr(0, maxLen) + "...(truncated)";
        }
        return s;
    } catch (const std::exception &e) {
        TAG_LOGW(AAFwkTag::INTENT, "dump failed: %{public}s", e.what());
        return "<dump_failed>";
    } catch (...) {
        return "<dump_failed>";
    }
}
```

**重点应用**：

- `insight_intent_profile.cpp:416` —— 日志里的 dump 必改：
  ```cpp
  // Before
  TAG_LOGD(AAFwkTag::INTENT, "jsonObject : %{public}s", jsonObject.dump().c_str());
  // After
  TAG_LOGD(AAFwkTag::INTENT, "jsonObject : %{public}s", SafeDump(jsonObject, 200).c_str());
  ```

- 写库路径 `insight_intent_rdb_storage_mgr.cpp:229, 244` 的 dump 不能截断（写入必须完整），但可以包 try-catch 返回失败。

### 方案 3：补 `is_array()` 检查（覆盖 I3）

**修改点**（共 2 处）：

```cpp
// insight_intent_info_for_query.cpp:120-135 (EntryInfoForQuery)
if (jsonObject.find(INSIGHT_INTENT_EXECUTE_MODE) != jsonObjectEnd) {
    const auto &modeArray = jsonObject[INSIGHT_INTENT_EXECUTE_MODE];
    if (!modeArray.is_array()) {                                    // 新增
        TAG_LOGE(AAFwkTag::INTENT, "executeMode is not array");
        g_parseResult = ERR_INVALID_VALUE;
    } else {
        for (const auto &modeStr : modeArray) {
            if (!modeStr.is_string()) {
                TAG_LOGE(AAFwkTag::INTENT, "modestr not string");
                continue;
            }
            std::string modeStrValue = modeStr.get<std::string>();
            auto it = STRING_EXECUTE_MODE_MAP.find(modeStrValue);
            if (it != STRING_EXECUTE_MODE_MAP.end()) {
                entryInfo.executeMode.push_back(it->second);
            } else {
                TAG_LOGW(AAFwkTag::INTENT, "Unknown ExecuteMode: %{public}s",
                    SafeDump(modeStr, 50).c_str());
            }
        }
    }
}
```

`insight_intent_info_for_query.cpp:250-265`（UIAbilityIntentInfoForQuery）做同样修改。

---

## 验证计划

### 单元测试（必做）

新增 `test/unittest/insight_intent_safe_get_test/`：

1. **类型不匹配测试**：构造 `intentVersion: 1`（数字），调用 `TransformTo`，验证返回 false 且**不崩溃**
2. **数组类型错测试**：构造 `executeMode: "UI"`（字符串），调用 from_json，验证返回错误码且**不崩溃**
3. **深嵌套 dump 测试**：构造 1500 层嵌套 JSON，调用 SafeDump，验证返回 `<dump_failed>`
4. **数值溢出测试**：构造超 INT32_MAX 的 number 字段，验证 SafeJsonGet 返回 false

### 集成测试（建议）

1. **DB 损坏场景**：手动往 `insight_intent.db` 写入畸形 JSON，重启 AMS，验证开机不崩
2. **IPC 恶意入参**：构造畸形的 InsightIntentInfoForQuery Parcel 数据，调用 AMS IPC，验证 IPC 失败但 AMS 不崩

### 回归测试

```bash
# 跑现有 intent 相关单测
run -t UT -ts insight_intent_db_cache_test
run -t UT -ts extract_insight_intent_profile_test
run -t UT -ts insight_intent_rdb_data_manager_test

# Fuzz 测试
run -t UT -ts extractinsightintentprofile_fuzzer
run -t UT -ts insightintentdbcachefirst_fuzzer
run -t UT -ts insightintentdbcache_fuzzer
```

### 验证闭环

- [ ] 方案 1：SafeJsonGet 工具实现
- [ ] 方案 1：4 处 get<T>() 替换
- [ ] 方案 2：SafeDump 工具实现
- [ ] 方案 2：R8 日志 dump 必改
- [ ] 方案 2：W2/W4 写库 dump 包 try-catch
- [ ] 方案 3：2 处 is_array 检查
- [ ] 新增单测覆盖类型不匹配、深嵌套、数值溢出场景
- [ ] 跑现有 intent 单测和 fuzz 全绿
- [ ] DB 损坏场景手动验证

---

## 参考资料

- nlohmann::json 异常类型：https://json.nlohmann.me/home/exceptions/
- `is_discarded()` 仅拦截 parse 阶段错误，不拦截 get 阶段错误
- `get<T>()` 没有 `allow_exceptions=false` 参数，只能 try-catch
- FFRT task 顶层无 try-catch，未捕获异常 → `std::terminate` → 进程崩溃
- AMS 属于 foundation 进程（SA 401），崩溃会触发整个系统能力框架重启

## 风险点完整索引

| ID | 文件:行号 | 等级 | 类别 | 触发场景 |
|----|----------|------|------|---------|
| W1 | extract_..:694 | ⭐⭐ | 写库 | ToJson 隐式转换 |
| W2 | rdb_storage_mgr:229 | ⭐⭐⭐ | 写库 | dump 大对象 |
| W3 | rdb_storage_mgr:240 | ⭐⭐ | 写库 | 同 W1 |
| W4 | rdb_storage_mgr:244 | ⭐⭐⭐ | 写库 | 同 W2 |
| W5 | rdb_data_mgr:336 | ⭐⭐ | 写库 | RDB API（已有重试） |
| W6 | rdb_data_mgr:107 | ⭐ | 写库 | bad_alloc 理论 |
| R1 | extract_..:673 | ⭐⭐⭐⭐⭐ | 读库 | get<Vec> 字段类型不匹配 |
| R2 | profile:422 | ⭐⭐⭐⭐⭐ | 读库 | 同 R1（配置类） |
| R3 | rdb_storage_mgr:131 | ⭐⭐⭐⭐⭐ | 读库 | 单条触发 R1 |
| R4 | rdb_storage_mgr:159 | ⭐⭐⭐⭐⭐ | 读库 | 单条触发 R2 |
| R5 | rdb_storage_mgr:190 | ⭐⭐⭐⭐⭐ | 读库 | 批量触发 R1 |
| R6 | rdb_storage_mgr:205 | ⭐⭐⭐⭐⭐ | 读库 | 批量触发 R2 |
| R7 | rdb_storage_mgr:175 | ⭐⭐⭐⭐⭐ | 读库 | TransformConfigIntent 触发 R2 |
| R8 | profile:416 | ⭐⭐⭐ | 读库 | 日志 dump 大对象 |
| R9 | profile:192, 220 | ⭐⭐⭐ | 读库 | 循环 dump |
| R10 | profile:332 | ⭐⭐⭐ | 读库 | 单点 dump |
| R11 | extract_..:181, 380, 389 | ⭐⭐⭐ | 读库 | entity dump |
| R12 | rdb_data_mgr:218+ | ⭐⭐ | 读库 | QueryByStep（已防护） |
| R13 | rdb_storage_mgr:60 | ⭐ | 读库 | bad_alloc 理论 |
| R14 | rdb_storage_mgr:64 | — | 读库 | 已防护 ✅ |
| I1 | for_query:701 | ⭐⭐⭐⭐⭐ | IPC | get<T> 反序列化 |
| I2 | for_query:810 | ⭐⭐⭐⭐⭐ | IPC | 批量 get<T> |
| I3 | for_query:120, 250 | ⭐⭐⭐⭐ | IPC | range-for 非数组 |
| I4 | for_query:127, 257 | — | IPC | 已防护 ✅ |
| I5 | for_query:356, 384 | — | IPC | 已防护 ✅ |
| I6 | for_query:555 | — | IPC | 已防护 ✅ |
| I7 | for_query:610, 621 | — | IPC | 已防护 ✅ |
| I8 | for_query:371, 399 | ⭐⭐ | IPC | 循环 dump |
| I9 | for_query:410 | — | IPC | 已防护 ✅ |
| D1-D9 | rdb_data_mgr 多处 | ⭐⭐ | RDB | 框架级，多已防护 |

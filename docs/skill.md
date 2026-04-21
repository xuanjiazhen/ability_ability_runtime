# Skill 执行框架

## 概述

Skill 执行框架提供了一种从系统侧（如 HDC 命令）启动目标应用并执行指定 Skill 函数的能力。框架负责跨进程调度、模块加载、函数调用和结果回传，支持同步和异步（Promise）两种执行模式。调用方可通过 `funcName` 参数指定要调用的函数名，不指定时默认调用 `ExecuteSkill`。

## 调用流程

### AMS 侧调度

```
调用方 (arktsScript / HDC)
  │
  │  IPC: StartSkillByHDC(bundleName, moduleName, skillName, params, srcEntrance, funcName)
  ▼
AbilityManagerService::StartSkillByHDC
  │
  │  构建 Want: SkillExecuteManager::GenerateSkillWant
  │    → SkillExecuteParam::WriteToWant 将执行参数嵌入 Want
  │
  │  启动目标 Ability: StartAbilityByCallWithSkillIntent(want)
  │
  │  ├─ Ability 已启动 → OnExecuteIntent(want)
  │  │    → 通过 AbilityScheduler IPC 直接发送 Want 到目标进程
  │  │
  │  └─ Ability 未启动 → StartAbilityByCall(want, connect)
  │       → 冷启动目标进程，Want 随 Ability 启动流程传递
  ▼
目标进程 UIAbilityImpl
  到达 ABILITY_STATE_BACKGROUND_NEW 分支
  │
  │  HandleExecuteSkill(want)
  │    ① SkillExecuteParam::GenerateFromWant 从 Want 提取参数
  │    ② 根据 lifecycleState_ 判断冷启动/已启动:
  │       ├─ INITIAL / STARTED → ExecuteSkillBackgroundByColdBoot (返回 false，延迟状态转换)
  │       └─ 其他            → ExecuteSkillBackgroundAlreadyStart (返回 true)
  │    ③ ability_->ExecuteSkillBackground(want, param, callback)
  ▼
JsUIAbility / EtsUIAbility::ExecuteSkillBackground
  │
  │  构建 SkillExecutorInfo { srcEntry, funcName, executeParam }
  │  SkillExecutorMgr::ExecuteSkill(runtime, skillInfo, callback)
  ▼
SkillExecutorMgr::ExecuteSkill
  │
  │  ① SkillExecutor::Create → JsSkillFunc
  │  ② executor->Init → 保存 funcName（默认 "ExecuteSkill"）+ LoadModule 加载 abc 模块
  │  ③ executor->ExecuteSkill(param, callback, isAsync)
  ▼
JsSkillFunc::CallExecuteSkill
  │
  │  ① 从导出对象获取 funcName 指定的函数
  │  ② napi_call_function 调用三方应用的导出函数
  ▼
三方应用导出函数（由 funcName 决定，默认 ExecuteSkill）
  │
  │  返回值处理:
  ├─ 同步返回 (string/object) → ReplySucceeded → callback
  └─ Promise → .then(resolve) / .catch(reject) → callback
  ▼
回调处理（根据启动模式分支）
  │
  ├─ 冷启动回调: Background() → AbilityTransactionCallback(BACKGROUND_NEW)
  │    手动完成状态转换，通知 AMS 进入后台
  │
  └─ 已启动回调: 无额外操作
       外层 AbilityTransactionCallback 自动完成状态转换
```

### 冷启动 vs 已启动对比

| 维度 | 冷启动 (Cold Boot) | 已启动 (Already Start) |
|------|-------------------|----------------------|
| 触发条件 | `lifecycleState_` 为 INITIAL 或 STARTED | `lifecycleState_` 为 FOREGROUND 等 |
| AMS 侧 | `StartAbilityByCall` 冷启动进程 | `OnExecuteIntent` 直接 IPC |
| HandleExecuteSkill 返回 | `false`（延迟状态转换） | `true`（状态转换已完成） |
| 异步回调 | `Background()` + `AbilityTransactionCallback(BACKGROUND_NEW)` | 无额外操作 |

## IPC 接口

### StartSkillByHDC

启动目标应用的 Skill 执行。

```
int32_t StartSkillByHDC(
    const std::string &bundleName,   // 目标应用包名
    const std::string &moduleName,   // 目标模块名
    const std::string &skillName,    // Skill 名称
    const std::shared_ptr<WantParams> &params,  // 传递给 Skill 的参数
    const std::string &srcEntrance,  // 模块入口路径（ohm url 格式）
    const std::string &funcName      // 要调用的函数名（默认 "ExecuteSkill"）
);
```

## SkillExecuteParam 数据结构

通过 Want 参数在进程间传递的 Skill 执行参数。

| 字段 | 类型 | 说明 |
|------|------|------|
| bundleName_ | string | 目标应用包名 |
| moduleName_ | string | 目标模块名 |
| skillName_ | string | Skill 名称 |
| params_ | shared_ptr\<WantParams\> | Skill 执行参数 |
| srcEntrance_ | string | 模块入口路径（ohm url 格式） |
| funcName_ | string | 要调用的函数名（空字符串时默认 "ExecuteSkill"） |

### Want 参数 Key

| Key | 说明 |
|-----|------|
| `ohos.skill.executeParam.bundleName` | 应用包名 |
| `ohos.skill.executeParam.moduleName` | 模块名 |
| `ohos.skill.executeParam.skillName` | Skill 名称 |
| `ohos.skill.executeParam.params` | 执行参数 |
| `ohos.skill.executeParam.srcEntrance` | 模块入口路径 |
| `ohos.skill.executeParam.funcName` | 目标函数名 |

## 三方应用接口规格

三方应用需要在 `srcEntrance` 指定的模块中导出函数。函数名由调用方通过 `funcName` 指定，不指定时默认为 `ExecuteSkill`。

### 函数签名

```typescript
// 默认函数名: ExecuteSkill（可通过 funcName 参数自定义）
export function <funcName>(params?: Record<string, Object>): string | object | Promise<string | object>;
```

### 参数说明

| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| params | Record\<string, Object\> | 否 | 调用方通过 IPC 传入的参数 |

### 返回值

| 返回类型 | 说明 |
|----------|------|
| string | 同步返回字符串结果 |
| object | 同步返回对象结果（自动 JSON 序列化） |
| Promise\<string\> | 异步返回字符串结果 |
| Promise\<object\> | 异步返回对象结果 |

### 示例

**默认函数名 ExecuteSkill**

```typescript
// src/main/ets/skill/WeatherSkill.ets
export function ExecuteSkill(params?: Record<string, Object>): string {
  const city = params?.["city"] as string;
  return `Weather in ${city}: sunny, 25°C`;
}
```

**自定义函数名（调用方传 funcName = "handleWeather"）**

```typescript
// src/main/ets/skill/WeatherSkill.ets
export function handleWeather(params?: Record<string, Object>): Promise<string> {
  const city = params?.["city"] as string;
  return new Promise((resolve, reject) => {
    fetchWeather(city)
      .then(weather => resolve(weather))
      .catch(err => reject(err));
  });
}
```

**同一个模块导出多个 Skill 函数**

```typescript
// src/main/ets/skill/MySkill.ets
export function ExecuteSkill(params?: Record<string, Object>): string {
  return "default skill";
}

export function handleTranslate(params?: Record<string, Object>): Promise<string> {
  // funcName = "handleTranslate" 时调用此函数
  return translate(params?.["text"] as string);
}

export function handleSummarize(params?: Record<string, Object>): string {
  // funcName = "handleSummarize" 时调用此函数
  return summarize(params?.["content"] as string);
}
```

### 错误处理

- 如果模块不存在或加载失败，框架返回 `ERR_INVALID_VALUE` 错误码
- 如果导出对象上不存在 `funcName` 指定的函数，框架返回 `ERR_INVALID_VALUE` 错误码
- 如果目标函数返回的 Promise 被 reject，框架将结果标记为执行失败

## 关键源文件

| 文件 | 说明 |
|------|------|
| `interfaces/inner_api/ability_manager/include/skill/skill_execute_param.h` | Skill 执行参数定义 |
| `interfaces/inner_api/ability_manager/include/skill/skill_execute_result.h` | Skill 执行结果定义 |
| `interfaces/kits/native/ability/native/skill_executor/skill_executor.h` | 执行器基类 |
| `interfaces/kits/native/ability/native/skill_executor/skill_executor_info.h` | 执行器信息结构 |
| `interfaces/kits/native/ability/native/skill_executor/skill_executor_mgr.h` | 执行器管理器 |
| `frameworks/native/ability/native/skill_executor/js_skill_func.h/.cpp` | JS 执行器实现 |
| `frameworks/native/ability/native/skill_executor/skill_executor_mgr.cpp` | 管理器实现 |
| `services/abilitymgr/include/skill/skill_execute_manager.h` | AMS 侧 Skill Want 生成 |
| `frameworks/native/ability/native/ui_ability_impl.cpp` | Skill 执行入口（HandleExecuteSkill） |

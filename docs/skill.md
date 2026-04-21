# Skill 执行框架

## 概述

Skill 执行框架提供了一种从应用侧（JS/ArkTS）启动目标应用 Ability 的能力。框架负责跨进程调度，将 Skill 参数通过 Want 传递给目标 Ability。目标 Ability 通过正常的生命周期回调（`OnStart`/`OnNewWant`）接收携带 Skill 参数的 Want。

## JS 应用接口

### execArkTSTool

```typescript
interface arkToolParams {
    bundleName: string;
    moduleName: string;
    skillName: string;
    arkTSPath: string;
    funcName: string;
    argv: string[];
}

interface arkToolResult {
    code: number;
    result?: Record<string, RecordData>;
}

abilityManager.execArkTSTool(params: arkToolParams): Promise<arkToolResult>
```

| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| params | arkToolParams | 是 | Skill 执行参数对象 |
| params.bundleName | string | 是 | 目标应用包名 |
| params.moduleName | string | 是 | 目标模块名 |
| params.skillName | string | 是 | Skill 名称 |
| params.arkTSPath | string | 否 | 目标 ArkTS 文件路径 |
| params.funcName | string | 否 | 目标函数名 |
| params.argv | string[] | 否 | 传递给 Skill 的参数列表 |

### 示例

```javascript
import abilityManager from '@ohos.app.ability.abilityManager';

abilityManager.execArkTSTool({
    bundleName: 'com.example.app',
    moduleName: 'entry',
    skillName: 'weather',
    arkTSPath: 'WeatherSkill',
    funcName: 'handleWeather',
    argv: ['Shenzhen']
}).then((result) => {
    console.log('skill started, code:', result.code);
}).catch((err) => {
    console.error('failed:', err);
});
```

## 调用流程

```
JS 应用侧
  abilityManager.execArkTSTool(params)
  │
  │  Promise<arkToolResult>
  ▼
NAPI 层 (js_ability_manager.cpp)
  JsAbilityManager::OnExecArkTSTool
  │  对象参数解析 + argv 字符串数组解析 + NapiAsyncTask 异步调用
  ▼
AbilityManagerClient::StartInAppSkill(bundleName, moduleName, skillName, arkTSPath, funcName, argv)
  │
  │  IPC
  ▼
AbilityManagerService::StartInAppSkill
  │  ① SkillExecuteManager::GenerateSkillWant → 参数嵌入 Want
  │  ② StartAbilityByCallWithSkillIntent(want)
  │
  │  ├─ Ability 已启动 → OnExecuteIntent(want)
  │  └─ Ability 未启动 → StartAbilityByCall(want) 冷启动
  ▼
目标进程 UIAbilityImpl
  HandleExecuteSkill(want) → Background() → return true
  ▼
目标 Ability
  通过 OnStart(want) / OnNewWant(want) 接收携带 Skill 参数的 Want
```

## IPC 接口

### StartInAppSkill

```
int32_t StartInAppSkill(
    const std::string &bundleName,
    const std::string &moduleName,
    const std::string &skillName,
    const std::string &arkTSPath = "",
    const std::string &funcName = "",
    const std::vector<std::string> &argv = {}
);
```

## SkillExecuteParam 数据结构

| 字段 | 类型 | 说明 |
|------|------|------|
| bundleName_ | string | 目标应用包名 |
| moduleName_ | string | 目标模块名 |
| skillName_ | string | Skill 名称 |
| arkTSPath_ | string | 目标 ArkTS 文件路径 |
| funcName_ | string | 目标函数名 |
| argv_ | vector\<string\> | 参数列表 |

### Want 参数 Key

| Key | 说明 |
|-----|------|
| `ohos.skill.executeParam.bundleName` | 应用包名 |
| `ohos.skill.executeParam.moduleName` | 模块名 |
| `ohos.skill.executeParam.skillName` | Skill 名称 |
| `ohos.skill.executeParam.arkTSPath` | ArkTS 文件路径 |
| `ohos.skill.executeParam.funcName` | 目标函数名 |
| `ohos.skill.executeParam.argvCount` | 参数个数 |
| `ohos.skill.executeParam.argv.N` | 第 N 个参数 (N 从 0 开始) |

## 关键源文件

| 文件 | 说明 |
|------|------|
| `frameworks/js/napi/ability_manager/js_ability_manager.cpp` | JS → C++ NAPI 桥接 |
| `interfaces/inner_api/ability_manager/include/skill/skill_execute_param.h` | Skill 执行参数定义 |
| `services/abilitymgr/src/skill/skill_execute_param.cpp` | 参数序列化实现 |
| `services/abilitymgr/include/skill/skill_execute_manager.h` | AMS 侧 Skill Want 生成 |
| `services/abilitymgr/src/skill/skill_execute_manager.cpp` | AMS 侧 Want 生成实现 |
| `frameworks/native/ability/native/ui_ability_impl.cpp` | Skill 执行入口（HandleExecuteSkill） |

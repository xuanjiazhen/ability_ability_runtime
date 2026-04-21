# Skill 执行框架

## 概述

Skill 执行框架提供了一种从系统侧（如 HDC 命令）启动目标应用 Ability 的能力。框架负责跨进程调度，将 Skill 参数通过 Want 传递给目标 Ability。目标 Ability 通过正常的生命周期回调（`OnStart`/`OnNewWant`）接收携带 Skill 参数的 Want。

## 调用流程

```
调用方 (arktsScript / HDC)
  │
  │  IPC: StartSkillByHDC(bundleName, moduleName, skillName, params)
  ▼
AbilityManagerService::StartSkillByHDC
  │
  │  构建 Want: SkillExecuteManager::GenerateSkillWant
  │    → SkillExecuteParam::WriteToWant 将参数嵌入 Want
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
  │    ② Background() 进入后台
  │    ③ return true → AbilityTransactionCallback 通知 AMS
  ▼
目标 Ability
  通过 OnStart(want) / OnNewWant(want) 接收携带 Skill 参数的 Want
  调用方收到 StartSkillByHDC 返回值（成功/失败）
```

## IPC 接口

### StartSkillByHDC

启动目标应用的 Skill 执行。

```
int32_t StartSkillByHDC(
    const std::string &bundleName,   // 目标应用包名
    const std::string &moduleName,   // 目标模块名
    const std::string &skillName,    // Skill 名称
    const std::shared_ptr<WantParams> &params   // 传递给 Skill 的参数
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

### Want 参数 Key

| Key | 说明 |
|-----|------|
| `ohos.skill.executeParam.bundleName` | 应用包名 |
| `ohos.skill.executeParam.moduleName` | 模块名 |
| `ohos.skill.executeParam.skillName` | Skill 名称 |
| `ohos.skill.executeParam.params` | 执行参数 |

## 关键源文件

| 文件 | 说明 |
|------|------|
| `interfaces/inner_api/ability_manager/include/skill/skill_execute_param.h` | Skill 执行参数定义 |
| `services/abilitymgr/include/skill/skill_execute_manager.h` | AMS 侧 Skill Want 生成 |
| `services/abilitymgr/src/skill/skill_execute_manager.cpp` | AMS 侧 Want 生成实现 |
| `services/abilitymgr/src/skill/skill_execute_param.cpp` | 参数序列化实现 |
| `frameworks/native/ability/native/ui_ability_impl.cpp` | Skill 执行入口（HandleExecuteSkill） |

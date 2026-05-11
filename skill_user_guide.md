# Skill 执行使用文档

## 1. 概述

Skill 框架支持跨进程调用目标 App 中注册的 ArkTS 函数。调用方通过 CliToolManagerService 发起请求，AMS 调度到目标 App（ServiceExtension 或 UIAbility），目标 App 执行函数后通过回调返回结果。

```
调用方 ──IPC──▶ CliToolManagerService ──IPC──▶ AMS ──IPC──▶ 目标 App
                                                                  │
调用方 ◀──event── CliToolManagerService ◀──IPC── AMS ◀──IPC──────┘
```

## 2. 调用方：发起 Skill 执行

### 2.1 调用方式

```typescript
import cliManager from '@ohos.app.ability.cliManager';

const session = await cliManager.execTool(
    "ohos-arkTSScript",   // toolName，固定值
    "",                    // subcommand，留空
    {                      // args，key-value 参数
        skillName: "mySkill",
        bundleName: "com.example.targetapp",  // 可选
        moduleName: "entry",                   // 可选
        scriptPath: "ets/skill/MySkill",       // 可选
        functionName: "handleSkill",           // 可选
        // 以下为自定义参数，会透传给目标函数
        param1: "hello",
        param2: 42,
        param3: true
    },
    "",                    // challenge，留空
    { background: false }  // options
);
```

### 2.2 参数说明

#### 系统保留参数

调用方在 args 中传递以下保留字段，用于定位目标 Skill：

| 字段 | 类型 | 必填 | 说明 |
|------|------|------|------|
| `skillName` | string | **是** | 目标 Skill 名称，对应目标 App 的 skill 配置 |
| `bundleName` | string | 否 | 目标 App 包名。不传时 AMS 通过 skillName 查询 |
| `moduleName` | string | 否 | 目标模块名。不传时 AMS 使用默认模块 |
| `scriptPath` | string | 否 | 脚本路径，指定加载哪个 ArkTS 文件 |
| `functionName` | string | 否 | 目标函数名。不传时使用 skill 配置中的默认函数 |

> 参数 key 支持 `--skillName` 和 `-skillName` 前缀，框架会自动去除前缀。

#### 自定义参数（skillArgs）

除保留字段外的所有参数会作为 `skillArgs` 透传给目标 App。

**支持的值类型**：

| 类型 | 示例 | 说明 |
|------|------|------|
| string | `"hello"` | 字符串 |
| boolean | `true` / `false` | 布尔值 |
| integer | `42` | 整数 (int32) |
| number | `3.14` | 浮点数 (double) |
| array | `[1, 2, 3]` | 数组，元素仅支持上述基本类型 |

**不支持**：嵌套 object（如 `{ nested: { key: "val" } }`）

#### 传递 JSON 字符串

如果参数较多，可以通过 `args` 字段传递 JSON 字符串：

```typescript
{
    skillName: "mySkill",
    args: '{"param1":"hello","param2":42}'
}
```

框架会自动解析 JSON 字符串并展开为独立参数。

### 2.3 返回值

```typescript
interface CliSessionInfo {
    sessionId: string;       // 会话 ID
    toolName: string;        // 工具名，固定 "ohos-arkTSScript"
    status: string;          // "completed" | "failed"
    result?: {
        exitCode: number;    // 目标 App 返回的 code，0 表示成功
        outputText: string;  // 目标 App 返回的 result，WantParams 字符串格式
    }
}
```

## 3. 目标 App：接收并处理 Skill 请求

### 3.1 Skill 注册

在目标 App 的 `module.json5` 中注册 Skill 配置，指定 `srcEntries`（源文件列表）和 `abilityName`（承载的 Extension）。

### 3.2 目标函数签名

框架通过 `srcEntries` 加载 ArkTS 模块，根据 `functionName` 查找并调用目标函数。

目标函数接收的参数：

```typescript
function handleSkill(
    info: {
        requestCode: string;    // 请求唯一标识，回调时需要传回
        context: Context;       // ServiceExtensionContext 或 AbilityContext
    },
    ...args: any[]              // 调用方透传的 skillArgs，按顺序展开
): void
```

**示例**：

```typescript
// 调用方传递: { skillName: "mySkill", param1: "hello", param2: 42 }
// 目标函数接收:
export function handleSkill(info: { requestCode: string; context: Context },
    param1: string, param2: number): void {
    console.log(`requestCode: ${info.requestCode}`);
    console.log(`param1: ${param1}, param2: ${param2}`);
}
```

### 3.3 返回结果

目标函数执行完成后，**必须主动调用** `completeArkTSScriptInApp` 返回结果：

```typescript
import scriptManager from '@ohos.app.ability.scriptManager';

scriptManager.completeArkTSScriptInApp(
    context,           // Context 对象
    requestCode,       // 从 info.requestCode 获取
    {
        code: 0,       // 执行结果码，0 表示成功
        result: {      // 可选，结构化返回数据
            answer: "done",
            count: 10
        },
        uris: [],      // 可选，返回的文件 URI 列表
        flags: 0       // 可选，URI 权限标志位
    }
);
```

#### 结果字段说明

| 字段 | 类型 | 必填 | 说明 |
|------|------|------|------|
| `code` | number | **是** | 执行结果码。0 = 成功，其他值 = 失败 |
| `result` | object | 否 | 键值对形式的返回数据，仅支持基本类型值 |
| `uris` | string[] | 否 | 文件 URI 列表，AMS 会自动授权给调用方 |
| `flags` | number | 否 | URI 权限标志位 |

> 如果不调用 `completeArkTSScriptInApp`，调用方的 Promise 将一直 pending，直到超时。

## 4. 完整示例

### 调用方

```typescript
import cliManager from '@ohos.app.ability.cliManager';

async function callSkill() {
    try {
        const session = await cliManager.execTool("ohos-arkTSScript", "", {
            skillName: "translateText",
            bundleName: "com.example.translator",
            functionName: "translate",
            sourceText: "Hello World",
            targetLang: "zh-CN"
        }, "", { background: false });

        if (session.status === "completed" && session.result?.exitCode === 0) {
            console.log("翻译结果:", session.result.outputText);
        } else {
            console.error("执行失败, exitCode:", session.result?.exitCode);
        }
    } catch (e) {
        console.error("Skill 调用异常:", e);
    }
}
```

### 目标 App

```typescript
// src/main/ets/skill/TranslateSkill.ets

import scriptManager from '@ohos.app.ability.scriptManager';

export function translate(
    info: { requestCode: string; context: Context },
    sourceText: string,
    targetLang: string
): void {
    // 执行翻译逻辑
    const result = doTranslate(sourceText, targetLang);

    // 返回结果
    scriptManager.completeArkTSScriptInApp(info.context, info.requestCode, {
        code: 0,
        result: {
            translatedText: result.text,
            confidence: result.score
        }
    });
}
```

## 5. 错误码

| 错误场景 | 调用方感知 |
|----------|------------|
| `skillName` 为空 | Promise reject |
| Skill 类型为 independent（不支持） | Promise reject |
| 目标 App 启动失败 | Promise reject |
| 目标函数未找到 | `session.result.exitCode = -1` |
| 目标函数执行异常 | `session.result.exitCode = -1` |
| 目标 App 未调用 completeArkTSScriptInApp | Promise 一直 pending，直到超时 |

## 6. 注意事项

1. **异步回调**：目标函数是异步执行的，必须调用 `completeArkTSScriptInApp` 才能返回结果
2. **参数类型**：skillArgs 仅支持 string / boolean / integer / number / array 基本类型，不支持嵌套 object
3. **requestCode**：目标函数必须将 `info.requestCode` 原样传回，AMS 依此关联请求和回调
4. **超时**：调用方可设置超时时间，超时后 Promise resolve 为 failed 状态
5. **bundleName 可选**：不传 bundleName 时，AMS 通过 skillName 从包管理查询目标 App

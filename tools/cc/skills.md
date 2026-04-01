# ohos-claw-cc 命令行工具功能说明

## 概述

`ohos-claw-cc` 是 OpenHarmony InsightIntent 框架的命令行调试工具，用于执行意图和查询意图信息。工具通过 `AbilityManagerClient` 与 `AbilityManagerService` 通信，支持 Shell 调用者免权限使用。

- **可执行文件**: `/system/bin/ohos-claw-cc`
- **源码路径**: `foundation/ability/ability_runtime/tools/cc/`
- **日志标签**: `AAFwkTag::CC_TOOL`

## 命令列表

```
ohos-claw-cc <command> <options>

Commands:
  help                显示帮助信息
  execute-intent      执行洞察意图并返回结果
  get-intent          查询洞察意图信息
```

---

## 1. help

显示所有可用命令列表。

### 入参格式

```
ohos-claw-cc help
```

无额外参数。

### 输出格式

```
usage: ohos-claw-cc <command> <options>
These are common ohos-claw-cc commands list:
  help                        list available commands
  execute-intent              execute insight intent with result
  get-intent                  get all insight intent info
```

### 错误输出

当传入不存在的命令时：

```
ohos-claw-cc: '<command>' is not a valid ohos-claw-cc command. See 'ohos-claw-cc help'.
<HELP_MSG>
```

---

## 2. execute-intent

执行一个洞察意图，同步等待执行结果返回（超时 30 秒）。

### 入参格式

```
ohos-claw-cc execute-intent -b <bundle> -m <module> -i <intent>
    [-a <ability>] [-e <mode>] [-p <json>] [-h]
```

### 参数规格

| 短选项 | 长选项 | 类型 | 必填 | 校验规则 | 说明 |
|--------|--------|------|------|----------|------|
| `-b` | `--bundle` | string | **是** | 非空 | 目标应用包名 |
| `-m` | `--module` | string | **是** | 非空 | 模块名 |
| `-i` | `--intent` | string | **是** | 非空 | 洞察意图名称 |
| `-a` | `--ability` | string | 否 | 非空 | Ability 名称，省略时自动解析 |
| `-e` | `--execute-mode` | integer | 否 | 0-3，`strtol` 解析 | 执行模式，省略时自动解析 |
| `-p` | `--param` | string | 否 | 合法 JSON | 意图执行参数，JSON 对象字符串 |
| `-h` | `--help` | - | - | - | 显示帮助信息后退出 |

### ExecuteMode 枚举值

| 值 | 名称 | 说明 |
|----|------|------|
| 0 | `UI_ABILITY_FOREGROUND` | 前台拉起 UIAbility 执行 |
| 1 | `UI_ABILITY_BACKGROUND` | 后台执行 |
| 2 | `UI_EXTENSION_ABILITY` | UIExtension 执行 |
| 3 | `SERVICE_EXTENSION_ABILITY` | ServiceExtension 执行 |

### 参数自动解析

当 `-a` 或 `-e` 未提供时，工具自动调用 `GetInsightIntentInfoByIntentName` 查询意图数据库。

**abilityName 解析顺序**（跳过已提供的参数）:

| 优先级 | 数据来源 | 字段 |
|--------|----------|------|
| 1 | UIAbility 意图信息 | `uiAbilityIntentInfo.abilityName` |
| 2 | Page 装饰器信息 | `pageInfo.uiAbility` |
| 3 | Entry 装饰器信息 | `entryInfo.abilityName` |
| 4 | Form 装饰器信息 | `formInfo.abilityName` |

全部为空时报错，提示用户显式提供 `-a`。

**executeMode 解析规则**:

| 匹配模式数 | 行为 |
|------------|------|
| 0 | 报错，提示显式提供 `-e` |
| 1 | 自动选择该模式 |
| >1 | 报错并列出所有支持的模式值，提示显式提供 `-e` |

支持模式来源：`entryInfo.executeMode` + `uiAbilityIntentInfo.supportExecuteMode`。

### 参数 JSON 格式 (`-p`)

`-p` 接受标准 JSON 对象字符串，内部由 `CcParamParser::BuildWantParamsFromJson` 转换为 `WantParams`。

```bash
-p '{"key1":"value1","key2":123,"key3":true,"key4":[1,2,3],"key5":{"nested":"obj"}}'
```

**类型映射表**:

| JSON 类型 | WantParams 映射类型 | 说明 |
|-----------|-------------------|------|
| string | `AAFwk::String` | 直接映射 |
| integer (int32 范围内) | `AAFwk::Integer` | -2147483648 ~ 2147483647 |
| integer (超出 int32) | `AAFwk::Long` | int64 |
| float / double | `AAFwk::String` | 转为字符串存储 |
| boolean | `AAFwk::Boolean` | true/false |
| null | `AAFwk::String` | 空字符串 |
| object | `AAFwk::WantParamWrapper` | 递归解析为嵌套 WantParams |
| array | `AAFwk::Array` | 根据首个元素类型决定数组元素类型 |

**数组类型推断**（基于首个元素）:

| 首元素类型 | 数组 InterfaceID |
|-----------|-----------------|
| String | `g_IID_IString` |
| Integer | `g_IID_IInteger` |
| Long | `g_IID_ILong` |
| Boolean | `g_IID_IBoolean` |
| Float | `g_IID_IFloat` |
| Double | `g_IID_IDouble` |
| Object | `g_IID_IWantParams` |

### 输出格式

#### 成功输出

```
execute intent successfully.
Result:
  innerErr: <int>
  code: <int>
  flags: <int>
  result: <WantParams.ToString()>    (仅当 result != null 时输出)
  uris: <count> item(s)              (仅当 uris 非空时输出)
    - <uri_1>
    - <uri_2>
    ...
```

**字段说明**:

| 字段 | 类型 | 始终输出 | 说明 |
|------|------|----------|------|
| `innerErr` | int32 | 是 | 意图执行内部错误码。0 = 成功，其他见 `InsightIntentInnerErr` |
| `code` | int32 | 是 | 业务返回码 |
| `flags` | int32 | 是 | 返回标志位 |
| `result` | WantParams | 否（null 时不输出） | 结构化执行结果，输出其 `ToString()` |
| `uris` | vector\<string\> | 否（空时不输出） | 返回的 URI 列表 |

**innerErr 枚举值**（`InsightIntentInnerErr`）:

| 值 | 名称 | 说明 |
|----|------|------|
| 0 | `INSIGHT_INTENT_ERR_OK` | 执行成功 |
| 1 | `INSIGHT_INTENT_INTERNAL_ERROR` | 内部错误 |
| 2 | `INSIGHT_INTENT_EXECUTE_REPLY_FAILED` | 执行回调失败 |

#### 错误输出

**参数解析阶段**:

| 场景 | 输出 |
|------|------|
| 缺少必填参数 | `error: missing required parameters.\nRequired: -b, -m, -i\n` |
| 无效 execute-mode 值 | `error: invalid execute mode value.\n` |
| 未知选项 | `fail: unknown option.\n` |
| 无参数调用 | `error: you must specify an option at least.\n` |
| 调用 `-h` | 完整帮助文本 |

**自动解析阶段**:

| 场景 | 输出 |
|------|------|
| 意图查询失败 | `error: failed to query intent from database.\n` |
| abilityName 未找到 | `error: abilityName not found in intent database.\nPlease provide -a <ability-name> explicitly.\n` |
| 无可用执行模式 | `error: no execute mode found in intent database.\nPlease provide -e <execute-mode> explicitly.\n` |
| 多个执行模式 | `error: intent supports multiple execute modes.\nPlease provide -e <execute-mode> explicitly.\nSupported modes: 0, 1\n` |

**执行阶段**:

| 场景 | 输出 |
|------|------|
| 执行失败 | `error: failed to execute intent.\n` + 错误码描述 |
| 连接服务失败 | `error: failed to execute your command.\n` |

### 示例

```bash
# 最简调用（自动解析 abilityName 和 executeMode）
ohos-claw-cc execute-intent -b com.example.app -m entry -i MyIntent

# 指定所有参数
ohos-claw-cc execute-intent \
    -b com.example.app \
    -m entry \
    -a MainAbility \
    -i MyIntent \
    -e 0

# 后台执行 + 传递参数
ohos-claw-cc execute-intent \
    -b com.example.app \
    -m entry \
    -i MyIntent \
    -e 1 \
    -p '{"city":"Beijing","days":3}'

# 嵌套参数
ohos-claw-cc execute-intent \
    -b com.example.app \
    -m entry \
    -i MyIntent \
    -p '{"user":{"name":"test","age":20},"tags":["a","b","c"]}'

# 查看帮助
ohos-claw-cc execute-intent -h
```

---

## 3. get-intent

查询洞察意图注册信息，支持三种查询模式和三种查询标志（位运算可组合）。

### 入参格式

```
ohos-claw-cc get-intent -e <query-mode> -f <flag>
    [-u <user-id>] [-b <bundle>] [-m <module>] [-i <intent>] [-h]
```

### 参数规格

| 短选项 | 长选项 | 类型 | 必填 | 校验规则 | 说明 |
|--------|--------|------|------|----------|------|
| `-e` | `--execute-mode` | integer | **是** | 1/2/3，`strtol` 解析 | 查询模式 |
| `-f` | `--flag` | integer | **是** | 1/2/4（位运算可组合），`strtol` 解析 | 查询标志 |
| `-u` | `--user-id` | integer | 否 | 整数，默认 100 | 用户 ID |
| `-b` | `--bundle` | string | 条件必填 | 非空 | 包名（模式 2、3 必填） |
| `-m` | `--module` | string | 条件必填 | 非空 | 模块名（模式 3 必填） |
| `-i` | `--intent` | string | 条件必填 | 非空 | 意图名称（模式 3 必填） |
| `-h` | `--help` | - | - | - | 显示帮助信息后退出 |

### 查询模式 (`-e`)

| 值 | 对应 API | 必填附加参数 | 返回类型 |
|----|----------|-------------|----------|
| 1 | `GetAllInsightIntentInfo` | 无 | `vector<InsightIntentInfoForQuery>` |
| 2 | `GetInsightIntentInfoByBundleName` | `-b` | `vector<InsightIntentInfoForQuery>` |
| 3 | `GetInsightIntentInfoByIntentName` | `-b` `-m` `-i` | `InsightIntentInfoForQuery`（单个） |

### 查询标志 (`-f`)

控制服务端返回数据的详细程度。三个基础值可通过位运算组合使用。

| 值 | 枚举名 | 说明 |
|----|--------|------|
| 1 | `GET_FULL_INSIGHT_INTENT` | 全量意图信息（除 entities 和 subIntentInfoForConfiguration） |
| 2 | `GET_SUMMARY_INSIGHT_INTENT` | 简要意图信息 |
| 4 | `GET_ENTITY_INFO` | 实体信息，不可单独使用，需与 1 或 2 组合 |

> **说明：**
>
> - 对于使用配置文件开发的意图，全量信息和简要信息完全一致。
> - 对于使用装饰器开发的意图，全量与简要存在差别，详见下方输出字段表。
> - `GET_ENTITY_INFO` 不可单独使用，例如 `1|4=5` 表示全量 + 实体，`2|4=6` 表示简要 + 实体。

### 输出格式

#### 模式 1 / 模式 2 输出（列表）

```
get insight intent info successfully (mode: all).       -- 或 (mode: by bundle)
Total count: <int>
BundleName: <string>                                     -- 仅模式 2

--- Insight Intent 1 ---
<InsightIntentInfoForQuery 详细字段，见下方>

--- Insight Intent 2 ---
...
```

#### 模式 3 输出（单条）

```
get insight intent info successfully (mode: by intent).

--- Insight Intent ---
<InsightIntentInfoForQuery 详细字段，见下方>
```

#### InsightIntentInfoForQuery 详细字段

每条意图信息按以下区块顺序输出。字段是否输出取决于 `-f` 标志值：

| 字段 | 全量 (flag=1) | 简要 (flag=2) | 说明 |
|------|:---:|:---:|------|
| BundleName | ✓ | ✓ | 包名 |
| ModuleName | ✓ | ✓ | 模块名 |
| IntentName | ✓ | ✓ | 意图名称 |
| DisplayName | ✓ | ✓ | 显示名称 |
| Domain | ✓ | ✗ | 垂域 |
| IntentVersion | ✓ | ✗ | 意图版本号 |
| IntentType | ✓ | ✓ | 意图类型 |
| DisplayDescription | ✓ | ✗ | 描述 |
| Schema | ✓ | ✗ | 标准意图名称 |
| Icon | ✓ | ✗ | 意图图标 |
| LlmDescription | ✓ | ✗ | LLM 功能描述 |
| Keywords | ✓ | ✗ | 搜索关键字 |
| DevelopType | ✓ | ✓ | 开发类型 |
| Parameters | ✓ | ✓ | 参数描述 |
| subIntentInfo | ✓ | ✓ | 子意图信息（见下方区块） |
| entities | ✗ | ✗ | 需组合 GET_ENTITY_INFO (4) |

##### 基础信息（始终输出）

```
BundleName: <string>          # 包名
ModuleName: <string>          # 模块名
IntentName: <string>          # 意图名称
DisplayName: <string>         # 显示名称
Domain: <string>              # 垂域                  (仅 flag=1)
IntentVersion: <string>       # 意图版本号             (仅 flag=1)
IntentType: <string>          # 意图类型
```

##### 详细信息

```
DisplayDescription: <string>  # 描述                  (仅 flag=1)
Schema: <string>              # 标准意图名称           (仅 flag=1)
Icon: <string>                # 意图图标               (仅 flag=1)
LlmDescription: <string>      # LLM 功能描述           (仅 flag=1)
Keywords: <item1>, <item2>    # 搜索关键字列表          (仅 flag=1, 非空时输出)
DevelopType: <string>         # 开发类型
Parameters: <string>          # 参数描述
```

##### LinkInfo 区块（条件输出）

当 `linkInfo.uri` **非空**时输出：

```
[LinkInfo]
  Uri: <string>                # 关联 URI
```

##### PageInfo 区块（条件输出）

当 `pageInfo.uiAbility` 和 `pageInfo.pagePath` **不全为空**时输出：

```
[PageInfo]
  UIAbility: <string>           # UIAbility 名称
  PagePath: <string>            # 页面路径
  NavigationId: <string>        # Navigation 组件 ID
  NavDestination: <string>      # NavDestination 名称
```

| 字段 | 类型 | 来源 |
|------|------|------|
| UIAbility | string | `pageInfo.uiAbility` |
| PagePath | string | `pageInfo.pagePath` |
| NavigationId | string | `pageInfo.navigationId` |
| NavDestination | string | `pageInfo.navDestinationName` |

##### EntryInfo 区块（条件输出）

当 `entryInfo.abilityName` **非空**时输出：

```
[EntryInfo]
  AbilityName: <string>         # EntryAbility 名称
  ExecuteMode: <m1>, <m2>       # 支持的执行模式列表
```

| 字段 | 类型 | 说明 |
|------|------|------|
| AbilityName | string | `entryInfo.abilityName` |
| ExecuteMode | int[] | `entryInfo.executeMode`，值为 ExecuteMode 枚举 (0-3) |

##### FormInfo 区块（条件输出）

当 `formInfo.abilityName` **非空**时输出：

```
[FormInfo]
  AbilityName: <string>         # FormAbility 名称
  FormName: <string>            # 卡片名称
```

| 字段 | 类型 | 来源 |
|------|------|------|
| AbilityName | string | `formInfo.abilityName` |
| FormName | string | `formInfo.formName` |

##### Entities 区块（条件输出）

当 `entities` 列表**非空**时输出：

```
[Entities] count: <int>
  [1]
    ClassName: <string>           # 实体类名
    EntityId: <string>            # 实体 ID
    EntityCategory: <string>      # 实体分类
    Parameters: <string>          # 实体参数
    ParentClass: <string>         # 父类名
  [2]
    ...
```

| 字段 | 类型 | 来源 |
|------|------|------|
| ClassName | string | `entity.className` |
| EntityId | string | `entity.entityId` |
| EntityCategory | string | `entity.entityCategory` |
| Parameters | string | `entity.parameters` |
| ParentClass | string | `entity.parentClassName` |

#### 错误输出

**参数解析阶段**:

| 场景 | 输出 |
|------|------|
| 缺少 `-e` 或 `-f` | `error: missing required parameters -e and -f.\n` |
| 无效整数值 | `error: invalid execute mode value.\n` / `error: invalid flag value.\n` / `error: invalid user id value.\n` |
| 未知选项 | `fail: unknown option.\n` |
| 模式 2 缺包名 | `error: mode 2 requires -b <bundle-name>.\n` |
| 模式 3 缺参数 | `error: mode 3 requires -b <bundle-name> -m <module-name> -i <intent-name>.\n` |
| 无效模式值 | `error: invalid execute mode. Must be 1, 2, or 3.\n` |

**查询阶段**:

| 场景 | 输出 |
|------|------|
| 查询失败 | `error: failed to get insight intent info.\n` + 错误码描述 |
| 按包名查询失败 | `error: failed to get insight intent info by bundle.\n` + 错误码描述 |
| 按意图名查询失败 | `error: failed to get insight intent info by intent name.\n` + 错误码描述 |
| 意图名不存在（API 返回空） | `error: intent not found.\nPlease check bundleName, moduleName and intentName.\n` |

### 示例

```bash
# 查询所有意图（全量信息）
ohos-claw-cc get-intent -e 1 -f 1

# 查询所有意图（简要信息）
ohos-claw-cc get-intent -e 1 -f 2

# 按包名查询（全量信息）
ohos-claw-cc get-intent -e 2 -f 1 -b com.example.app

# 按意图名精确查询
ohos-claw-cc get-intent -e 3 -f 1 -b com.example.app -m entry -i MyIntent

# 查询全量 + 实体信息（1|4=5）
ohos-claw-cc get-intent -e 3 -f 5 -b com.example.app -m entry -i MyIntent

# 查询简要 + 实体信息（2|4=6）
ohos-claw-cc get-intent -e 3 -f 6 -b com.example.app -m entry -i MyIntent

# 指定用户 ID
ohos-claw-cc get-intent -e 1 -f 1 -u 200

# 查看帮助
ohos-claw-cc get-intent -h
```

---

## 通用行为

### 参数解析

- 使用 `getopt_long` 进行标准 POSIX 风格参数解析
- 整数参数通过 `strtol` 解析，溢出时返回错误
- `-h` / `--help` 在任何子命令中均可用
- 参数顺序无关

### 退出码

| 退出码 | 含义 |
|--------|------|
| 0 | 命令执行成功 |
| 非 0 | 命令执行失败（参数错误、服务端错误等） |

### 服务连接

工具启动时通过 `AbilityManagerClient::Connect()` 连接 `AbilityManagerService`，连接失败时输出：

```
error: failed to execute your command.
```

---

## 源码结构

```
tools/cc/
├── BUILD.gn                    # 构建配置（目标名 ohos-claw-cc）
├── include/
│   ├── cc_command.h            # 命令入口、帮助文本、类声明
│   └── cc_param_parser.h       # JSON → WantParams 解析器声明
├── src/
│   ├── main.cpp                # 入口 main()
│   ├── cc_command.cpp          # 命令解析与执行逻辑
│   └── cc_param_parser.cpp     # JSON → WantParams 转换实现
└── skills.md                   # 本文件
```

## 关键依赖

| 依赖 | 用途 |
|------|------|
| `ability_manager` (inner_api) | `AbilityManagerClient` IPC 通信 |
| `ability_base:want` | Want / WantParams 参数构造 |
| `json:nlohmann_json_static` | JSON 参数解析 |
| `hilog:libhilog` | 日志输出 |
| `ipc:ipc_core` | IPC 基础设施 |
| `samgr:samgr_proxy` | 系统服务发现 |

## 权限说明

| 调用来源 | 权限要求 | 校验方式 |
|----------|----------|----------|
| Shell / hdc | 无 | 服务端 `IsShellCall()` 判定后跳过权限校验 |
| 系统应用 | `ohos.permission.EXECUTE_INSIGHT_INTENT` | `VerifyCallingPermission` + `JudgeCallerIsAllowedToUseSystemAPI` |
| 第三方应用 | 不允许 | 返回 `ERR_NOT_SYSTEM_APP` |

---

## ohos-claw-cc 测试清单

### 一、Help 命令（已有 3 个测试）

| # | 测试场景 | 状态 |
|---|---------|------|
| 1 | `ohos-claw-cc help` 显示帮助信息 | ✅ 0100 |
| 2 | 无效子命令显示错误+帮助 | ✅ 0200 |
| 3 | `ohos-claw-cc` 无子命令显示帮助 | ✅ 0300 |
| 4 | `ohos-claw-cc execute-intent -h` 显示子命令帮助 | ✅ (execute 0100) |
| 5 | `ohos-claw-cc get-intent -h` 显示子命令帮助 | ✅ (get 0100) |

### 二、execute-intent 参数解析（已有 7 个测试）

| # | 测试场景 | 状态 |
|---|---------|------|
| 1 | 缺少所有必选参数 -b -m -i 报错 | ✅ 0200 |
| 2 | 缺少 -i 报错 | ✅ 0300 |
| 3 | 缺少 -m 报错 | ✅ 0400 |
| 4 | 缺少 -b 报错 | ✅ 0500 |
| 5 | -e 值非法（非数字）报错 | ✅ 0600 |
| 6 | 未知选项报错 | ✅ 0700 |
| 7 | 无任何选项执行报错 | ✅ 0200(重复) |
| 8 | -e 值为空字符串 | ❌ 缺失 |
| 9 | -p JSON 格式非法 | ❌ 缺失 |
| 10 | -p JSON 为空对象 `{}` | ❌ 缺失 |
| 11 | -p JSON 包含嵌套对象 | ❌ 缺失 |
| 12 | -p JSON 包含数组 | ❌ 缺失 |
| 13 | -p JSON 包含布尔/null/浮点类型 | ❌ 缺失 |
| 14 | -b -m -i 为空字符串 | ❌ 缺失 |

### 三、execute-intent 自动解析（已有 8 个测试）

| # | 测试场景 | 状态 |
|---|---------|------|
| 1 | 查询返回空，自动解析失败 | ✅ 0800 |
| 2 | abilityName 在所有来源中都找不到 | ✅ 0900 |
| 3 | 查询返回有效信息，单模式自动解析成功 | ✅ 1000 |
| 4 | 多个 executeMode 无法自动解析 | ✅ 1200 |
| 5 | 查询中无 executeMode | ✅ 1300 |
| 6 | abilityName 从 pageInfo 兜底 | ✅ 1500 |
| 7 | abilityName 从 entryInfo 兜底 | ✅ 1600 |
| 8 | abilityName 从 formInfo 兜底 | ✅ 1700 |
| 9 | abilityName 从 uiAbilityIntentInfo 优先 | ❌ 缺失 |
| 10 | 显式指定 -a 跳过自动解析 | ❌ 缺失 |

### 四、execute-intent 执行（已有 2 个测试）

| # | 测试场景 | 状态 |
|---|---------|------|
| 1 | 全量参数显式指定执行成功 | ✅ 1100 |
| 2 | 带 JSON param 执行 | ✅ 1400 |
| 3 | 执行结果包含 innerErr/code/flags | ✅ 1100(包含) |
| 4 | 执行结果包含 result WantParams | ❌ 缺失 |
| 5 | 执行结果包含 uris 列表 | ❌ 缺失 |
| 6 | 执行超时返回错误 | ❌ 缺失 |
| 7 | 服务端连接失败 | ❌ 缺失 |
| 8 | -e 0 (UI_ABILITY_FOREGROUND) 执行 | ❌ 缺失 |
| 9 | -e 1 (UI_ABILITY_BACKGROUND) 执行 | ❌ 缺失 |
| 10 | -e 2 (UI_EXTENSION_ABILITY) 执行 | ❌ 缺失 |
| 11 | -e 3 (SERVICE_EXTENSION_ABILITY) 执行 | ❌ 缺失 |

### 五、get-intent 参数解析（已有 7 个测试）

| # | 测试场景 | 状态 |
|---|---------|------|
| 1 | 缺少必选 -e -f 报错 | ✅ 0200 |
| 2 | 缺少 -f 报错 | ✅ 0300 |
| 3 | -e 非法值报错 | ✅ 0400 |
| 4 | -f 非法值报错 | ✅ 0500 |
| 5 | -u 非法值报错 | ✅ 0600 |
| 6 | 未知选项报错 | ✅ 2300 |
| 7 | 无效 execute mode (4) 报错 | ✅ 1400 |

### 六、get-intent 查询（已有 11 个测试）

| # | 测试场景 | 状态 |
|---|---------|------|
| 1 | mode 1 查询全部成功 | ✅ 0800 |
| 2 | mode 1 空结果 | ✅ 0900 |
| 3 | mode 2 缺少 -b 报错 | ✅ 1000 |
| 4 | mode 2 按包名查询成功 | ✅ 1100 |
| 5 | mode 3 缺少必选参数报错 | ✅ 1200 |
| 6 | mode 3 按意图名查询成功 | ✅ 1300 |
| 7 | mode 1 服务失败 | ✅ 1500 |
| 8 | mode 2 服务失败 | ✅ 1600 |
| 9 | mode 3 服务失败 | ✅ 1700 |
| 10 | mode 1 自定义 userId | ✅ 2400 |
| 11 | mode 1 多条意图显示计数 | ✅ 2500 |
| 12 | mode 2 bundleName 不存在 | ❌ 缺失 |
| 13 | mode 3 intentName 不存在（返回空对象） | ❌ 缺失 |

### 七、get-intent 输出格式（已有 6 个测试）

| # | 测试场景 | 状态 |
|---|---------|------|
| 1 | 输出包含基本信息字段 | ✅ 1800 |
| 2 | 输出包含 PageInfo | ✅ 1900 |
| 3 | 输出包含 EntryInfo | ✅ 2000 |
| 4 | 输出包含 Entities（flag 含 4） | ✅ 2200 |
| 5 | 输出包含 FormInfo | ✅ 2500(部分) |
| 6 | 输出包含 LinkInfo（linkInfo.uri 非空时） | ❌ 缺失 |
| 7 | flag=1 全量输出包含 Domain/IntentVersion/Schema/Icon 等 | ❌ 缺失 |
| 8 | flag=2 简要输出不包含 Domain/IntentVersion/Schema 等 | ❌ 缺失 |
| 9 | flag=1 全量输出包含 Keywords | ❌ 缺失 |
| 10 | flag=2 简要输出不包含 Keywords | ❌ 缺失 |
| 11 | PageInfo 为空时不输出该段 | ❌ 缺失 |
| 12 | EntryInfo 为空时不输出该段 | ❌ 缺失 |
| 13 | Entities 仅在 flag 含 4 时输出 | ❌ 缺失 |

### 八、flag 类型覆盖（未覆盖）

| # | flag 值 | 含义 | 状态 |
|---|---------|------|------|
| 1 | 1 | GET_FULL_INSIGHT_INTENT | ✅ 已在多处使用 |
| 2 | 2 | GET_SUMMARY_INSIGHT_INTENT | ❌ 缺失 |
| 3 | 4 | GET_ENTITY_INFO | ❌ 缺失 |
| 4 | 5 (1\|4) | GET_FULL + GET_ENTITY | ❌ 缺失 |
| 5 | 6 (2\|4) | GET_SUMMARY + GET_ENTITY | ❌ 缺失 |

### 统计

| 类别 | 已有 | 缺失 | 总计 |
|------|------|------|------|
| help | 3 | 0 | 3 |
| execute-intent 参数解析 | 7 | 7 | 14 |
| execute-intent 自动解析 | 8 | 2 | 10 |
| execute-intent 执行 | 2 | 8 | 10 |
| get-intent 参数解析 | 7 | 0 | 7 |
| get-intent 查询 | 11 | 2 | 13 |
| get-intent 输出格式 | 5 | 8 | 13 |
| flag 类型覆盖 | 1 | 4 | 5 |
| **合计** | **45** | **33** | **78** |

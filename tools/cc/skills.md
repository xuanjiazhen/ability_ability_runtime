# claw_cc 命令行工具功能说明

## 概述

`claw_cc` 是 OpenHarmony InsightIntent 框架的命令行调试工具，用于执行意图和查询意图信息。工具通过 `AbilityManagerClient` 与 `AbilityManagerService` 通信，支持 Shell 调用者免权限使用。

- **可执行文件**: `/system/bin/claw_cc`
- **源码路径**: `foundation/ability/ability_runtime/tools/cc/`
- **日志标签**: `AAFwkTag::CC_TOOL`

## 命令列表

```
claw_cc <command> <options>

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
claw_cc help
```

无额外参数。

### 输出格式

```
usage: claw_cc <command> <options>
These are common claw_cc commands list:
  help                        list available commands
  execute-intent              execute insight intent with result
  get-intent                  get all insight intent info
```

### 错误输出

当传入不存在的命令时：

```
claw_cc: '<command>' is not a valid claw_cc command. See 'claw_cc help'.
<HELP_MSG>
```

---

## 2. execute-intent

执行一个洞察意图，同步等待执行结果返回（超时 30 秒）。

### 入参格式

```
claw_cc execute-intent -b <bundle> -m <module> -i <intent>
    [-a <ability>] [-e <mode>] [-p <json>] [-h]
```

### 参数规格

| 短选项 | 长选项 | 类型 | 必填 | 校验规则 | 说明 |
|--------|--------|------|------|----------|------|
| `-b` | `--bundle` | string | **是** | 非空 | 目标应用包名 |
| `-m` | `--module` | string | **是** | 非空 | 模块名 |
| `-i` | `--intent` | string | **是** | 非空 | 洞察意图名称 |
| `-a` | `--ability` | string | 否 | 非空 | Ability 名称，省略时自动解析 |
| `-e` | `--execute-mode` | integer | 否 | 0-3，`std::from_chars` 解析 | 执行模式，省略时自动解析 |
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
claw_cc execute-intent -b com.example.app -m entry -i MyIntent

# 指定所有参数
claw_cc execute-intent \
    -b com.example.app \
    -m entry \
    -a MainAbility \
    -i MyIntent \
    -e 0

# 后台执行 + 传递参数
claw_cc execute-intent \
    -b com.example.app \
    -m entry \
    -i MyIntent \
    -e 1 \
    -p '{"city":"Beijing","days":3}'

# 嵌套参数
claw_cc execute-intent \
    -b com.example.app \
    -m entry \
    -i MyIntent \
    -p '{"user":{"name":"test","age":20},"tags":["a","b","c"]}'

# 查看帮助
claw_cc execute-intent -h
```

---

## 3. get-intent

查询洞察意图注册信息，支持三种查询模式和六种查询粒度。

### 入参格式

```
claw_cc get-intent -e <query-mode> -f <flag>
    [-u <user-id>] [-b <bundle>] [-m <module>] [-i <intent>] [-h]
```

### 参数规格

| 短选项 | 长选项 | 类型 | 必填 | 校验规则 | 说明 |
|--------|--------|------|------|----------|------|
| `-e` | `--execute-mode` | integer | **是** | 1/2/3，`std::from_chars` 解析 | 查询模式 |
| `-f` | `--flag` | integer | **是** | 1/2/4/5/6，`std::from_chars` 解析 | 查询标志 |
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

控制服务端返回数据的详细程度。

| 值 | 枚举名 | 说明 |
|----|--------|------|
| 1 | `GET_FULL_INSIGHT_INTENT` | 完整意图信息 |
| 2 | `GET_SUMMARY_INSIGHT_INTENT` | 摘要信息（精简） |
| 4 | `GET_ENTITY_INFO` | 仅实体信息 |
| 5 | `GET_FULL_INSIGHT_INTENT_ENTITY` | 完整意图 + 实体信息 |
| 6 | `GET_SUMMARY_INSIGHT_INTENT_ENTITY` | 摘要 + 实体信息 |

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

每条意图信息按以下区块顺序输出，**区块内的字段始终输出**（即使为空字符串），**整个区块仅在有有效数据时输出**。

##### 基础信息（始终输出）

```
BundleName: <string>          # 包名
ModuleName: <string>          # 模块名
IntentName: <string>          # 意图名称
DisplayName: <string>         # 显示名称
Domain: <string>              # 领域
IntentVersion: <string>       # 意图版本号
IntentType: <string>          # 意图类型
```

##### 详细信息（始终输出）

```
SrcEntry: <string>            # 源码入口路径
Description: <string>         # 描述
Schema: <string>              # Schema 定义
DeveloType: <string>          # 开发类型
Parameters: <string>          # 参数描述
Result: <string>              # 返回值描述
Keywords: <item1>, <item2>    # 关键词列表（非空时输出）
InputParams: <p1>, <p2>       # 输入参数列表（非空时输出）
OutputParams: <o1>, <o2>      # 输出参数列表（非空时输出）
CfgEntities: <string>         # 配置实体
LinkUri: <string>             # 关联 URI
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

##### UIAbilityInfo 区块（条件输出）

当 `uiAbilityIntentInfo.abilityName` **非空**时输出：

```
[UIAbilityInfo]
  AbilityName: <string>           # UIAbility 名称
  SupportExecuteMode: <m1>, <m2>  # 支持的执行模式列表
```

| 字段 | 类型 | 说明 |
|------|------|------|
| AbilityName | string | `uiAbilityIntentInfo.abilityName` |
| SupportExecuteMode | int[] | `uiAbilityIntentInfo.supportExecuteMode` |

此区块后紧跟扩展信息（条件输出）：

```
[UIExtensionInfo]
  AbilityName: <string>          # uiExtensionIntentInfo.abilityName（非空时输出）

[ServiceExtensionInfo]
  AbilityName: <string>          # serviceExtensionIntentInfo.abilityName（非空时输出）

[FormIntentInfo]
  AbilityName: <string>          # formIntentInfo.abilityName（非空时输出）
  FormName: <string>             # formIntentInfo.formName（非空时输出）
```

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

### 示例

```bash
# 查询所有意图（完整信息）
claw_cc get-intent -e 1 -f 1

# 查询所有意图（摘要信息）
claw_cc get-intent -e 1 -f 2

# 按包名查询（完整 + 实体信息）
claw_cc get-intent -e 2 -f 5 -b com.example.app

# 按意图名精确查询
claw_cc get-intent -e 3 -f 1 -b com.example.app -m entry -i MyIntent

# 查询摘要 + 实体信息
claw_cc get-intent -e 3 -f 6 -b com.example.app -m entry -i MyIntent

# 指定用户 ID
claw_cc get-intent -e 1 -f 1 -u 200

# 查看帮助
claw_cc get-intent -h
```

---

## 通用行为

### 参数解析

- 使用 `getopt_long` 进行标准 POSIX 风格参数解析
- 整数参数通过 `std::from_chars` 解析，溢出时返回错误
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
├── BUILD.gn                    # 构建配置（目标名 claw_cc）
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

# config.json 校验规则

## 1. 适用范围

本文档定义 `config.json` 的正式校验规则，用于约束 CLI 工具配置文件格式、参数映射语义和事件声明方式。

- 规则中列出的预置字段按严格模式处理
- 顶层和子命令中的非预置扩展字段允许存在，但框架不做解析
- 发现任一错误即认为配置非法
校验流程：

1. 文件与 JSON 基础校验
2. 顶层公共字段校验
3. 工具格式判定校验
4. 子命令定义校验
5. JSON Schema 子集校验
6. `input` / `output` 根节点校验
7. `argumentMapping` 校验
8. `input` 与 `argumentMapping` 关联校验
9. `events` / `eventSchemas` / `output` 校验

---

## 2. 文件与 JSON 基础校验

| 错误码 | 规则 | 覆盖用例 | 未覆盖原因 |
|------|------|------|------|
| `CFG001` | 文件不存在或不可读 | true | - |
| `CFG002` | 文件内容不是合法 JSON | true | - |
| `CFG003` | 根节点不是 JSON object | true | - |
| `CFG004` | 根节点为空对象 | true | - |
| `CFG005` | 文件编码不是 UTF-8 | true | - |
| `CFG006` | JSON 中存在重复 key | true | - |

---

## 3. 顶层公共字段校验

### 3.1 顶层预置字段

顶层预置字段如下：

- `name`
- `version`
- `description`
- `executable`
- `requirePermissions`
- `timeout`
- `events`
- `eventSchemas`
- `hasSubcommands`
- `input`
- `output`
- `argumentMapping`
- `subcommands`

说明：顶层允许增加非预置扩展字段，例如 `private`。非预置字段框架不做解析，也不参与校验和运行时行为。

### 3.2 通用字段校验规则

| 错误码 | 规则 | 覆盖用例 | 未覆盖原因 |
|------|------|------|------|
| `TOP001` | `name` 必填，必须是 `prefix-toolName` 格式的非空 string | true | - |
| `TOP002` | `version` 必填，必须是非空 string | true | - |
| `TOP003` | `description` 必填，必须是非空 string | true | - |
| `TOP004` | `executable` 必填，必须是非空 string | true | - |
| `TOP005` | `executable` 必须是绝对路径 | true | - |
| `TOP006` | `requirePermissions` 若存在，必须是 string 数组 | true | - |
| `TOP007` | `requirePermissions` 不允许重复值 | true | - |
| `TOP008` | `timeout` 若存在，必须是 integer | true | - |
| `TOP009` | `timeout` 若存在，必须大于等于 `0` | true | - |
| `TOP010` | `events` 若存在，必须是 string 数组 | true | - |
| `TOP011` | `events` 不允许重复值 | true | - |
| `TOP012` | `eventSchemas` 若存在，必须是 object | true | - |
| `TOP013` | 顶层允许出现非预置扩展字段；框架默认放行，但不做解析、不参与校验和运行时行为 | true | - |
| `TOP014` | `name` 中 `prefix` 只能是 `ohos` 或 `hms` | true | - |
| `TOP015` | `name` 中 `toolName` 必须非空，且长度不超过 `16` 个字符 | true | - |


### 3.3 命令描述字段摘要

顶层命令和子命令都可以定义以下参数描述字段：

- `requirePermissions`
- `timeout`
- `input`
- `argumentMapping`
- `events`
- `eventSchemas`
- `output`

这些字段的细则统一在后续章节定义：

- `input` / `output` 根节点约束见第 `7` 节
- `argumentMapping` 规则见第 `8` 节
- `input` 与 `argumentMapping` 关联规则见第 `9` 节
- `events` / `eventSchemas` / `output` 规则见第 `10` 节

---

## 4. 工具格式判定校验

当前只支持无子命令工具和有子命令工具两种合法格式。

### 4.1 无子命令工具

判定条件：

- `hasSubcommands` 不存在，或值为 `false`

校验规则：

| 错误码 | 规则 | 覆盖用例 | 未覆盖原因 |
|------|------|------|------|
| `FMT001` | 顶层必须存在 `input`，且必须是合法 object Schema；零参数时允许 `input.properties = {}` | true | - |
| `FMT002` | 顶层必须存在 `argumentMapping` | true | - |
| `FMT003` | 顶层不允许存在 `subcommands` | true | - |
| `FMT004` | 顶层 `output` 若存在，必须是合法 Schema，且根节点必须为 `object` | true | - |
| `FMT005` | 顶层 `hasSubcommands` 若存在，必须是 boolean，且值为 `false` | true | - |

### 4.2 有子命令工具

判定条件：

- `hasSubcommands` 必须为 `true`

校验规则：

| 错误码 | 规则 | 覆盖用例 | 未覆盖原因 |
|------|------|------|------|
| `FMT101` | 顶层必须存在 `subcommands` | true | - |
| `FMT102` | `subcommands` 必须是 object | true | - |
| `FMT103` | `subcommands` 不允许为空 object | true | - |
| `FMT104` | 顶层 `input` 若定义，必须是合法 object Schema | true | - |
| `FMT105` | 顶层 `input` 与 `argumentMapping` 若定义则必须同时出现 | true | - |
| `FMT106` | 顶层 `output` 若存在，必须是合法 Schema，且根节点必须为 `object` | true | - |
| `FMT107` | 每个子命令名必须是非空 string | true | - |
| `FMT108` | 子命令名不能重复 | true | - |
| `FMT109` | 子命令名不能与父命令 `name` 重复 | true | - |

---

## 5. 子命令定义校验

### 5.1 子命令预置字段

`subcommands.<name>` 预置字段如下：

- `description`
- `requirePermissions`
- `timeout`
- `input`
- `output`
- `events`
- `eventSchemas`
- `argumentMapping`

说明：子命令允许增加非预置扩展字段，例如 `private`。非预置字段框架不做解析，也不参与校验和运行时行为。

### 5.2 校验规则

| 错误码 | 规则 | 覆盖用例 | 未覆盖原因 |
|------|------|------|------|
| `SUB001` | 子命令节点必须是 object | true | - |
| `SUB002` | 子命令必须存在 `description` | true | - |
| `SUB003` | 子命令 `description` 必须是非空 string | true | - |
| `SUB004` | 子命令必须存在 `input` | true | - |
| `SUB005` | 子命令必须存在 `argumentMapping` | true | - |
| `SUB006` | 子命令 `output` 若存在，必须是合法 Schema，且根节点必须为 `object` | true | - |
| `SUB007` | 子命令允许出现非预置扩展字段；框架默认放行，但不做解析、不参与校验和运行时行为 | true | - |
| `SUB008` | 子命令 `events` 若存在，必须是 string 数组，且不允许重复值 | true | - |
| `SUB009` | 子命令 `eventSchemas` 若存在，必须是 object | true | - |
| `SUB010` | 子命令 `requirePermissions` 若存在，必须是 string 数组，且不允许重复值 | true | - |
| `SUB011` | 子命令 `timeout` 若存在，必须是 integer 且大于等于 `0` | true | - |

### 5.3 作用域规则

| 规则 | 说明 |
|------|------|
| `SUB101` | 子命令 `input` 和 `argumentMapping` 必填；零参数时 `input` 仍需是合法 object Schema，但允许 `input.properties = {}`，同时 `argumentMapping = {}` |
| `SUB102` | 顶层与子命令配置相互独立；指定 `subcommand` 时只使用子命令配置，未指定 `subcommand` 时只使用顶层配置 |
| `SUB103` | 有子命令工具可以同时定义父命令；未指定 `subcommand` 时执行父命令配置 |

---

## 6. JSON Schema 子集校验

校验器不支持完整 JSON Schema，只支持当前文档规定的受限子集。

### 6.1 通用规则

| 错误码 | 规则 | 覆盖用例 | 未覆盖原因 |
|------|------|------|------|
| `SCH001` | Schema 节点必须是 object | true | - |
| `SCH002` | Schema 节点必须存在 `type` | true | - |
| `SCH003` | `type` 只允许 `string`、`number`、`integer`、`boolean`、`array`、`object` | true | - |
| `SCH004` | 不允许出现未支持的 Schema keyword | true | - |
| `SCH005` | `description` 若存在，必须是 string | true | - |
| `SCH006` | `default` 若存在，类型必须与 `type` 一致 | true | - |
| `SCH007` | `enum` 采用 JSON Schema 设计格式预留，但当前版本不支持使用 | true | - |
| `SCH008` | `union` 采用 JSON Schema 联合类型设计格式预留，如 `oneOf` / `anyOf` / `allOf`，但当前版本不支持使用 | true | - |

### 6.2 string 类型

允许字段：

- `type`
- `description`
- `default`
- `minLength`
- `maxLength`
- `pattern`
- `format`

校验规则：

| 错误码 | 规则 | 覆盖用例 | 未覆盖原因 |
|------|------|------|------|
| `SCH101` | `minLength` 若存在，必须是 integer 且大于等于 `0` | true | - |
| `SCH102` | `maxLength` 若存在，必须是 integer 且大于等于 `0` | true | - |
| `SCH103` | `minLength` 不得大于 `maxLength` | true | - |
| `SCH104` | `pattern` 若存在，必须是合法正则字符串 | true | - |
| `SCH105` | `format` 若存在，必须是以下支持值之一：`ipv4`、`ipv6`、`hostname`、`email`、`uri`、`date`、`date-time` | true | - |

### 6.3 number 类型

允许字段：

- `type`
- `description`
- `default`
- `minimum`
- `maximum`
- `exclusiveMinimum`
- `exclusiveMaximum`
- `multipleOf`

校验规则：

| 错误码 | 规则 | 覆盖用例 | 未覆盖原因 |
|------|------|------|------|
| `SCH201` | `minimum` / `maximum` 若存在，必须是 number | true | - |
| `SCH202` | `exclusiveMinimum` / `exclusiveMaximum` 若存在，必须是 number | true | - |
| `SCH203` | `minimum` 不得大于 `maximum` | true | - |
| `SCH204` | `exclusiveMinimum` 不得大于等于 `exclusiveMaximum` | true | - |
| `SCH205` | `multipleOf` 若存在，必须是正数 | true | - |

### 6.4 integer 类型

允许字段与 `number` 相同，但值必须满足整数语义。

校验规则：

| 错误码 | 规则 | 覆盖用例 | 未覆盖原因 |
|------|------|------|------|
| `SCH301` | `minimum` / `maximum` / `exclusiveMinimum` / `exclusiveMaximum` / `multipleOf` 若存在，必须是整数 | true | - |
| `SCH302` | `multipleOf` 必须大于 `0` | true | - |
| `SCH303` | `minimum` 不得大于 `maximum` | true | - |

### 6.5 boolean 类型

允许字段：

- `type`
- `description`
- `default`

校验规则：

| 错误码 | 规则 | 覆盖用例 | 未覆盖原因 |
|------|------|------|------|
| `SCH401` | boolean 类型不允许出现不适用约束字段，如 `minLength`、`minimum` | true | - |
| `SCH402` | `default` 若存在，必须是 boolean | true | - |

### 6.6 array 类型

允许字段：

- `type`
- `description`
- `default`
- `items`
- `minItems`
- `maxItems`
- `uniqueItems`

校验规则：

| 错误码 | 规则 | 覆盖用例 | 未覆盖原因 |
|------|------|------|------|
| `SCH501` | `items` 必须存在 | true | - |
| `SCH502` | `items` 必须是合法 Schema object | true | - |
| `SCH503` | `minItems` / `maxItems` 若存在，必须是 integer 且大于等于 `0` | true | - |
| `SCH504` | `minItems` 不得大于 `maxItems` | true | - |
| `SCH505` | `uniqueItems` 若存在，必须是 boolean | true | - |
| `SCH506` | `default` 若存在，必须是 array | true | - |
| `SCH507` | `items.type` 只允许简单类型：`boolean`、`integer`、`number`、`string` | true | - |
| `SCH508` | array 的 `default` 若存在，其中每个元素都必须与 `items.type` 一致 | true | - |
| `SCH509` | `uniqueItems` 为 `true` 时，array 的 `default` 若存在，则其中不允许出现重复值 | true | - |

说明：`SCH507` 用于限制数组元素类型必须是简单类型。

### 6.7 object 类型

允许字段：

- `type`
- `description`
- `properties`
- `required`

校验规则：

| 错误码 | 规则 | 覆盖用例 | 未覆盖原因 |
|------|------|------|------|
| `SCH601` | `properties` 若存在，必须是 object | true | - |
| `SCH602` | `properties` 中每个 value 都必须是合法 Schema object | true | - |
| `SCH603` | `required` 若存在，必须是 string 数组 | true | - |
| `SCH604` | `required` 中每个字段都必须出现在 `properties` | true | - |
| `SCH605` | `required` 不允许重复 | true | - |
| `SCH606` | object 类型必须显式定义 `properties` | true | - |

---

## 7. input / output 根节点约束

| 错误码 | 规则 | 覆盖用例 | 未覆盖原因 |
|------|------|------|------|
| `SCX001` | 顶层 `input.type` 必须是 `object` | true | - |
| `SCX002` | 顶层 `input.properties` 必须存在且必须是 object | true | - |
| `SCX003` | 顶层 `output` 若存在，其根节点必须是 `object` | true | - |
| `SCX004` | 子命令 `input.type` 必须是 `object` | true | - |
| `SCX005` | 子命令 `input.properties` 必须存在且必须是 object | true | - |

说明：

- `input = {}` 不是合法零参数写法
- 零参数模式必须写成 `input.type = object`、`input.properties = {}`、`argumentMapping = {}`

---

## 8. argumentMapping 校验

### 8.1 支持的映射类型

当前支持以下五种：

- `flag`
- `positional`
- `flattened`
- `json-string`
- `mixed`

### 8.2 参数映射模板语言

- 当前只允许 `{value}` 作为模板占位符
- 对于 `object` / `array`，实际映射时先序列化为 JSON 字符串，再替换 `{value}`

### 8.3 通用规则

| 错误码 | 规则 | 覆盖用例 | 未覆盖原因 |
|------|------|------|------|
| `MAP001` | `argumentMapping` 必须是 object；零参数时允许为 `{}` | true | - |
| `MAP002` | `argumentMapping.type` 必填 | true | - |
| `MAP003` | `argumentMapping.type` 只允许 `flag`、`positional`、`flattened`、`json-string`、`mixed` | true | - |
| `MAP004` | `argumentMapping` 出现未知字段 | true | - |

### 8.4 flag 模式校验

允许字段：

- `type`
- `templates`

校验规则：

| 错误码 | 规则 | 覆盖用例 | 未覆盖原因 |
|------|------|------|------|
| `MAP101` | `templates` 必须存在 | true | - |
| `MAP102` | `templates` 必须是 object | true | - |
| `MAP103` | `templates` 不允许为空对象 | true | - |
| `MAP104` | `templates` 中每个 key 必须存在于 `input.properties` | true | - |
| `MAP105` | template value 必须是 string 或 object | true | - |
| `MAP106` | template value 为 object 时，只允许 `if_true`、`if_false` | true | - |
| `MAP107` | template object 必须同时包含 `if_true` 和 `if_false` | true | - |
| `MAP108` | 非 boolean 字段不允许使用 `if_true` / `if_false` | true | - |
| `MAP109` | object 字段不允许使用 flag 模式 | true | - |
| `MAP110` | array 字段使用 flag 模式时，template value 必须是 string | true | - |
| `MAP111` | template key 不允许为空 | true | - |
| `MAP112` | `flag` 模式下的字符串模板必须包含 `{value}`，boolean 也不例外 | true | - |

说明：`MAP107` 要求 boolean 对象模板必须显式给出真值和假值两条分支。

### 8.5 positional 模式校验

允许字段：

- `type`
- `order`

校验规则：

| 错误码 | 规则 | 覆盖用例 | 未覆盖原因 |
|------|------|------|------|
| `MAP201` | `order` 必须存在 | true | - |
| `MAP202` | `order` 必须是非空 string 数组 | true | - |
| `MAP203` | `order` 中字段不允许重复 | true | - |
| `MAP204` | `order` 中每个字段必须存在于 `input.properties` | true | - |
| `MAP205` | `order` 中字段类型不允许为 `object`、`array` | true | - |
| `MAP207` | positional 模式下只允许`type`和`order`字段 | true | - |

说明：

- `string` 直接作为位置参数追加
- `number`、`integer`、`boolean` 必须先转换为字符串再追加
- `boolean` 的字符串化结果必须为 `"true"` 或 `"false"`

### 8.6 flattened 模式校验

允许字段：

- `type`
- `separator`
- `templates`

校验规则：

| 错误码 | 规则 | 覆盖用例 | 未覆盖原因 |
|------|------|------|------|
| `MAP301` | `templates` 必须存在 | true | - |
| `MAP302` | `templates` 必须是 object | true | - |
| `MAP303` | `templates` 不允许为空对象 | true | - |
| `MAP304` | `separator` 未设置时默认值为 `.`；若存在，只允许 `.`, `_`, `-` | true | - |
| `MAP305` | `templates` 的每个 key 必须是合法路径 | true | - |
| `MAP306` | 每个路径必须能在 `input.properties` 中解析到真实字段 | true | - |
| `MAP307` | 路径中间节点必须都是 `object` | true | - |
| `MAP308` | 路径叶子节点不能是未声明字段 | true | - |
| `MAP309` | 路径叶子节点若为 `object`，报错 | true | - |
| `MAP310` | 路径叶子节点若为 `array`，报错 | true | - |
| `MAP311` | boolean 叶子节点允许使用 `if_true` / `if_false` 对象，或包含 `{value}` 的字符串模板 | true | - |
| `MAP312` | 路径分隔写法必须与 `separator` 保持一致 | true | - |
| `MAP313` | `flattened` 模式下只允许出现 `type`、`separator` 和 `templates` 字段 | true | - |

说明：

- `separator` 只用于字段路径解析；未设置时默认使用 `.`
- 模板中的参数名写法由配置显式指定，不从字段路径自动推导

### 8.7 json-string 模式校验

允许字段：

- `type`
- `templates`

校验规则：

| 错误码 | 规则 | 覆盖用例 | 未覆盖原因 |
|------|------|------|------|
| `MAP401` | `templates` 必须存在 | true | - |
| `MAP402` | `templates` 必须是 object | true | - |
| `MAP403` | `templates` 不允许为空对象 | true | - |
| `MAP404` | `templates` 中每个 key 必须存在于 `input.properties` | true | - |
| `MAP405` | template value 必须是 string 或 `if_true` / `if_false` 对象 | true | - |
| `MAP406` | `object` 或 `array` 字段允许使用字符串模板 | true | - |
| `MAP407` | 仅 boolean 字段允许使用 `if_true` / `if_false` 模板 | true | - |
| `MAP408` | 模板中不允许出现未定义占位符 | true | - |
| `MAP409` | boolean 字段使用普通字符串模板时，必须包含 `{value}` | true | - |

---

### 8.8 mixed 模式校验

允许字段：

- `type`
- `mappings`

`mappings.<param>` 允许字段：

- `mode`
- `template`
- `separator`
- `order`

校验规则：

| 错误码 | 规则 | 覆盖用例 | 未覆盖原因 |
|------|------|------|------|
| `MAP501` | `mappings` 必须存在 | true | - |
| `MAP502` | `mappings` 必须是非空 object | true | - |
| `MAP503` | `mappings.<param>.mode` 必填，且只允许 `flag`、`positional`、`flattened`、`json-string` | true | - |
| `MAP504` | `mappings.<param>` 只允许 `mode`、`template`、`separator`、`order` 字段 | true | - |
| `MAP505` | `flag` / `json-string` 条目必须定义 `template`；`template` 必须为 string 或 boolean 对象模板 | true | - |
| `MAP506` | `positional` 条目必须定义 `order`；`order` 必须为正整数；且不允许定义 `template`、`separator` | true | - |
| `MAP507` | `flattened` 条目必须定义非空 object 类型的 `template`；`separator` 若存在只允许 `.`, `_`, `-` | true | - |
| `MAP508` | `flattened.template` 中的路径 key 必须全部以所属 `mappings.<param>` 为根 | true | - |
| `MAP509` | mixed 中各条目引用的字段或路径叶子不允许重复消费 | true | - |
| `MAP510` | mixed 中所有 `positional.order` 不允许重复 | true | - |

说明：

- `mixed` 是组合器，不引入新的模板语言
- `flag`、`positional`、`flattened`、`json-string` 的既有类型规则继续有效，只是作用域变为 `mappings.<param>`
- `flattened` 在 mixed 中采用分组表达：

```json
{
  "mappings": {
    "server": {
      "mode": "flattened",
      "template": {
        "server.host": "--host={value}",
        "server.port": "--port={value}"
      },
      "separator": "."
    }
  }
}
```

- mixed 的输出顺序固定为：
  - 所有 `mode = positional` 的条目按 `order` 升序输出
  - 其余条目按 `mappings` 的书写顺序输出

---

## 9. input 与 argumentMapping 关联校验

| 错误码 | 规则 | 覆盖用例 | 未覆盖原因 |
|------|------|------|------|
| `REL001` | `input` 与 `argumentMapping` 中定义的参数集合必须一致；零参数时必须使用 `input.properties = {}` 与 `argumentMapping = {}` | true | - |
| `REL002` | `input.required` 中的字段必须被 `argumentMapping` 有效引用 | true | - |
| `REL003` | `argumentMapping` 中出现 `input` 未定义字段，报错 | true | - |
| `REL004` | `flattened` 引用路径的根节点必须存在于 `input.properties` | true | - |
| `REL005` | 映射类型与字段类型明显不匹配时报错 | true | - |

---

## 10. events / eventSchemas / output 校验

说明：

- `events` 只用于声明 `result` 和自定义事件
- `stdout`、`stderr`、`exit`、`error` 由框架内置支持，不要求出现在 `events` 中；若出现在 `events` 中，也不要求在 `eventSchemas` 中定义
- `eventSchemas` 只用于描述自定义事件的 `data`，不用于描述 `result`
- `eventSchemas` 中不允许定义 `stdout`、`stderr`、`exit`、`error` 这 `4` 个预设事件
- `events` 中声明的每个自定义事件都必须在 `eventSchemas` 中定义
- `eventSchemas` 中声明的每个自定义事件都必须出现在 `events` 中

| 错误码 | 规则 | 覆盖用例 | 未覆盖原因 |
|------|------|------|------|
| `EVT001` | `events` 若存在，必须是 string 数组 | true | - |
| `EVT002` | `eventSchemas` 若存在，必须是 object | true | - |
| `EVT003` | `eventSchemas` 中的 key 必须全部出现在 `events` 中 | true | - |
| `EVT004` | `events` 包含 `result` 时，必须存在 `output` | true | - |
| `EVT005` | `eventSchemas` 中每个 value 必须是合法 Schema object | true | - |
| `EVT006` | `stdout`、`stderr`、`exit`、`error`、`result` 不应在 `eventSchemas` 中声明；其中前 `4` 个为预设事件 | true | - |
| `EVT007` | 自定义事件名称必须匹配 `[A-Za-z][A-Za-z0-9_.-]*`；`events` 与 `eventSchemas` 中的非法自定义事件名统一返回该错误 | true | - |
| `EVT008` | `events` 中声明的每个自定义事件都必须在 `eventSchemas` 中定义 | true | - |
| `EVT009` | `output` 若存在，`events` 中必须包含 `result` | true | - |

说明：`EVT009` 用于把 `output` 与 `result` 事件关系收紧成双向约束。

---

## 11. 本版新增与收紧规则

以下规则为本版相对上一版规则的新增或收紧项：

| 规则 | 本版要求 | 说明 |
|------|------|------|
| `SCH507` | `array.items.type` 只允许简单类型：`boolean`、`integer`、`number`、`string` | 新增限制 |
| `MAP107` | boolean 对象模板必须同时包含 `if_true` 和 `if_false` | 相比“至少一个分支”更严格 |
| `EVT009` | 只要定义了 `output`，`events` 中就必须显式声明 `result` | 与仅校验 `result -> output` 的单向约束不同 |
| `SCH004` | 未支持的 Schema keyword 一律视为非法 | 保留严格策略 |

说明：

- 本文档中的规则已作为当前发布版本口径。
- 文档、validator 和测试用例必须同步维护，避免规则定义与实现再次分叉。

---

## 12. 校验器工作说明

### 12.1 输入与输出

- 校验器输入为单个 `config.json` 文件
- 输出为校验结果集合，每条结果至少包含：
  - 错误码
  - JSON Pointer 路径
  - 可读错误信息
- 校验结果支持文本模式和 JSON 模式两种渲染方式

### 12.2 校验顺序

校验器按以下顺序执行静态校验：

1. 文件与 JSON 基础校验
2. 顶层公共字段校验
3. 工具格式判定校验
4. 子命令定义校验
5. JSON Schema 子集校验
6. `input` / `output` 根节点校验
7. `argumentMapping` 校验
8. `input` 与 `argumentMapping` 关联校验
9. `events` / `eventSchemas` / `output` 校验

### 12.3 工作方式

- 校验器只负责静态配置校验，不负责命令执行或运行时行为验证
- 校验器采用“收集错误”模式，同一次校验可返回多个问题
- 对于顶层和子命令中的扩展字段，校验器默认放行，但不做语义解析
- 对于未支持的 Schema keyword、非法映射模式和事件声明冲突，校验器直接报错

### 12.4 维护要求

- 新增规则时，文档、build 侧 validator、`tools/validator` 测试和样例必须同步更新
- 新增错误码时，必须补充对应的 coverage 用例
- 调整现有规则口径时，必须同步修正错误文案和示例配置

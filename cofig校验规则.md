# config.json 校验规则

## 1. 适用范围

本文档定义 `config.json` 的严格校验规则。校验器按严格模式工作：

- 不区分默认模式和 strict 模式
- 规则中列出的项默认都作为错误处理
- 发现错误即认为配置非法

校验范围包括：

1. 文件与 JSON 基础校验
2. 顶层公共字段校验
3. 工具格式判定校验
4. 子命令定义校验
5. JSON Schema 子集校验
6. `inputSchema` / `outputSchema` 根节点校验
7. `argumentMapping` 校验
8. `inputSchema` 与 `argumentMapping` 关联校验
9. `eventTypes` / `eventSchemas` 校验

---

## 2. 文件与 JSON 基础校验

| 错误码 | 规则 |
|------|------|
| `CFG001` | 文件不存在或不可读 |
| `CFG002` | 文件内容不是合法 JSON |
| `CFG003` | 根节点不是 JSON object |
| `CFG004` | 根节点为空对象 |
| `CFG005` | 文件编码不是 UTF-8 |
| `CFG006` | JSON 中存在重复 key |

---

## 3. 顶层公共字段校验

### 3.1 顶层允许字段

顶层只允许出现以下字段：

- `name`
- `version`
- `description`
- `executablePath`
- `permissions`
- `timeout`
- `eventTypes`
- `eventSchemas`
- `hasSubcommands`
- `inputSchema`
- `outputSchema`
- `argumentMapping`
- `subcommands`

### 3.2 校验规则

| 错误码 | 规则 |
|------|------|
| `TOP001` | `name` 必填，必须是非空 string |
| `TOP002` | `version` 必填，必须是非空 string |
| `TOP003` | `description` 必填，必须是非空 string |
| `TOP004` | `executablePath` 必填，必须是非空 string |
| `TOP005` | `executablePath` 必须是绝对路径 |
| `TOP006` | `permissions` 若存在，必须是 string 数组 |
| `TOP007` | `permissions` 不允许重复值 |
| `TOP008` | `timeout` 若存在，必须是 integer |
| `TOP009` | `timeout` 若存在，必须大于等于 `0` |
| `TOP010` | `eventTypes` 若存在，必须是 string 数组 |
| `TOP011` | `eventTypes` 不允许重复值 |
| `TOP012` | `eventSchemas` 若存在，必须是 object |
| `TOP013` | 存在未知顶层字段 |

---

## 4. 工具格式判定校验

当前只支持两种合法格式：

- 无子命令工具
- 有子命令工具

### 4.1 无子命令工具

判定条件：

- `hasSubcommands` 不存在，或值为 `false`

校验规则：

| 错误码 | 规则 |
|------|------|
| `FMT001` | 顶层必须存在 `inputSchema` |
| `FMT002` | 顶层必须存在 `argumentMapping` |
| `FMT003` | 顶层不允许存在 `subcommands` |
| `FMT004` | 顶层 `outputSchema` 若存在，必须是合法 Schema |
| `FMT005` | 顶层 `hasSubcommands` 若存在，必须是 boolean |

### 4.2 有子命令工具

判定条件：

- `hasSubcommands` 必须为 `true`

校验规则：

| 错误码 | 规则 |
|------|------|
| `FMT101` | 顶层必须存在 `subcommands` |
| `FMT102` | `subcommands` 必须是 object |
| `FMT103` | `subcommands` 不允许为空对象 |
| `FMT104` | 顶层 `inputSchema` 若存在，必须是合法 Schema，且其根节点必须为 `object` |
| `FMT105` | 顶层 `argumentMapping` 若存在，必须是合法 `argumentMapping` 对象 |
| `FMT106` | 顶层 `outputSchema` 若存在，必须是合法 Schema，且其根节点必须为 `object` |
| `FMT107` | 每个子命令名必须是非空字符串 |
| `FMT108` | 子命令名不能重复 |

---

## 5. 子命令定义校验

### 5.1 子命令允许字段

`subcommands.<name>` 只允许出现以下字段：

- `description`
- `permissions`
- `timeout`
- `inputSchema`
- `outputSchema`
- `eventTypes`
- `eventSchemas`
- `argumentMapping`

### 5.2 校验规则

| 错误码 | 规则 |
|------|------|
| `SUB001` | 子命令节点必须是 object |
| `SUB002` | 子命令必须存在 `description` |
| `SUB003` | 子命令 `description` 必须是非空 string |
| `SUB004` | 子命令必须存在 `inputSchema` |
| `SUB005` | 子命令必须存在 `argumentMapping` |
| `SUB006` | 子命令 `outputSchema` 若存在，必须是合法 Schema |
| `SUB007` | 子命令存在未知字段 |
| `SUB008` | 子命令 `eventTypes` 若存在，必须是 string 数组 |
| `SUB009` | 子命令 `eventSchemas` 若存在，必须是 object |
| `SUB010` | 子命令 `permissions` 若存在，必须是 string 数组 |
| `SUB011` | 子命令 `timeout` 若存在，必须是 integer 且大于等于 `0` |

### 5.3 字段回退规则

| 规则 | 说明 |
|------|------|
| `SUB101` | 子命令 `inputSchema` 和 `argumentMapping` 必填，且不允许回退到顶层 |
| `SUB102` | 子命令 `permissions`、`timeout`、`outputSchema`、`eventTypes`、`eventSchemas` 若未定义，必须回退到顶层同名字段 |

---

## 6. JSON Schema 子集校验

校验器不支持完整 JSON Schema，只支持当前文档规定的受限子集。

### 6.1 通用规则

| 错误码 | 规则 |
|------|------|
| `SCH001` | Schema 节点必须是 object |
| `SCH002` | Schema 节点必须存在 `type` |
| `SCH003` | `type` 只允许 `string`、`number`、`integer`、`boolean`、`array`、`object` |
| `SCH004` | 出现未支持的 Schema keyword |
| `SCH005` | `description` 若存在，必须是 string |
| `SCH006` | `default` 若存在，类型必须与 `type` 一致 |

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

| 错误码 | 规则 |
|------|------|
| `SCH101` | `minLength` 若存在，必须是 integer 且大于等于 `0` |
| `SCH102` | `maxLength` 若存在，必须是 integer 且大于等于 `0` |
| `SCH103` | `minLength` 不得大于 `maxLength` |
| `SCH104` | `pattern` 若存在，必须是合法正则字符串 |
| `SCH105` | `format` 若存在，必须是以下支持值之一：`ipv4`、`ipv6`、`hostname`、`email`、`uri`、`date`、`date-time` |

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

| 错误码 | 规则 |
|------|------|
| `SCH201` | `minimum` / `maximum` 若存在，必须是 number |
| `SCH202` | `exclusiveMinimum` / `exclusiveMaximum` 若存在，必须是 number |
| `SCH203` | `minimum` 不得大于 `maximum` |
| `SCH204` | `exclusiveMinimum` 不得大于等于 `exclusiveMaximum` |
| `SCH205` | `multipleOf` 若存在，必须是正数 |

### 6.4 integer 类型

允许字段与 `number` 相同，但值必须满足整数语义。

校验规则：

| 错误码 | 规则 |
|------|------|
| `SCH301` | `minimum` / `maximum` / `exclusiveMinimum` / `exclusiveMaximum` / `multipleOf` 若存在，必须是整数 |
| `SCH302` | `multipleOf` 必须大于 `0` |
| `SCH303` | `minimum` 不得大于 `maximum` |

### 6.5 boolean 类型

允许字段：

- `type`
- `description`
- `default`

校验规则：

| 错误码 | 规则 |
|------|------|
| `SCH401` | boolean 类型不允许出现不适用约束字段，如 `minLength`、`minimum` |
| `SCH402` | `default` 若存在，必须是 boolean |

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

| 错误码 | 规则 |
|------|------|
| `SCH501` | `items` 必须存在 |
| `SCH502` | `items` 必须是合法 Schema object |
| `SCH503` | `minItems` / `maxItems` 若存在，必须是 integer 且大于等于 `0` |
| `SCH504` | `minItems` 不得大于 `maxItems` |
| `SCH505` | `uniqueItems` 若存在，必须是 boolean |
| `SCH506` | `default` 若存在，必须是 array |

### 6.7 object 类型

允许字段：

- `type`
- `description`
- `properties`
- `required`
- `additionalProperties`
- `minProperties`
- `maxProperties`

校验规则：

| 错误码 | 规则 |
|------|------|
| `SCH601` | `properties` 若存在，必须是 object |
| `SCH602` | `properties` 中每个 value 都必须是合法 Schema object |
| `SCH603` | `required` 若存在，必须是 string 数组 |
| `SCH604` | `required` 中每个字段都必须出现在 `properties` |
| `SCH605` | `required` 不允许重复 |
| `SCH606` | `additionalProperties` 若存在，必须是 boolean |
| `SCH607` | `minProperties` / `maxProperties` 若存在，必须是 integer 且大于等于 `0` |
| `SCH608` | `minProperties` 不得大于 `maxProperties` |

---

## 7. inputSchema / outputSchema 根节点约束

| 错误码 | 规则 |
|------|------|
| `SCX001` | 顶层 `inputSchema.type` 必须是 `object` |
| `SCX002` | 顶层 `inputSchema.properties` 必须存在 |
| `SCX003` | 顶层 `outputSchema` 若存在，其根节点必须是 `object` |
| `SCX004` | 子命令 `inputSchema.type` 必须是 `object` |
| `SCX005` | 子命令 `inputSchema.properties` 必须存在 |

---

## 8. argumentMapping 校验

### 8.1 支持的映射类型

当前只支持以下四种：

- `flag`
- `positional`
- `flattened`
- `json-string`

### 8.2 通用规则

| 错误码 | 规则 |
|------|------|
| `MAP001` | `argumentMapping` 必须是 object |
| `MAP002` | `argumentMapping.type` 必填 |
| `MAP003` | `argumentMapping.type` 只允许 `flag`、`positional`、`flattened`、`json-string` |
| `MAP004` | `argumentMapping` 出现未知字段 |
| `MAP005` | `argumentMapping` 必须与对应的 `inputSchema` 联合校验 |

### 8.3 flag 模式校验

#### 8.3.1 允许字段

- `type`
- `templates`

#### 8.3.2 校验规则

| 错误码 | 规则 |
|------|------|
| `MAP101` | `templates` 必须存在 |
| `MAP102` | `templates` 必须是 object |
| `MAP103` | `templates` 不允许为空对象 |
| `MAP104` | `templates` 中每个 key 必须存在于 `inputSchema.properties` |
| `MAP105` | template value 必须是 string 或 object |
| `MAP106` | template value 为 object 时，只允许 `if_true`、`if_false` |
| `MAP107` | template object 至少包含 `if_true` 或 `if_false` 之一 |
| `MAP108` | 非 boolean 字段不允许使用 `if_true` / `if_false` |
| `MAP109` | object 字段不允许使用 flag 模式 |
| `MAP110` | array 字段使用 flag 模式时，template value 必须是 string |
| `MAP111` | template key 不允许为空 |
| `MAP112` | 非 boolean 字段的字符串模板必须包含 `{value}` |

### 8.4 positional 模式校验

#### 8.4.1 允许字段

- `type`
- `order`

#### 8.4.2 校验规则

| 错误码 | 规则 |
|------|------|
| `MAP201` | `order` 必须存在 |
| `MAP202` | `order` 必须是非空 string 数组 |
| `MAP203` | `order` 中字段不允许重复 |
| `MAP204` | `order` 中每个字段必须存在于 `inputSchema.properties` |
| `MAP205` | `order` 中字段类型若为 object，报错 |
| `MAP206` | `order` 中字段类型若为 array，报错 |
| `MAP207` | positional 模式下不允许出现 `templates` |
| `MAP208` | positional 模式下不允许出现 `separator` |

说明：

- `string` 直接作为位置参数追加
- `number`、`integer`、`boolean` 必须先转换为字符串再追加
- `boolean` 的字符串化结果必须为 `"true"` 或 `"false"`
- `array`、`object` 禁止用于 `positional`

### 8.5 flattened 模式校验

#### 8.5.1 允许字段

- `type`
- `separator`
- `templates`

#### 8.5.2 校验规则

| 错误码 | 规则 |
|------|------|
| `MAP301` | `templates` 必须存在 |
| `MAP302` | `templates` 必须是 object |
| `MAP303` | `templates` 不允许为空对象 |
| `MAP304` | `separator` 若存在，只允许 `.`, `_`, `-` |
| `MAP305` | `templates` 的每个 key 必须是合法路径 |
| `MAP306` | 每个路径必须能在 `inputSchema` 中解析到真实字段 |
| `MAP307` | 路径中间节点必须都是 `object` |
| `MAP308` | 路径叶子节点不能是未声明字段 |
| `MAP309` | 路径叶子节点若为 object，报错 |
| `MAP310` | 路径叶子节点若为 array，报错 |
| `MAP311` | boolean 叶子节点必须使用 `if_true` / `if_false` 对象 |
| `MAP312` | 路径分隔写法必须与 `separator` 保持一致 |

说明：

- `separator` 只用于字段路径解析
- 模板中的参数名写法由配置显式指定，不从字段路径自动推导

### 8.6 json-string 模式校验

#### 8.6.1 允许字段

- `type`
- `templates`

#### 8.6.2 校验规则

| 错误码 | 规则 |
|------|------|
| `MAP401` | `templates` 必须存在 |
| `MAP402` | `templates` 必须是 object |
| `MAP403` | `templates` 不允许为空对象 |
| `MAP404` | `templates` 中每个 key 必须存在于 `inputSchema.properties` |
| `MAP405` | template value 必须是 string 或 `if_true` / `if_false` 对象 |
| `MAP406` | object 或 array 字段允许使用字符串模板 |
| `MAP407` | boolean 字段允许使用 `if_true` / `if_false` |
| `MAP408` | 模板中若出现未定义占位符，报错 |
| `MAP409` | boolean 字段使用普通字符串模板时，必须包含 `{value}`，否则报错 |
| `MAP410` | object / array 字段不允许使用 `if_true` / `if_false` |

说明：

- 当前只允许 `{value}` 作为模板占位符
- 对于 object / array，实际映射时先序列化为 JSON 字符串，再替换 `{value}`

---

## 9. inputSchema 与 argumentMapping 关联校验

| 错误码 | 规则 |
|------|------|
| `REL001` | `argumentMapping` 引用的字段必须全部出现在 `inputSchema.properties` |
| `REL002` | `inputSchema.required` 中的字段必须被 `argumentMapping` 有效引用 |
| `REL003` | `argumentMapping` 中出现 `inputSchema` 未定义字段，报错 |
| `REL004` | `flattened` 引用路径的根节点必须存在于 `inputSchema.properties` |
| `REL005` | 映射类型与字段类型明显不匹配时报错 |
| `REL006` | `inputSchema` 中没有任何可映射字段却定义了 `argumentMapping`，报错 |

---

## 10. eventTypes / eventSchemas 校验

说明：

- `eventTypes` 只用于声明 `result` 和自定义事件
- `stdout`、`stderr`、`exit`、`error` 由框架内置支持，不要求出现在 `eventTypes` 中
- `result` 若使用，必须显式出现在 `eventTypes` 中
- `eventSchemas` 只用于描述自定义事件的 `data`，不用于描述 `result`
- 自定义事件名称只允许字母和数字，且首字符必须是字母，推荐正则为 `[A-Za-z][A-Za-z0-9]*`
- 工具主动输出的事件 JSON 中，`event` 不得为 `error` 或 `exit`

| 错误码 | 规则 |
|------|------|
| `EVT001` | `eventTypes` 若存在，必须是字符串数组 |
| `EVT002` | `eventSchemas` 若存在，必须是 object |
| `EVT003` | `eventSchemas` 中的 key 必须全部出现在 `eventTypes` 中 |
| `EVT004` | `eventTypes` 包含 `result` 时，必须存在 `outputSchema` |
| `EVT005` | `eventSchemas` 中每个 value 必须是合法 Schema object |
| `EVT006` | `stdout`、`stderr`、`exit`、`error`、`result` 不应在 `eventSchemas` 中声明 |
| `EVT007` | 自定义事件名称必须匹配 `[A-Za-z][A-Za-z0-9]*` |

---

## 11. 两种格式的最终判定矩阵

### 11.1 无子命令工具必须满足

- 有 `inputSchema`
- 有 `argumentMapping`
- 没有 `subcommands`
- `hasSubcommands` 缺省或为 `false`

### 11.2 有子命令工具必须满足

- `hasSubcommands = true`
- 有 `subcommands`
- 顶层可定义默认的 `inputSchema`
- 顶层可定义默认的 `argumentMapping`
- 顶层可定义默认的 `outputSchema`
- 每个子命令都独立定义 `inputSchema` 和 `argumentMapping`

---

## 12. 最小实现优先级

若需要先实现一版最小可用校验器，建议优先落以下规则：

- `CFG002`
- `CFG003`
- `TOP001`
- `TOP004`
- `FMT001`
- `FMT101`
- `SCX001`
- `SCH003`
- `MAP002`
- `MAP003`
- `MAP104`
- `MAP204`
- `REL001`
- `EVT003`

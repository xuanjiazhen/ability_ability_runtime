| 用例名 | 预期结果 | 描述 |
|------|------|------|
| `valid_argument_mapping_flag_001.json` | `pass` | 0 参数场景下，`argumentMapping` 可以为空对象。 |
| `valid_argument_mapping_flag_002.json` | `failed` | `flag` 模式缺少 `templates`，主错误为 `MAP101`。 |
| `valid_argument_mapping_flag_003.json` | `failed` | `templates` 不能为空，主错误为 `MAP103`。 |
| `valid_argument_mapping_flag_004.json` | `failed` | `templates` 中出现未在 `input.properties` 中定义的字段，主错误为 `MAP104`。 |
| `valid_argument_mapping_flag_005.json` | `pass` | boolean 对象模板同时包含 `if_true` 和 `if_false`。 |
| `valid_argument_mapping_flag_006.json` | `failed` | boolean 对象模板缺少 `if_false`，主错误为 `MAP107`。 |
| `valid_argument_mapping_flag_007.json` | `failed` | boolean 对象模板同时缺少 `if_true` 和 `if_false`，主错误为 `MAP107`。 |
| `valid_argument_mapping_flag_008.json` | `failed` | 非 boolean 字段使用 `if_true` / `if_false` 对象模板，主错误为 `MAP108`。 |
| `valid_argument_mapping_flag_009.json` | `failed` | object 字段使用 `flag` 模式字符串模板，主错误为 `MAP109`。 |
| `valid_argument_mapping_flag_PASS.json` | `pass` | boolean 字段使用包含 `{value}` 的字符串模板。 |
| `valid_argument_mapping_position_001.json` | `pass` | 0 参数场景下，`argumentMapping` 可以为空对象。 |
| `valid_argument_mapping_position_002.json` | `failed` | `positional` 模式缺少 `order`，主错误为 `MAP201`。 |
| `valid_argument_mapping_position_003.json` | `failed` | `order` 不能为空数组，主错误为 `MAP202`。 |
| `valid_argument_mapping_position_004.json` | `failed` | `order` 中字段重复，主错误为 `MAP203`。 |
| `valid_argument_mapping_position_005.json` | `failed` | `order` 中出现未在 `input.properties` 中定义的字段，主错误为 `MAP204`。 |
| `valid_argument_mapping_position_006.json` | `failed` | object 字段不能用于 `positional` 模式，主错误为 `MAP205`。 |
| `valid_argument_mapping_position_007.json` | `failed` | array 字段不能用于 `positional` 模式，主错误为 `MAP206`。 |
| `valid_argument_mapping_position_008.json` | `failed` | `positional` 模式只允许 `type` 和 `order` 字段，主错误为 `MAP207`。 |
| `valid_argument_mapping_position_PASS.json` | `pass` | `string`、`number`、`integer`、`boolean` 字段按 `order` 顺序作为位置参数传递。 |
| `valid_argument_mapping_flattened_001.json` | `pass` | 0 参数场景下，`argumentMapping` 可以为空对象。 |
| `valid_argument_mapping_flattened_002.json` | `failed` | `flattened` 模式缺少 `templates`，主错误为 `MAP301`。 |
| `valid_argument_mapping_flattened_003.json` | `failed` | `templates` 必须是 object，主错误为 `MAP302`。 |
| `valid_argument_mapping_flattened_004.json` | `failed` | `templates` 不能为空对象，主错误为 `MAP303`。 |
| `valid_argument_mapping_flattened_005.json` | `failed` | `separator` 只允许 `.`、`_`、`-`，主错误为 `MAP304`。 |
| `valid_argument_mapping_flattened_006.json` | `failed` | `flattened` 模板 key 必须是嵌套路径，主错误为 `MAP305`。 |
| `valid_argument_mapping_flattened_007.json` | `failed` | `flattened` 路径未在 `input.properties` 中定义，主错误为 `MAP306`。 |
| `valid_argument_mapping_flattened_008.json` | `failed` | `flattened` 路径的中间节点必须为 object，主错误为 `MAP307`。 |
| `valid_argument_mapping_flattened_009.json` | `failed` | `flattened` 路径叶子字段必须已声明，主错误为 `MAP308`。 |
| `valid_argument_mapping_flattened_010.json` | `failed` | `flattened` 叶子字段不能为 object，主错误为 `MAP309`。 |
| `valid_argument_mapping_flattened_011.json` | `failed` | `flattened` 叶子字段不能为 array，主错误为 `MAP310`。 |
| `valid_argument_mapping_flattened_012.json` | `failed` | boolean 叶子字段使用字符串模板时必须包含 `{value}`，主错误为 `MAP311`。 |
| `valid_argument_mapping_flattened_013.json` | `failed` | `flattened` 路径必须与配置的 `separator` 保持一致，主错误为 `MAP312`。 |
| `valid_argument_mapping_flattened_014.json` | `failed` | `flattened` 模式只允许 `type`、`separator` 和 `templates` 字段，主错误为 `MAP313`。 |
| `valid_argument_mapping_flattened_015.json` | `pass` | 显式设置 `separator` 为 `.` 时，嵌套路径可按 `flattened` 模式正常映射。 |
| `valid_argument_mapping_flattened_016.json` | `pass` | 显式设置 `separator` 为 `_` 时，嵌套路径可按 `flattened` 模式正常映射。 |
| `valid_argument_mapping_flattened_017.json` | `pass` | 显式设置 `separator` 为 `-` 时，嵌套路径可按 `flattened` 模式正常映射。 |
| `valid_argument_mapping_flattened_018.json` | `pass` | 3 层以上 object 路径在 `separator = .` 时可按 `flattened` 模式正常映射。 |
| `valid_argument_mapping_flattened_019.json` | `pass` | 3 层以上 object 路径在 `separator = _` 时可按 `flattened` 模式正常映射，深层 boolean 叶子支持对象模板。 |
| `valid_argument_mapping_flattened_020.json` | `failed` | 深层 `flattened` 路径的中间节点必须为 object，主错误为 `MAP307`。 |
| `valid_argument_mapping_flattened_021.json` | `failed` | 深层 `flattened` 路径叶子字段必须已声明，主错误为 `MAP308`。 |
| `valid_argument_mapping_flattened_022.json` | `failed` | 深层 `flattened` 叶子字段不能为 array，主错误为 `MAP310`。 |
| `valid_argument_mapping_flattened_023.json` | `failed` | 深层 `flattened` 路径未在`input.properties`中 |
| `valid_argument_mapping_flattened_024.json` | `failed` | 深层 `flattened` 可正常解析 |
| `valid_argument_mapping_flattened_PASS.json` | `pass` | 嵌套 `string`、`integer` 和 `boolean` 叶子字段可按 `flattened` 模式映射，boolean 支持对象模板。 |
| `valid_argument_mapping_json_string_001.json` | `pass` | 0 参数场景下，`argumentMapping` 可以为空对象。 |
| `valid_argument_mapping_json_string_002.json` | `failed` | `json-string` 模式缺少 `templates`，主错误为 `MAP401`。 |
| `valid_argument_mapping_json_string_003.json` | `failed` | `templates` 必须是 object，主错误为 `MAP402`。 |
| `valid_argument_mapping_json_string_004.json` | `failed` | `templates` 不能为空对象，主错误为 `MAP403`。 |
| `valid_argument_mapping_json_string_005.json` | `failed` | `templates` 中出现未在 `input.properties` 中定义的字段，主错误为 `MAP404`。 |
| `valid_argument_mapping_json_string_006.json` | `failed` | `json-string` 模板值必须为 string 或 object，主错误为 `MAP405`。 |
| `valid_argument_mapping_json_string_007.json` | `failed` | 非 boolean 字段不允许使用 `if_true` / `if_false` 对象模板，主错误为 `MAP407`，同时伴随 `MAP410`。 |
| `valid_argument_mapping_json_string_008.json` | `failed` | 非 boolean 字段字符串模板必须包含 `{value}`，主错误为 `MAP408`。 |
| `valid_argument_mapping_json_string_009.json` | `failed` | boolean 字段使用字符串模板时必须包含 `{value}`，主错误为 `MAP409`。 |
| `valid_argument_mapping_json_string_010.json` | `pass` | boolean 字段可使用包含 `{value}` 的字符串模板。 |
| `valid_argument_mapping_json_string_011.json` | `failed` | boolean 字段使用字符串模板时必须包含 `{value}`，主错误为 `MAP409`。 |
| `valid_argument_mapping_json_string_012.json` | `pass` | 层次较深的 object 字段可作为整体参数通过 `json-string` 模式传递。 |
| `valid_argument_mapping_json_string_013.json` | `pass` | 层次较深的 object 字段可与普通标量字段一起通过 `json-string` 模式传递。 |
| `valid_argument_mapping_json_string_014.json` | `pass` | 包含更深层 object 和嵌套 array 的 object 字段，可作为整体参数通过 `json-string` 模式传递。 |
| `valid_argument_mapping_json_string_PASS.json` | `pass` | `string`、`object`、`array` 和 `boolean` 字段可按 `json-string` 模式映射，boolean 支持对象模板。 |
| `valid_argument_mapping_mixed_001.json` | `pass` | 最小 mixed 正例：仅包含一个 `positional` 条目。 |
| `valid_argument_mapping_mixed_002.json` | `failed` | mixed 模式缺少 `mappings`，主错误为 `MAP501`。 |
| `valid_argument_mapping_mixed_003.json` | `failed` | `mappings` 必须是非空 object，主错误为 `MAP502`。 |
| `valid_argument_mapping_mixed_004.json` | `pass` | mixed 中单个 `flag` 条目配置正确时可通过。 |
| `valid_argument_mapping_mixed_005.json` | `failed` | `mappings.<param>.mode` 非法，主错误为 `MAP503`。 |
| `valid_argument_mapping_mixed_006.json` | `pass` | mixed 中多个 `positional` 条目按不同 `order` 合法配置时可通过。 |
| `valid_argument_mapping_mixed_007.json` | `failed` | mixed `positional.order` 必须为正整数，主错误为 `MAP506`。 |
| `valid_argument_mapping_mixed_008.json` | `pass` | mixed 中 `flattened` 条目使用 object 模板和合法 `separator` 时可通过。 |
| `valid_argument_mapping_mixed_009.json` | `failed` | mixed `flattened.template` 必须为非空 object，主错误为 `MAP507`。 |
| `valid_argument_mapping_mixed_010.json` | `failed` | mixed `flattened.template` 中的路径 key 必须以所属参数为根，主错误为 `MAP508`。 |
| `valid_argument_mapping_mixed_011.json` | `pass` | mixed 中单个 `json-string` 条目配置正确时可通过。 |
| `valid_argument_mapping_mixed_012.json` | `failed` | mixed 的 `flag` / `json-string` 条目缺少 `template`，主错误为 `MAP505`。 |
| `valid_argument_mapping_mixed_013.json` | `failed` | mixed 条目出现未定义字段，主错误为 `MAP504`。 |
| `valid_argument_mapping_mixed_014.json` | `pass` | 复杂 mixed 正例：同时包含 `positional`、`flag`、`flattened`、`json-string`。 |
| `valid_argument_mapping_mixed_015.json` | `failed` | mixed 中字段或路径叶子重复消费，主错误为 `MAP509`。 |
| `valid_argument_mapping_mixed_016.json` | `failed` | mixed 中多个 `positional.order` 不允许重复，主错误为 `MAP510`。 |
| `valid_argument_mapping_mixed_017.json` | `pass` | mixed 综合正例：覆盖 `string`、`integer`、`number`、`boolean`、`object` 在合法 mode 下的组合映射。 |
| `event_base_pass.json` | `pass` | 最小基线配置，不声明 `events`、`eventSchemas`、`output`。 |
| `event_evt001_fail.json` | `failed` | `events` 若存在，必须是 string 数组，主错误为 `EVT001`。 |
| `event_evt001_pass.json` | `pass` | `events` 使用 string 数组并为自定义事件提供匹配的 `eventSchemas` 时可通过。 |
| `event_top011_fail.json` | `failed` | `events` 不允许重复值，主错误为 `TOP011`。 |
| `event_top011_pass.json` | `pass` | `events` 中的值不重复，且每个自定义事件都具备对应 schema 时可通过。 |
| `event_evt002_fail.json` | `failed` | `eventSchemas` 若存在，必须是 object，主错误为 `EVT002`。 |
| `event_evt002_pass.json` | `pass` | `eventSchemas` 使用 object 且结构合法时可通过。 |
| `event_evt003_fail.json` | `failed` | `eventSchemas` 中的 key 必须全部出现在 `events` 中，主错误为 `EVT003`。 |
| `event_evt003_pass.json` | `pass` | `eventSchemas` 中声明的 key 全部出现在 `events` 中时可通过。 |
| `event_evt005_fail.json` | `failed` | `eventSchemas` 中每个 value 必须是合法 Schema object，主错误为 `EVT005`。 |
| `event_evt005_pass.json` | `pass` | `eventSchemas` 中每个 value 为合法 Schema object 时可通过。 |
| `event_evt006_fail.json` | `failed` | 预设事件不应在 `eventSchemas` 中声明，主错误为 `EVT006`。 |
| `event_evt006_pass.json` | `pass` | 预设事件仅出现在 `events` 中、未出现在 `eventSchemas` 中时可通过。 |
| `event_evt007_events_fail.json` | `failed` | 非法自定义事件名统一返回 `EVT007`。 |
| `event_evt007_events_pass.json` | `pass` | `events` 中使用合法自定义事件名并提供匹配 schema 时可通过。 |
| `event_evt007_schemas_fail.json` | `failed` | `eventSchemas` 中的非法自定义事件名统一返回 `EVT007`。 |
| `event_evt007_schemas_pass.json` | `pass` | `eventSchemas` 中使用合法自定义事件名并与 `events` 保持一致时可通过。 |
| `event_evt008_fail.json` | `failed` | `events` 中声明的自定义事件必须在 `eventSchemas` 中定义，主错误为 `EVT008`。 |
| `event_evt008_pass.json` | `pass` | `events` 中声明的自定义事件在 `eventSchemas` 中定义后可通过。 |
| `event_evt004_fail.json` | `failed` | `events` 包含 `result` 时，必须存在 `output`，主错误为 `EVT004`。 |
| `event_evt004_pass.json` | `pass` | `events` 包含 `result` 且定义了 `output` 时可通过。 |
| `event_evt009_fail.json` | `failed` | `output` 若存在，`events` 中必须包含 `result`，主错误为 `EVT009`。 |
| `event_evt009_pass.json` | `pass` | `output` 与 `result` 事件同时声明，并可与自定义事件共存。 |
| `event_builtin_events_pass.json` | `pass` | `stdout`、`stderr`、`exit`、`error` 可直接出现在 `events` 中，且不要求在 `eventSchemas` 中定义。 |
| `event_result_custom_pass.json` | `pass` | `result`、`output` 与自定义事件可同时合法声明。 |
| `event_result_builtin_pass.json` | `pass` | `result`、`output` 可与预设事件组合使用，且无需定义 `eventSchemas`。 |

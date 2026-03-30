# ArkTSScript.bin

`ArkTSScript.bin` 是一个轻量级 ArkTS 脚本执行器，可在空进程中加载并执行 ArkCompiler Bytecode（`.abc`）文件。

## 特性

- 基于 `JsRuntime` 执行 ArkTS 字节码
- 支持多种参数类型：`string`、`number`、`boolean`、`null`、`undefined`、`object`
- 支持从命令行调用脚本中挂到 `globalThis` 的函数
- 自动创建并绑定 `ScriptContext` 到 `globalThis.scriptContext`
- 提供常用目录、应用信息和上下文能力，便于脚本访问运行环境

## 用法

### 命令格式

```bash
arkTSScript.bin <path> -f funName [-a arg...]
```

`path` 必须是已经编译好的 `.abc` 文件路径。

### 参数说明

| 参数 | 必填 | 说明 |
|------|------|------|
| `path` | 是 | ABC 文件路径 |
| `-f, --function` | 是 | 要调用的全局函数名 |
| `-a, --arg` | 否 | 函数参数，可重复传入多个 |
| `-h, --help` | 否 | 输出帮助信息 |

说明：

- `-f/--function` 对应的函数会从 `globalThis` 上查找。
- `--help` 只打印帮助，不执行脚本。

### 参数类型

支持自动识别和显式类型两种写法：

| 类型 | 自动识别示例 | 显式写法示例 |
|------|--------------|--------------|
| `string` | `hello` | `str:hello`、`string:hello` |
| `number` | `123`、`3.14` | `num:123`、`number:3.14` |
| `boolean` | `true`、`false` | `bool:true`、`boolean:false` |
| `null` | `null` | `null:null` |
| `undefined` | `undefined` | `undefined:undef` |
| `object` | - | `json:{"key":"value"}` |

### 使用示例

#### 1. 调用默认入口 `main`

```bash
arkTSScript.bin /data/scripts/test.abc -f main
```

#### 2. 传入字符串参数

```bash
arkTSScript.bin /data/scripts/test.abc -f greet -a "Alice"
```

#### 3. 传入数字参数

```bash
arkTSScript.bin /data/scripts/test.abc -f add -a 10 -a 20
```

#### 4. 传入混合类型参数

```bash
arkTSScript.bin /data/scripts/test.abc -f configure -a str:production -a num:8080 -a bool:true
```

#### 5. 强制指定参数类型

```bash
# 把 "123" 当作字符串，而不是数字
arkTSScript.bin /data/scripts/test.abc -f process -a str:123

# 显式传入布尔值
arkTSScript.bin /data/scripts/test.abc -f toggle -a bool:true

# 显式传入 null
arkTSScript.bin /data/scripts/test.abc -f init -a null:null

# 显式传入 JSON 对象
arkTSScript.bin /data/scripts/test.abc -f update -a json:'{"name":"test","value":42}'
```

#### 6. 查看帮助

```bash
arkTSScript.bin --help
```

## 脚本开发

ABC 文件加载完成后，运行时会自动把上下文对象和目标函数所需的参数准备好。

### 1. `ScriptContext`

`ScriptContext` 会自动绑定到 `globalThis.scriptContext`。

> Note: `scriptContext` 继承自 `Context`，但它是面向脚本执行场景的精简版 `Context`。创建 `scriptContext` 时不需要包含 [Context.d.ts](/d:/ability_ability_runtime/arkTSScript/Context.d.ts) 中这些变量和接口：`resourceManager`、`eventHub`、`createModuleContext`、`createSystemHspModuleResourceManager`、`createModuleResourceManager`、`createDisplayContext`、`createAreaModeContext`。

> Note: 下面的 `ScriptContext` 示例严格按上面的裁剪规则给出，即在 `Context.d.ts` 的基础上，仅删除明确说明“不需要”的字段和接口。

```typescript
interface ScriptContext {
    applicationInfo: ApplicationInfo;
    cacheDir: string;
    tempDir: string;
    filesDir: string;
    databaseDir: string;
    preferencesDir: string;
    bundleCodeDir: string;
    distributedFilesDir: string;
    resourceDir: string;
    cloudFileDir: string;
    readonly logFileDir: string;
    area: contextConstant.AreaMode;
    processName: string;

    createBundleContext(bundleName: string): ScriptContext;
    getApplicationContext(): ApplicationContext;
}
```

使用示例：

```javascript
const context = scriptContext;
console.log("CacheDir: " + context.cacheDir);
```

### 2. 定义函数

要调用的函数需要能从 `globalThis` 上访问到：

```javascript
globalThis.myFunction = function(arg1, arg2) {
    return arg1 + arg2;
};

function main(...args) {
    console.log("Main called with: " + JSON.stringify(args));
    return "success";
}

globalThis.main = main;
```

说明：

- 目标函数会从 `globalThis` 上查找。
- 运行阶段会区分“函数不存在”“目标不是函数”“参数转换失败”“函数调用失败”等场景。

### 3. 多类型参数示例

```javascript
const context = scriptContext;

function main(...args) {
    console.log("Arguments count: " + args.length);
    args.forEach((arg, index) => {
        console.log("Arg[" + index + "]: " + JSON.stringify(arg) +
                    " (type: " + typeof arg + ")");
    });
    return args;
}

function add(a, b) {
    if (typeof a !== "number" || typeof b !== "number") {
        console.error("Arguments must be numbers");
        return null;
    }
    return a + b;
}

function configure(environment, port, debug) {
    console.log("Environment: " + environment + " (" + typeof environment + ")");
    console.log("Port: " + port + " (" + typeof port + ")");
    console.log("Debug: " + debug + " (" + typeof debug + ")");
    return {
        env: environment,
        port: port,
        debug: debug
    };
}

function process(data) {
    if (data === null) {
        return "received null";
    }
    if (data === undefined) {
        return "received undefined";
    }
    return "received: " + JSON.stringify(data);
}

function update(config) {
    if (typeof config === "object" && config !== null) {
        console.log("Config keys: " + Object.keys(config).join(", "));
        return config;
    }
    return null;
}

if (typeof globalThis !== "undefined") {
    globalThis.main = main;
    globalThis.add = add;
    globalThis.configure = configure;
    globalThis.process = process;
    globalThis.update = update;
}
```

## 构建与部署

### 1. 构建 `arkTSScript.bin`

```bash
cd arkTSScript/src
hb build arkTSScript
```

### 2. 把脚本编译成 ABC

```bash
# 使用 ArkCompiler 工具链
ark_js_vm --compile test_script.js -o test.abc

# 或使用 es2abc
es2abc test_script.js -o test.abc
```

### 3. 推送到设备

```bash
# 推送可执行文件
hdc shell mkdir -p /system/bin
hdc file send arkTSScript.bin /system/bin/
hdc shell chmod +x /system/bin/arkTSScript.bin

# 推送 ABC 文件
hdc shell mkdir -p /data/scripts
hdc file send test.abc /data/scripts/
```

## 实现概览

整体流程如下：

1. 解析命令行参数：使用 `getopt_long` 处理 `path`、`-f/--function`、`-a/--arg`、`-h/--help`。
2. 创建 `JsRuntime`：基于当前运行环境创建轻量运行时。
3. 创建 `ScriptContext`：参考 `MainThread` 构造 `ContextImpl`，并设置 `ApplicationInfo`。
4. 绑定 Context：把上下文对象绑定到 `globalThis.scriptContext`。
5. 加载 ABC：通过 `JsRuntime` 加载并执行目标 `.abc` 文件。
6. 调用目标函数：把 `ArgValue` 转成 `napi_value` 后调用 `globalThis` 上的目标函数。

相关参考：

- `JsRuntime`：`foundation/ability/ability_runtime/frameworks/native/runtime/js_runtime.cpp`
- `ContextImpl`：`foundation/ability/ability_runtime/frameworks/native/appkit/app/main_thread.cpp`
- ABC 加载链路：`foundation/ability/ability_runtime/js_environment/frameworks/js_environment/src/js_environment.cpp`

## 限制与注意事项

1. 只支持执行已经编译好的 ArkCompiler Bytecode（`.abc`），不直接执行源码。
2. 目标函数必须能从 `globalThis` 上访问到。
3. 当前运行时是轻量执行环境，不包含 ArkUI 页面能力。
4. `ScriptContext` 是面向脚本场景的裁剪版 `Context`，文档里的接口范围以本 README 为准。
5. `object` 类型参数通过 `json:` 前缀传入，内容需要是合法 JSON。

## 参数自动推断规则

自动推断顺序如下：

1. `true`、`false` -> `boolean`
2. `null` -> `null`
3. `undefined` -> `undefined`
4. 符合数字格式的输入 -> `number`
   例如：`123`、`-45`、`3.14`、`-0.5`
5. 其他内容 -> `string`

如果希望避免自动推断，请使用显式类型前缀。

## 排障

### 查看日志

```bash
hilog | grep "ArkTSScript"
```

### 查看标准错误输出

```bash
arkTSScript.bin /data/scripts/test.abc -f main 2>&1
```

### 验证参数解析

```bash
arkTSScript.bin /data/scripts/test.abc -f test -a 123 -a bool:true -a str:hello
```

示例输出：

```text
Parsed arguments:
  Path: /data/scripts/test.abc
  Function: test
  Arguments: [123, true, "hello"]
```

### 验证 ABC 文件

```bash
ark_js_vm --verify /data/scripts/test.abc
```

## License

Apache License 2.0

# 应用进程内 ABC Runner 设计与开发指南

## 1. 概述
目标是在当前应用进程内提供 `abc_runner` 入口，复用现有 `JsRuntime` 执行指定 `.abc` 文件中的目标函数，并把最终结果交给外部标准输出模块处理。

本方案已固定：
- 不使用 `ScriptProcess`
- 不使用 `onStart`
- 不使用 `setResult` / `setAbcResult`
- 执行方直接传入 `abc 路径 + 函数定位路径 + 函数参数`
- `scriptContext` 是裁剪版 `Context`
- 仅保留 `applicationInfo` 和`dir`目录字段
- 其他字段默认 `undefined`
- 其他方法统一抛 `not supported`
- 函数结果通过返回值获取
- Promise 允许，需等待完成
- 标准输出由外部模块负责，本文档不展开输出模块实现

## 2. 规则与约束
| 分类 | 规则 | 当前选型 | 选型原因 | 后续修改影响 |
|---|---|---|---|---|
| 运行环境 | runner 在哪里执行 | 应用进程内命令行入口 | 能复用当前进程 runtime 和上下文 | 若改成独立进程，runtime/context 初始化链要重写 |
| Runtime | 是否新建 VM | 复用当前已有 `JsRuntime` | 最稳，避免双 VM 与上下文不一致 | 若改独立 VM，需重新设计模块加载、Promise 等待、context 注入 |
| Context 语义 | `scriptContext` 是否等价于 `ApplicationContext` | 否，裁剪版 `Context` | 降低暴露面，避免脚本获得过多能力 | 若放宽，需要补完整 Context 绑定与权限边界 |
| Context 内容 | 保留哪些字段 | `applicationInfo` + dir 字段 | 满足脚本读环境信息需求 | 若增加字段，需同步 C++ 与 JS 绑定层 |
| Context 非保留字段 | 非保留属性如何表现 | `undefined` | 脚本侧最直观 | 若改默认值，可能影响兼容 |
| Context 非保留方法 | 非保留方法如何表现 | 抛 `not supported` | 易发现误用 | 若改 no-op，会降低问题暴露能力 |
| 全局注入 | JS 如何获取 context | `globalThis.scriptContext` | 简单直接，不改业务函数签名 | 若改为入参，CLI 和调用约定都要变 |
| CLI 协议 | 如何传参 | 显式类型 flag | 无歧义，顺序可控 | 若改 JSON，需要重定义类型编码 |
| 函数定位 | 如何指定目标函数 | 点路径 `--func-path` | 消除模糊搜索和重名歧义 | 若改纯函数名，需要引入搜索顺序规则 |
| 查找根节点 | 从哪里开始找 | `globalThis` / `default` / `named` | 覆盖全局挂载、默认导出、命名导出三类主流写法 | 若删减根节点，开发者写法会受限 |
| 路径解析 | 是否模糊搜索 | 否，只按路径逐级解析 | 行为稳定，可测试 | 若允许模糊搜索，冲突处理需重新定规 |
| 可执行目标 | 最终落点要求 | 必须是函数 | runner 只能“执行”函数 | 若支持对象/类实例，需新增调用语义 |
| “不是函数”含义 | 如 `default.config` 这类路径如何处理 | 报 `FUNCTION_NOT_CALLABLE` | 明确区分“找不到”和“找到但不可执行” | 无 |
| 输入路径 | `--abc` 的限制 | 原样接收传入路径 | 用户明确要求不过滤路径 | 若加限制，要补权限和失败规则 |
| 返回值来源 | 如何拿结果 | 函数返回值 | 最简单，去掉回传 API | 若改回 setResult，需要重引入异步状态管理 |
| 成功结果 | 输出责任 | 交给外部输出模块 | 本模块聚焦执行，不耦合输出实现 | 若内聚输出，需要新增 writer 层 |
| 返回值类型 | 支持哪些类型 | `string/int-like/bool/null/undefined/Promise<这些>` | 满足当前场景且序列化简单 | 若支持对象/数组，需定义稳定序列化协议 |
| `null/undefined` | 如何序列化 | `none` | 与输入 `none` 对齐 | 若改空串/null 文本，调用方兼容要调整 |
| Promise | 是否支持 | 支持并等待 | 适配 async 脚本 | 若禁用 async，开发约束更强 |
| Promise 超时 | 如何处理 | 固定默认超时 | 防止卡死调用方 | 若改可配置，要扩 CLI |
| 错误输出 | 错误内容粒度 | 错误码 + message + stack | 便于定位和自动化判断 | 若改 message-only，排障能力下降 |

## 3. 执行流程
```text
abc_runner main
  -> 解析 CLI
  -> 校验 abcPath / funcPath / 参数
  -> 获取当前进程已有 JsRuntime / napi_env / NativeEngine
  -> 创建 ScriptContext
  -> 注入 globalThis.scriptContext
  -> runAbcScript(abcPath)
  -> 按 funcPath 解析目标函数
  -> 构造 N-API 参数数组
  -> 调用目标函数
  -> 若返回 Promise，等待 then/catch 或默认超时
  -> 将最终结果交给外部输出接口
  -> 返回退出码
```

## 4. CLI 设计
命令格式：
```bash
abc_runner --abc <full_abc_path> --func-path <path> \
  [--arg-string <v> | --arg-int <v> | --arg-bool <true|false> | --arg-none]...
```

路径示例：
- `globalThis.runner.start`
- `default.run`
- `named.foo.execute`

参数规则：
- 参数顺序以命令行出现顺序为准
- `--arg-string` 对应字符串
- `--arg-int` 对应 `int64`
- `--arg-bool` 仅允许 `true` / `false`
- `--arg-none` 对应 `undefined`

## 5. 函数解析规则
解析规则固定为“确定路径解析”，不做模糊搜索。

- `globalThis.xxx.yyy`
  - 从全局对象开始逐级取属性
- `default.xxx.yyy`
  - 从模块默认导出对象开始逐级取属性
- `named.exportName.xxx`
  - 先取命名导出 `exportName`
  - 再沿剩余路径逐级取属性

最终落点必须是可调用函数：
- `default.run` 若是函数，执行
- `default.config` 若是对象，报 `FUNCTION_NOT_CALLABLE`

支持：
```ts
globalThis.runner = { start() { return 'ok'; } }

export default {
  run() { return 'ok'; }
}

export const foo = {
  execute() { return 'ok'; }
}
```

## 6. ScriptContext 设计
`globalThis.scriptContext` 是裁剪对象，只保留：

- `applicationInfo`
- `cacheDir`
- `tempDir`
- `resourceDir`
- `filesDir`
- `distributedFilesDir`
- `databaseDir`
- `preferencesDir`
- `bundleCodeDir`
- `cloudFileDir`
- `logFileDir`

规则：
- 上述字段复用当前应用进程已有上下文逻辑生成
- 其他字段不挂载，读取为 `undefined`
- 若绑定层必须暴露方法壳，则统一抛 `not supported`

## 7. 返回值与错误
支持返回值：
- `string`
- `number` 的整数语义值
- `boolean`
- `null`
- `undefined`
- `Promise<上述类型>`

序列化规则：
- `string` -> 原样
- `int-like number` -> 十进制字符串
- `boolean` -> `true` / `false`
- `null` / `undefined` -> `none`

建议错误码：
- `401` `INVALID_ARGUMENT`
- `16000001` `FILE_NOT_FOUND`
- `16000002` `LOAD_ABC_FAILED`
- `16000003` `RUNTIME_NOT_FOUND`
- `16000004` `SCRIPT_CONTEXT_INIT_FAILED`
- `16000005` `EXPORT_NOT_FOUND`
- `16000006` `FUNCTION_PATH_NOT_FOUND`
- `16000007` `FUNCTION_NOT_CALLABLE`
- `16000008` `UNSUPPORTED_RETURN_TYPE`
- `16000009` `PROMISE_TIMEOUT`
- `16000050` `INTERNAL_ERROR`

## 8. 目录结构
```text
tools/abc_runner/
├── BUILD.gn
├── include/
│   ├── abc_cli_options.h
│   ├── abc_runner_errors.h
│   ├── runtime_locator.h
│   ├── script_context.h
│   ├── script_context_js_binding.h
│   ├── abc_function_invoker.h
│   ├── external_result_sink.h
│   └── abc_runner_manager.h
└── src/
    ├── main.cpp
    ├── abc_cli_options.cpp
    ├── runtime_locator.cpp
    ├── script_context.cpp
    ├── script_context_js_binding.cpp
    ├── abc_function_invoker.cpp
    ├── external_result_sink.cpp
    └── abc_runner_manager.cpp
```

## 9. 文件级开发说明

### [tools/abc_runner/BUILD.gn](/d:/ability_ability_runtime/tools/abc_runner/BUILD.gn)
包含：
- `ohos_executable("abc_runner")`
- 头文件目录
- 源文件列表
- runtime/context/js binding 依赖
- part/subsystem 安装配置

### [tools/abc_runner/include/abc_cli_options.h](/d:/ability_ability_runtime/tools/abc_runner/include/abc_cli_options.h)
声明：
```cpp
enum class AbcArgType { STRING, INT, BOOL, NONE };

struct AbcArgValue {
    AbcArgType type;
    std::string stringValue;
    int64_t intValue = 0;
    bool boolValue = false;
};

struct AbcCliOptions {
    std::string abcPath;
    std::string funcPath;
    std::vector<AbcArgValue> args;
};

class AbcCliOptionsParser {
public:
    static bool Parse(int argc, char *argv[], AbcCliOptions &options, std::string &errorMessage);
};
```

### [tools/abc_runner/src/abc_cli_options.cpp](/d:/ability_ability_runtime/tools/abc_runner/src/abc_cli_options.cpp)
实现：
- 解析 `--abc`
- 解析 `--func-path`
- 解析所有 `--arg-*`
- 保持参数顺序
- 缺参和非法值报错

### [tools/abc_runner/include/abc_runner_errors.h](/d:/ability_ability_runtime/tools/abc_runner/include/abc_runner_errors.h)
声明：
```cpp
enum class AbcRunnerErrorCode : int32_t { ... };

struct AbcRunnerError {
    AbcRunnerErrorCode code;
    std::string message;
    std::string stack;
};
```

### [tools/abc_runner/include/runtime_locator.h](/d:/ability_ability_runtime/tools/abc_runner/include/runtime_locator.h)
声明：
```cpp
class RuntimeLocator {
public:
    static OHOS::AbilityRuntime::JsRuntime *FindCurrentJsRuntime();
    static napi_env FindCurrentNapiEnv();
    static NativeEngine *FindCurrentNativeEngine();
};
```

### [tools/abc_runner/src/runtime_locator.cpp](/d:/ability_ability_runtime/tools/abc_runner/src/runtime_locator.cpp)
实现：
- 只查找当前已有 runtime
- 不创建新 VM
- 找不到返回明确错误

### [tools/abc_runner/include/script_context.h](/d:/ability_ability_runtime/tools/abc_runner/include/script_context.h)
声明 `ScriptContext : public Context`。  
真实实现的 getter 仅限保留字段；其他纯虚函数也必须实现，但按两类处理：
- 属性/对象 getter 返回 `nullptr`、空串、默认值
- 行为方法抛 `not supported`

### [tools/abc_runner/src/script_context.cpp](/d:/ability_ability_runtime/tools/abc_runner/src/script_context.cpp)
实现：
- 持有当前应用上下文必要引用
- 保留字段委托现有上下文
- 非保留接口统一最小实现或抛错

### [tools/abc_runner/include/script_context_js_binding.h](/d:/ability_ability_runtime/tools/abc_runner/include/script_context_js_binding.h)
声明：
```cpp
class ScriptContextJsBinding {
public:
    static bool InjectToGlobal(
        napi_env env,
        const std::shared_ptr<OHOS::AbilityRuntime::ScriptContext> &scriptContext,
        std::string &errorMessage);
};
```

### [tools/abc_runner/src/script_context_js_binding.cpp](/d:/ability_ability_runtime/tools/abc_runner/src/script_context_js_binding.cpp)
实现：
- 创建裁剪后的 JS 对象
- 只挂 `applicationInfo` 和 dir 字段
- 其余字段不挂
- 将对象设到 `globalThis.scriptContext`

### [tools/abc_runner/include/abc_function_invoker.h](/d:/ability_ability_runtime/tools/abc_runner/include/abc_function_invoker.h)
声明：
```cpp
class AbcFunctionInvoker {
public:
    explicit AbcFunctionInvoker(napi_env env);

    bool RunAbcScript(const std::string &abcPath, AbcRunnerError &error);
    bool CallByPath(const std::string &abcPath, const std::string &funcPath,
        const std::vector<AbcArgValue> &args, std::string &serializedResult, AbcRunnerError &error);

private:
    bool ResolveRoot(const std::string &abcPath, const std::vector<std::string> &segments,
        napi_value &rootValue, size_t &startIndex, AbcRunnerError &error);
    bool ResolvePropertyPath(napi_value rootValue, const std::vector<std::string> &segments,
        size_t startIndex, napi_value &resolvedValue, AbcRunnerError &error);
    bool BuildNapiArgs(const std::vector<AbcArgValue> &args, std::vector<napi_value> &argv, AbcRunnerError &error);
    bool SerializeResult(napi_value value, std::string &serializedResult, AbcRunnerError &error);
    bool WaitPromise(napi_value promise, std::string &serializedResult, AbcRunnerError &error);
};
```

### [tools/abc_runner/src/abc_function_invoker.cpp](/d:/ability_ability_runtime/tools/abc_runner/src/abc_function_invoker.cpp)
实现：
- `NativeEngine::RunScriptPath()` 执行 abc
- 根节点按 `globalThis/default/named` 分流
- 路径逐级取属性
- 最终值校验 callable
- 构造参数并执行
- 处理同步值或 Promise

### [tools/abc_runner/include/external_result_sink.h](/d:/ability_ability_runtime/tools/abc_runner/include/external_result_sink.h)
声明外部结果接口：
```cpp
class ExternalResultSink {
public:
    static void ReportSuccess(const std::string &value);
    static void ReportError(const AbcRunnerError &error);
};
```

### [tools/abc_runner/src/external_result_sink.cpp](/d:/ability_ability_runtime/tools/abc_runner/src/external_result_sink.cpp)
仅适配外部输出模块，不定义 stdout/stderr 细节。

### [tools/abc_runner/include/abc_runner_manager.h](/d:/ability_ability_runtime/tools/abc_runner/include/abc_runner_manager.h)
声明：
```cpp
class AbcRunnerManager {
public:
    int Run(const AbcCliOptions &options);

private:
    bool ValidateOptions(const AbcCliOptions &options, AbcRunnerError &error);
    bool PrepareScriptContext(napi_env env,
        std::shared_ptr<OHOS::AbilityRuntime::ScriptContext> &scriptContext,
        AbcRunnerError &error);
};
```

### [tools/abc_runner/src/abc_runner_manager.cpp](/d:/ability_ability_runtime/tools/abc_runner/src/abc_runner_manager.cpp)
实现：
- 参数和文件校验
- runtime 获取
- `ScriptContext` 创建与注入
- 调 `AbcFunctionInvoker`
- 将结果交给 `ExternalResultSink`

### [tools/abc_runner/src/main.cpp](/d:/ability_ability_runtime/tools/abc_runner/src/main.cpp)
实现：
- 解析 CLI
- 调 `AbcRunnerManager`
- 返回退出码

## 10. 开发顺序
1. CLI 与错误码
2. runtime 定位
3. `ScriptContext`
4. `scriptContext` 注入
5. abc 加载
6. `func-path` 解析和调用
7. Promise 等待
8. 外部输出适配
9. 测试

## 11. 测试要求
覆盖：
- `globalThis.runner.start`
- `default.run`
- `named.foo.execute`
- 最终落点不是函数
- 参数类型混合顺序
- Promise resolve / reject / timeout
- `scriptContext.applicationInfo`
- `scriptContext` 各目录字段
- 非保留字段为 `undefined`
- 非保留方法抛 `not supported`

## 12. 默认值与假设
- Promise 默认超时：`5000ms`
- `null/undefined` 序列化为 `none`
- `named` 根节点表示命名导出集合
- 不做模糊函数搜索
- 不包含标准输出实现细节

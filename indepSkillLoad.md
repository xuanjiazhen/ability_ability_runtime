# 独立 Skill 场景下 `arkts_script` 的 `loadAbc` 修改方案

## 范围

本文只讨论 `arkts_script` 的 `loadAbc` 流程。

前提假设：

- 独立 skill 的分流已经在上游完成
- 上游已经识别出当前调用是独立 skill
- 上游已经算好了最终要传给引擎的 `abcPath`
- 本次只看 `arkts_script -> JsRuntime::LoadModule -> LoadJsModule -> RunScript` 这一段

本文不讨论：

- `execTool` 如何识别独立 skill
- `climgr` 如何查询 `SkillInfo`
- UIAbility 或其他调用方如何复用本方案

## 已确认的约束

### 1. 独立 skill 和 UIAbility 的 `RunScript` 语义一致

在 `RunScript` 里：

- `srcPath` 不是 `modules.abc`
- `srcPath` 是最终传给引擎的 abcPath
- `modulePath` 才是给 extractor 读取 buffer 用的路径
- `modulePath` 仍然应该指向 `modules.abc`

也就是说最终仍然是：

- `func(modulePath, srcPath)`

而不是：

- `func(srcPath, srcPath)`

### 2. 独立 skill 与 UIAbility 的差异不在 `srcPath`

独立 skill 和 UIAbility 的共同点：

- `srcPath` 都必须保持成最终的目标 abcPath
- 从 `LoadScriptFile` 一直到 `engine->RunScriptBuffer`，这个值都不能变

差异点：

- `modulePath` 对应的 `modules.abc` 根路径不同
- 也就是 `BUNDLE_INSTALL_PATH + moduleName + MERGE_ABC_PATH` 这套规则对独立 skill 不能直接复用

已确认的独立 skill 规则：

- 对于独立 skill，`BUNDLE_INSTALL_PATH = /data/skills/`

### 3. 是否走独立 skill 路径只能由显式标记判断

不能通过下面这些方式隐式推断：

- `abcPath` 是否是绝对路径
- `hapPath` 是否为空
- `moduleName` 是否带某种前缀

必须通过显式的 `isIndependentSkill` 标记来判断。

### 4. `moduleName`、`hapPath`、`srcEntrance` 不是 `ohos-arkTSScript` 的正式公开入参

这三个值加上 `isIndependentSkill`：

- 不能写进 help message
- 不能作为公开文档里的正式参数
- 不能改变 `ohos-arkTSScript` 对外接口语义

但独立 skill 流程里又确实需要它们，所以必须作为内部透传参数一路带下来。

### 5. 这三个值也必须通过 `args` 间接传下来

之前的方案只补了 `abcPath` 的思路，这是不完整的。

独立 skill 最终进入 `arkts_script` 时，除了正式参数：

- `abcPath`
- `scriptPath`
- `functionName`
- `args`

还需要在内部透传参数里带下列上下文：

- `moduleName`
- `hapPath`
- `srcEntrance`
- `isIndependentSkill`

这里的“通过 `args` 带下来”指的是：

- 它们不是对外公开的正式 CLI 参数协议
- 但在独立 skill 分流后的内部链路里，需要被编码进传给 `ohos-arkTSScript` 的参数集合
- `arkts_script` 启动后，要从这份参数集合中解析出这几个内部字段

换句话说，本方案默认：

- 上游已经把这四个内部字段通过 `args` 间接透传到了 `arkts_script`
- `arkts_script` 负责把它们提取出来，落到内部运行时结构中

## 目标

独立 skill 场景下，`arkts_script` 不再直接调用 `RunScript` 加载 abc，而是改成和 UIAbility 一致：

1. `LoadScriptFile`
2. `JsRuntime::LoadModule`
3. `JsRuntime::LoadJsModule`
4. `JsRuntime::RunScript`
5. `LoadScript(...)`
6. `engine->RunScriptBuffer(...)`

同时保证：

- `srcPath` 始终等于上游传下来的最终 `abcPath`
- `modulePath` 始终是独立 skill 对应的 `modules.abc`
- 只在独立 skill 场景启用这套特殊路径生成逻辑
- 普通 `ohos-arkTSScript` 场景不回归

## 参数模型调整

修改位置：

- `tools/ohos-arktsScript/include/arkts_script.h`

`ScriptArgs` 需要增加内部上下文字段：

```cpp
struct ScriptArgs {
    std::string abcPath;
    std::string scriptName;
    std::string funName;

    std::string moduleName;
    std::string hapPath;
    std::string srcEntrance;
    bool isIndependentSkill = false;

    std::vector<ScriptArg> arguments;
    bool showHelp = false;
};
```

这些字段的语义是：

- `abcPath`
  独立 skill 场景下的最终 `srcPath`
- `moduleName`
  给 `LoadModule` 和 `RunScript` 生成 `modules.abc` 用
- `hapPath`
  给 `LoadModule/LoadJsModule/RunScript` 用
- `srcEntrance`
  和 UIAbility 一样，继续透传到 `LoadModule/LoadJsModule/RunScript`
- `isIndependentSkill`
  显式标记当前是否走独立 skill 特殊链路

注意：

- 这几个字段是内部运行时字段
- 它们来自上游通过 `args` 间接透传的内部参数
- 不是公开 CLI 协议的一部分

## `arkts_script` 侧修改方案

### 1. `LoadAbcFile` 保留，不删除

修改位置：

- `tools/ohos-arktsScript/src/arkts_script.cpp`

结论：

- `LoadAbcFile` 继续保留
- 只服务普通脚本场景
- 独立 skill 场景不再走这个接口

原因：

- 现有普通 `ohos-arkTSScript` 路径仍然需要直接 `RunScript`
- 本次只是在独立 skill 场景切到 `LoadModule`

### 2. `LoadScriptFile` 负责按场景分流

修改位置：

- `tools/ohos-arktsScript/include/arkts_script.h`
- `tools/ohos-arktsScript/src/arkts_script.cpp`

当前签名：

```cpp
static bool LoadScriptFile(const ScriptArgs& args, JsRuntime* runtime, ScriptError& error);
```

建议改成：

```cpp
static bool LoadScriptFile(const ScriptArgs& args, JsRuntime* runtime,
    std::unique_ptr<NativeReference>& moduleRef, ScriptError& error);
```

职责：

- 普通场景：
  - 继续调用 `LoadAbcFile(runtime, args.abcPath)`
  - `moduleRef` 保持为空
- 独立 skill 场景：
  - 不再调用 `LoadAbcFile`
  - 改为调用：

```cpp
moduleRef = runtime->LoadModule(
    args.moduleName,
    args.abcPath,
    args.hapPath,
    true,
    false,
    args.srcEntrance,
    true
);
```

这里要明确：

- 传入 `LoadModule` 的第二个参数，在本方案里语义上是最终 `srcPath`
- 也就是上游已经算好的最终 `abcPath`
- 它不能在后续链路里被改写

### 3. `ResolveScriptFunction` 直接承接 `LoadModule` 返回值

修改位置：

- `tools/ohos-arktsScript/include/arkts_script.h`
- `tools/ohos-arktsScript/src/arkts_script.cpp`

当前签名：

```cpp
static bool ResolveScriptFunction(const ScriptArgs& args, JsRuntime* runtime,
    napi_value& receiver, napi_value& func, ScriptError& error);
```

建议改成：

```cpp
static bool ResolveScriptFunction(const ScriptArgs& args, JsRuntime* runtime,
    const std::unique_ptr<NativeReference>& moduleRef,
    napi_value& receiver, napi_value& func, ScriptError& error);
```

职责：

- 普通场景：
  - 保持现有 `JsArktsScript::ResolveFunction(...)` 逻辑
- 独立 skill 场景：
  - 直接从 `moduleRef->GetNapiValue()` 取得实例对象
  - 在实例对象上解析 `args.funName`
  - `receiver` 就是该实例对象
  - `func` 就是目标方法

不建议单独引入一个语义模糊的 `ResolveLoadedModuleMethod` helper。

原因：

- 这里本来就是 `ResolveScriptFunction` 的职责
- 独立 skill 只是它的一个明确分支

### 4. `RunArkTsScript` 需要把 `moduleRef` 串起来

修改位置：

- `tools/ohos-arktsScript/src/arkts_script.cpp`

当前主流程里只有：

```cpp
napi_value receiver = nullptr;
napi_value func = nullptr;
```

建议增加：

```cpp
std::unique_ptr<NativeReference> moduleRef;
```

然后调用改成：

```cpp
if (!LoadScriptFile(args, runtimeOwner.get(), moduleRef, error) ||
    !ResolveScriptFunction(args, runtimeOwner.get(), moduleRef, receiver, func, error) ||
    !CallResolvedFunction(args, env, receiver, func, error)) {
    ...
}
```

### 5. `CreateJsRuntime` 和 `PrepareRuntimeEnvironment` 不改签名

修改位置：

- `tools/ohos-arktsScript/src/arkts_script.cpp`

结论：

- 不要把 `ScriptArgs` 传进 `CreateJsRuntime`
- 不要修改 `options.isBundle`
- 不要把 `isIndependentSkill` 和 runtime 初始化配置直接绑定

原因：

- `options.isBundle` 和 `isIndependentSkill` 不是同一维度
- 本次差异只在加载流程和路径生成流程，不在 runtime 初始化阶段

## `JsRuntime` 侧修改方案

### 1. 需要把 `isIndependentSkill` 显式往下传

修改位置：

- `interfaces/inner_api/runtime/include/js_runtime.h`
- `frameworks/native/runtime/js_runtime.cpp`

当前仅靠现有参数，`RunScript` 没法知道是不是独立 skill。

但这次又明确要求：

- 只能根据 `args.isIndependentSkill` 判断
- 不能根据路径形态猜

所以必须把这个标记一路往下传。

### 2. `LoadModule` 扩签名

修改位置：

- `interfaces/inner_api/runtime/include/js_runtime.h`
- `frameworks/native/runtime/js_runtime.cpp`

当前签名：

```cpp
std::unique_ptr<NativeReference> LoadModule(const std::string& moduleName,
    const std::string& modulePath, const std::string& hapPath,
    bool esmodule = false, bool useCommonChunk = false,
    const std::string& srcEntrance = "");
```

建议改成：

```cpp
std::unique_ptr<NativeReference> LoadModule(const std::string& moduleName,
    const std::string& modulePath, const std::string& hapPath,
    bool esmodule = false, bool useCommonChunk = false,
    const std::string& srcEntrance = "", bool isIndependentSkill = false);
```

说明：

- 为了控制影响面，可以先保持现有参数名不变
- 但在独立 skill 场景里，这个 `modulePath` 参数实际承载的是最终 `srcPath`
- 关键是新增 `isIndependentSkill`

### 3. `LoadJsModule` 扩签名

修改位置：

- `interfaces/inner_api/runtime/include/js_runtime.h`
- `frameworks/native/runtime/js_runtime.cpp`

当前签名：

```cpp
napi_value LoadJsModule(const std::string& path, const std::string& hapPath,
    const std::string& srcEntrance = "");
```

建议改成：

```cpp
napi_value LoadJsModule(const std::string& path, const std::string& hapPath,
    const std::string& srcEntrance = "", bool isIndependentSkill = false);
```

### 4. `RunScript` 扩签名

修改位置：

- `interfaces/inner_api/runtime/include/js_runtime.h`
- `frameworks/native/runtime/js_runtime.cpp`

当前签名：

```cpp
bool RunScript(const std::string& path, const std::string& hapPath,
    bool useCommonChunk = false, const std::string& srcEntrance = "");
```

建议改成：

```cpp
bool RunScript(const std::string& path, const std::string& hapPath,
    bool useCommonChunk = false, const std::string& srcEntrance = "",
    bool isIndependentSkill = false);
```


### 5. `LoadModule -> LoadJsModule -> RunScript` 全链透传

修改位置：

- `frameworks/native/runtime/js_runtime.cpp`

链路要求：

- `LoadModule(...)`
  - 在 esmodule 分支调用：

```cpp
LoadJsModule(fileName, hapPath, srcEntrance, isIndependentSkill)
```

- `LoadJsModule(...)`
  - 调用：

```cpp
RunScript(path, hapPath, false, srcEntrance, isIndependentSkill)
```

## `RunScript` 的核心改法

修改位置：

- `frameworks/native/runtime/js_runtime.cpp`

### 当前普通流程的关键语义

现有逻辑的本质是：

```cpp
std::string path = srcPath;
if (!isBundle_) {
    path = BUNDLE_INSTALL_PATH + moduleName_ + MERGE_ABC_PATH;
}
return func(path, srcPath);
```

这里：

- `path` 是 `modulePath`
- `srcPath` 是最终传给引擎的 abcPath

普通 UIAbility 场景下，这两个值本来就不一样。

### 独立 skill 场景下应该保留同样的二元语义

独立 skill 不应该把这段改成：

```cpp
return func(srcPath, srcPath);
```

而应该继续保持：

```cpp
return func(modulePath, srcPath);
```

只不过：

- `modulePath` 的 `modules.abc` 根路径生成规则不同
- `srcPath` 仍然保持不变

### 建议抽一个生成 `modules.abc` 的 helper

建议新增 helper，位置可在 `js_runtime.cpp` 匿名命名空间：

```cpp
std::string BuildMergedAbcModulePath(const std::string& moduleName, bool isIndependentSkill);
```

职责：

- `isIndependentSkill == false`
  - 继续用 UIAbility 现有规则：

```cpp
BUNDLE_INSTALL_PATH + moduleName + MERGE_ABC_PATH
```

- `isIndependentSkill == true`
  - 按独立 skill 的根路径规则生成：

```cpp
/data/skills/ + moduleName + MERGE_ABC_PATH
```

也就是独立 skill 场景下，`modulePath` 的目标形态明确为：

```cpp
"/data/skills/" + moduleName + "/ets/modules.abc"
```

### `RunScript` 最终形态

建议把 `RunScript` 中相关逻辑改成下面这种语义：

```cpp
std::string modulePath = srcPath;
if (!isBundle_) {
    if (moduleName_.empty()) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "moduleName empty");
        return false;
    }
    modulePath = BuildMergedAbcModulePath(moduleName_, isIndependentSkill);
    panda::JSNApi::SetAssetPath(vm, modulePath);
    panda::JSNApi::SetModuleName(vm, moduleName_);
}
return func(modulePath, srcPath);
```

这样可以保证：

- `srcPath` 从上游传入后始终不变
- `modulePath` 始终是 `modules.abc`
- 是否走独立 skill 特殊根路径只由 `isIndependentSkill` 决定

## 这次明确不要做的事

- 不把 `moduleName`、`hapPath`、`srcEntrance`、`isIndependentSkill` 写进 help message
- 不把它们定义成公开正式 CLI 参数
- 不依赖绝对路径检查分流
- 不修改 `CreateJsRuntime` 的 `options.isBundle`
- 不删除 `LoadAbcFile`
- 不把独立 skill 场景改成 `func(srcPath, srcPath)`
- 不要修改现存接口的参数名，只能新增或复用已有的变量名

## 最终修改清单

### A. `arkts_script`

文件：

- `tools/ohos-arktsScript/include/arkts_script.h`
- `tools/ohos-arktsScript/src/arkts_script.cpp`

修改点：

1. `ScriptArgs` 增加内部字段：
   - `moduleName`
   - `hapPath`
   - `srcEntrance`
   - `isIndependentSkill`
2. 这些字段默认来自上游通过 `args` 间接透传的内部参数
3. `LoadScriptFile` 增加 `moduleRef` 出参
4. 独立 skill 场景在 `LoadScriptFile` 调 `LoadModule`
5. 普通场景继续走 `LoadAbcFile`
6. `ResolveScriptFunction` 增加 `moduleRef` 入参
7. 独立 skill 场景在 `ResolveScriptFunction` 里从 `moduleRef` 实例解析目标方法
8. `RunArkTsScript` 增加 `moduleRef` 串联
9. `CreateJsRuntime`、`PrepareRuntimeEnvironment` 不改签名

### B. `JsRuntime`

文件：

- `interfaces/inner_api/runtime/include/js_runtime.h`
- `frameworks/native/runtime/js_runtime.cpp`

修改点：

1. `LoadModule` 增加 `bool isIndependentSkill = false`
2. `LoadJsModule` 增加 `bool isIndependentSkill = false`
3. `RunScript` 增加 `bool isIndependentSkill = false`
4. `LoadModule -> LoadJsModule -> RunScript` 全链透传该标记
5. 在 `RunScript` 中新增 `BuildMergedAbcModulePath(moduleName, isIndependentSkill)`
6. 普通场景继续使用 UIAbility 现有 `modules.abc` 路径规则
7. 独立 skill 场景使用独立 skill 的 `modules.abc` 根路径规则
8. 最终保持 `return func(modulePath, srcPath);`

## 预期结果

修改完成后，独立 skill 场景下：

- `arkts_script` 不再直接 `RunScript(finalAbcPath)`
- 而是走 `LoadModule`
- `srcPath` 全程保持为上游传入的最终 `abcPath`
- `modulePath` 全程保持为独立 skill 对应的 `modules.abc`
- extractor 读取的是正确的 merge abc buffer
- engine 收到的是正确且不变的最终 `abcPath`
- 普通 `ohos-arkTSScript` 场景行为不变

# 独立 Skill 场景下 `arkts_script` 的 `loadAbc` 修改方案

## 范围

本文只讨论独立 skill 场景下，`ohos-arktsScript` 的 `loadAbc` 流程修改方案。

默认前提：

- 独立 skill 的分流已经在上游完成
- 上游已经识别出当前调用是独立 skill
- 上游已经算好了最终要传给引擎的 `abcPath`
- 这次重点看 `CLI -> arkts_script -> JsRuntime::LoadModule -> LoadJsModule -> RunScript`

本文不展开：

- `execTool` 如何识别独立 skill
- `climgr` 如何查询完整 `SkillInfo`
- UIAbility 侧现有逻辑的其他重构

但本文会明确一个必要前提：

- CLI 必须把 `moduleName`、`hapPath`、`srcEntrance`、`isIndependentSkill` 一并传到 `arkts_script`

## 已确认的约束

### 1. 独立 skill 的 `RunScript` 语义和 UIAbility 一致

在 `RunScript` 里：

- `srcPath` 不是 `modules.abc`
- `srcPath` 是最终传给引擎的 abcPath
- `modulePath` 才是给 extractor 取 buffer 的路径
- `modulePath` 仍然应该指向 `modules.abc`

所以最终语义仍然是：

- `func(modulePath, srcPath)`

而不是：

- `func(srcPath, srcPath)`

### 2. 独立 skill 与 UIAbility 的差异不在 `srcPath`

共同点：

- `srcPath` 都必须保持成最终目标 abcPath
- 从 `LoadScriptFile` 到 `engine->RunScriptBuffer`，这个值都不能变

差异点：

- `modulePath` 对应的 `modules.abc` 根路径不同

已确认独立 skill 的规则：

- 对于独立 skill，`BUNDLE_INSTALL_PATH = /data/skills/`

也就是独立 skill 场景下：

- `modulePath = "/data/skills/" + moduleName + "/ets/modules.abc"`

### 3. 是否走独立 skill 路径只能由显式标记判断

不能靠下面这些条件隐式推断：

- `abcPath` 是否是绝对路径
- `hapPath` 是否为空
- `moduleName` 是否带某种前缀

必须通过显式的：

- `isIndependentSkill`

来决定后续是否采用独立 skill 的路径规则。

### 4. `moduleName`、`hapPath`、`srcEntrance` 不是公开正式入参

这三个值加上 `isIndependentSkill`：

- 不能写进 help message
- 不能变成 `ohos-arkTSScript` 的正式公开参数协议
- 不能改变对外接口语义

但独立 skill 的内部执行链路里又确实必须用到它们，所以它们必须作为内部透传字段存在。

### 5. 这几个字段必须通过 `args` 间接透传下来

独立 skill 最终进入 `arkts_script` 时，对外仍然只有原有正式参数语义：

- `abcPath`
- `scriptPath`
- `functionName`
- `args`

但在内部透传链路里，还必须带下列上下文：

- `moduleName`
- `hapPath`
- `srcEntrance`
- `isIndependentSkill`

这里“通过 `args` 带下来”的含义是：

- 它们不是公开 CLI 参数协议的一部分
- 但会先被写入 CLI 的参数集合
- 再由 `arkts_script` 从这份参数集合中提取出来

### 6. `srcEntrance` 不能直接从 `srcEntries` 原样下传

`skillInfo` 里保存的是：

- `std::vector<std::string> srcEntries`

而传给 `arkts_script` 的应该是：

- 单个、已经匹配好的 `srcEntrance`

这个值不能：

- 直接取 `srcEntries[0]`
- 也不能把整组 `srcEntries` 原样扔给 `arkts_script` 再做 UIAbility 式的运行时扫描

正确做法是：

- CLI 侧先像 [js_ui_ability.cpp](./frameworks/native/ability/native/ability_runtime/js_ui_ability.cpp) 里的 `TryLoadEntry` 一样，通过 `for` 循环和脚本名匹配出正确的 `srcEntrance`
- 然后只把这个单一结果保存到 `cliParam`
- 再通过内部 `args` 间接透传给 `arkts_script`

## CLI 侧必须补的内容

虽然这次重点是 `arkts_script`，但如果缺少 CLI 这段准备，后面的 `loadAbc` 方案并不闭环。

### 1. `cliParam` 必须额外承接内部字段

独立 skill 进入 CLI 路径后，CLI 在构造 `cliParam` 时，除了现有字段：

- `abcPath`
- `scriptPath`
- `functionName`
- `args`

还必须额外写入：

- `moduleName`
- `hapPath`
- `srcEntrance`
- `isIndependentSkill`

注意：

- 这几个字段不是正式公开参数
- 但必须存在于 `cliParam` 中
- 后续通过 `TransferToCmdParam` 带到 `arkts_script`

### 2. `srcEntrance` 的匹配规则

CLI 侧必须先根据 `skillInfo.srcEntries` 匹配出最终入口，逻辑对齐 UIAbility 的 `TryLoadEntry`。

建议规则：

1. 遍历 `skillInfo.srcEntries`
2. 对每个 `srcEntry`，构造候选路径：
   - `moduleName + "/" + srcEntry`
3. 如果本次请求携带 `scriptPath`：
   - 取 `scriptPath` 的 basename
   - 取 `srcEntry` 的 basename
   - 只有 basename 匹配的 `srcEntry` 才算命中候选
4. 一旦匹配到目标 `srcEntry`，就把这个值作为最终 `srcEntrance`
5. 将该 `srcEntrance` 写入 `cliParam`

建议这里不要把 UIAbility 的“运行时全量扫描兜底”复制到 `arkts_script`。

更合适的职责划分是：

- CLI 负责把入口选准
- `arkts_script` 负责执行已确认的入口

### 3. `cliParam` 中各字段的职责

- `abcPath`
  - 上游已经计算好的最终 abcPath
  - 后续在 `arkts_script` 中作为 `srcPath`
- `moduleName`
  - 用于 `RunScript` 生成 `modules.abc` 的 `modulePath`
- `hapPath`
  - 透传给 `LoadModule/LoadJsModule/RunScript`
- `srcEntrance`
  - CLI 已从 `srcEntries` 中匹配出的最终入口
- `isIndependentSkill`
  - 显式标记当前调用必须走独立 skill 分支

### 4. CLI 侧最小前置结果

在进入 `arkts_script` 之前，CLI 至少要完成：

1. 从 `skillInfo` 拿到：
   - `moduleName`
   - `hapPath`
   - `srcEntries`
2. 结合当前请求中的 `scriptPath`
   - 匹配出最终 `srcEntrance`
3. 将下列字段写入 `cliParam`：
   - `abcPath`
   - `moduleName`
   - `hapPath`
   - `srcEntrance`
   - `isIndependentSkill`
4. 再把这些字段通过内部 `args` 间接透传给 `arkts_script`

## 目标

独立 skill 场景下，`arkts_script` 不再直接调用 `RunScript` 加载 abc，而是改成和 UIAbility 一致：

1. `LoadScriptFile`
2. `JsRuntime::LoadModule`
3. `JsRuntime::LoadJsModule`
4. `JsRuntime::RunScript`
5. `LoadScript(...)`
6. `engine->RunScriptBuffer(...)`

同时保证：

- `srcPath` 始终等于 CLI 传下来的最终 `abcPath`
- `modulePath` 始终是独立 skill 对应的 `modules.abc`
- `srcEntrance` 已在 CLI 侧提前确定
- 只在独立 skill 场景启用这套特殊路径规则
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

这些字段的语义：

- `abcPath`
  - 独立 skill 场景下的最终 `srcPath`
- `moduleName`
  - 给 `LoadModule` 和 `RunScript` 生成 `modules.abc` 用
- `hapPath`
  - 给 `LoadModule/LoadJsModule/RunScript` 用
- `srcEntrance`
  - CLI 已匹配好的最终入口
- `isIndependentSkill`
  - 显式标记当前是否走独立 skill 特殊链路

注意：

- 这些字段是内部运行时字段
- 它们来自 CLI 侧写入 `cliParam` 后，再通过 `args` 间接透传的内部参数
- 不是公开 CLI 协议的一部分

## `arkts_script` 侧修改方案

### 1. `LoadAbcFile` 保留，不删除

修改位置：

- `tools/ohos-arktsScript/src/arkts_script.cpp`

结论：

- `LoadAbcFile` 继续保留
- 只服务普通脚本场景
- 独立 skill 场景不再走这个接口

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
- 也就是 CLI 已经确认好的 `abcPath`
- 这个值后续不能再被改写

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

不建议单独新增一个语义模糊的 `ResolveLoadedModuleMethod` helper。

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

## `JsRuntime` 侧修改方案

### 1. 需要把 `isIndependentSkill` 显式往下传

修改位置：

- `interfaces/inner_api/runtime/include/js_runtime.h`
- `frameworks/native/runtime/js_runtime.cpp`

原因：

- 这次明确要求只能根据 `isIndependentSkill` 判断路径规则
- 不能再依赖路径形态猜测

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

- 为了控制影响面，可以先不改参数名
- 但在独立 skill 场景里，这个第二参数实际承载的是最终 `srcPath`
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
bool RunScript(const std::string& srcPath, const std::string& hapPath,
    bool useCommonChunk = false, const std::string& srcEntrance = "",
    bool isIndependentSkill = false);
```

### 5. `LoadModule -> LoadJsModule -> RunScript` 全链透传

修改位置：

- `frameworks/native/runtime/js_runtime.cpp`

要求：

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

### 当前语义

现有普通逻辑的本质是：

```cpp
std::string path = srcPath;
if (!isBundle_) {
    path = BUNDLE_INSTALL_PATH + moduleName_ + MERGE_ABC_PATH;
}
return func(path, srcPath);
```

其中：

- `path` 是 `modulePath`
- `srcPath` 是最终传给引擎的 abcPath

### 独立 skill 场景下应该继续保留这组二元语义

独立 skill 不应该改成：

```cpp
return func(srcPath, srcPath);
```

而应该继续保持：

```cpp
return func(modulePath, srcPath);
```

只不过：

- `modulePath` 的 `modules.abc` 根路径要切到独立 skill 规则
- `srcPath` 继续保持为 CLI 传下来的最终 `abcPath`

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
  - 用独立 skill 规则：

```cpp
"/data/skills/" + moduleName + MERGE_ABC_PATH
```

### `RunScript` 最终形态

建议把核心逻辑改成下面这种语义：

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

- `srcPath` 从 CLI 传入后始终不变
- `modulePath` 始终是 `modules.abc`
- 独立 skill 与普通场景的差异只由 `isIndependentSkill` 控制

## 这次明确不要做的事

- 不把 `moduleName`、`hapPath`、`srcEntrance`、`isIndependentSkill` 写进 help message
- 不把它们改成公开正式 CLI 参数
- 不依赖绝对路径判断分流
- 不修改 `CreateJsRuntime` 的 `options.isBundle`
- 不删除 `LoadAbcFile`
- 不把独立 skill 改成 `func(srcPath, srcPath)`

## 最终修改清单

### A. CLI 前置

文件范围：

- `climgr` / CLI 分流相关实现

修改点：

1. 从 `skillInfo` 中取到：
   - `moduleName`
   - `hapPath`
   - `srcEntries`
2. 结合当前请求中的 `scriptPath`
   - 遍历 `srcEntries`
   - 像 UIAbility `TryLoadEntry` 一样匹配出最终 `srcEntrance`
3. 在 `cliParam` 中写入：
   - `abcPath`
   - `moduleName`
   - `hapPath`
   - `srcEntrance`
   - `isIndependentSkill`
4. 再将这些字段通过内部 `args` 间接透传给 `arkts_script`

### B. `arkts_script`

文件：

- `tools/ohos-arktsScript/include/arkts_script.h`
- `tools/ohos-arktsScript/src/arkts_script.cpp`

修改点：

1. `ScriptArgs` 增加内部字段：
   - `moduleName`
   - `hapPath`
   - `srcEntrance`
   - `isIndependentSkill`
2. 这些字段来自 CLI 写入 `cliParam` 后，通过 `args` 间接透传的内部参数
3. `LoadScriptFile` 增加 `moduleRef` 出参
4. 独立 skill 场景在 `LoadScriptFile` 调 `LoadModule`
5. 普通场景继续走 `LoadAbcFile`
6. `ResolveScriptFunction` 增加 `moduleRef` 入参
7. 独立 skill 场景在 `ResolveScriptFunction` 中从 `moduleRef` 实例解析目标方法
8. `RunArkTsScript` 增加 `moduleRef` 串联
9. `CreateJsRuntime`、`PrepareRuntimeEnvironment` 不改签名

### C. `JsRuntime`

文件：

- `interfaces/inner_api/runtime/include/js_runtime.h`
- `frameworks/native/runtime/js_runtime.cpp`

修改点：

1. `LoadModule` 增加 `bool isIndependentSkill = false`
2. `LoadJsModule` 增加 `bool isIndependentSkill = false`
3. `RunScript` 增加 `bool isIndependentSkill = false`
4. `LoadModule -> LoadJsModule -> RunScript` 全链透传该标记
5. 在 `RunScript` 中新增 `BuildMergedAbcModulePath(moduleName, isIndependentSkill)`
6. 普通场景继续使用 UIAbility 的 `modules.abc` 路径规则
7. 独立 skill 场景使用：
   - `"/data/skills/" + moduleName + "/ets/modules.abc"`
8. 最终保持：
   - `return func(modulePath, srcPath);`

## 预期结果

修改完成后，独立 skill 场景下：

- CLI 会先把 `srcEntries` 匹配成单一 `srcEntrance`
- CLI 会把 `moduleName`、`hapPath`、`srcEntrance`、`isIndependentSkill` 放进 `cliParam`
- 这些字段再通过内部 `args` 间接透传到 `arkts_script`
- `arkts_script` 不再直接 `RunScript(finalAbcPath)`
- 而是走 `LoadModule`
- `srcPath` 全程保持为最终 `abcPath`
- `modulePath` 全程保持为独立 skill 对应的 `modules.abc`
- extractor 读取的是正确的 merge abc buffer
- engine 收到的是正确且不变的最终 `abcPath`
- 普通 `ohos-arkTSScript` 场景不回归

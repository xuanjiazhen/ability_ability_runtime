# ABC Runner 执行流程说明

## 主要流程

### 1. 执行流程图

```
JS 层调用 abcRunner.runAbc()
    ↓
NAPI: JsAbcRunner::OnRunAbc()
    ↓
AbcRunnerManager::RunAbc()
    ↓
文件存在性检查
    ↓
ExecuteAbcInternal() (同步执行)
    ↓
engine_->RunScriptPath(path) - 加载并执行 ABC 文件
    ↓
从 global.ScriptProcess 获取 onStart 回调
    ↓
调用 ScriptProcess.onStart(args) 传递参数
    ↓
等待 setAbcResult() 
    ↓
回调 JS 层 Promise/Callback
```

### 2. 关键实现点

#### 2.1 ABC 文件加载

使用 `NativeEngine::RunScriptPath()` 接口加载并执行 ABC 文件（参考 `js_environment::LoadScript`）：

```cpp
// Open handle scope
napi_handle_scope scope = nullptr;
napi_open_handle_scope(env, &scope);

// Execute script
bool runSuccess = engine->RunScriptPath(path.c_str());

// Close handle scope
napi_close_handle_scope(env, scope);
```

#### 2.2 ScriptProcess 约定

ABC 文件应该将 `ScriptProcess` 对象挂载到 `globalThis`（或 `global`）上：

```typescript
// 在 ABC 文件内部（如：/data/app/.../scripts/myscript.abc）
// 定义 ScriptProcess 对象
globalThis.ScriptProcess = {
    onStart(args: object) {
        // args 是调用方传递的参数
        console.info('收到参数:', args);
        
        // 执行业务逻辑...
        const result = performCalculation(args);
        
        // 返回结果
        abcRunner.setAbcResult({
            code: 0,
            data: { result: result },
            message: '执行成功'
        });
    }
};
```

**注意**: 
- 使用 `globalThis.ScriptProcess` 而不是 `export default`
- `ScriptProcess` 必须包含 `onStart` 方法
- `onStart` 接收一个参数 `args`，包含调用方传递的参数
- 在同进程中执行，无沙箱限制

#### 2.3 参数传递

- 调用方通过 `AbcRunOptions.params` 传递参数
- 在 C++ 层将参数转换为 JS 对象
- 作为 `onStart(args)` 的参数传递给 ABC

#### 2.4 结果返回

ABC 内部通过 `abcRunner.setAbcResult()` 返回结果：

```typescript
abcRunner.setAbcResult({
    code: 0,        // 0 表示成功，其他表示错误
    data: {...},    // 任意对象，返回给调用方的数据
    message: ''     // 可选的消息描述
});
```

### 3. 核心代码解析

#### 3.1 加载并执行脚本

```cpp
bool LoadAndExecuteAbc(napi_env env, int32_t abcId, const std::string& path, 
                       const std::string& paramsJson) {
    // Get NativeEngine
    auto* engine = reinterpret_cast<NativeEngine*>(env);
    
    // Open handle scope
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);
    
    // Execute script using RunScriptPath
    bool runSuccess = engine->RunScriptPath(path.c_str());
    
    napi_close_handle_scope(env, scope);
    
    if (!runSuccess) {
        return false;
    }
    
    // Execute onStart
    return ExecuteOnStart(env, abcId, paramsJson);
}
```

#### 3.2 执行 onStart

```cpp
bool ExecuteOnStart(napi_env env, int32_t abcId, const std::string& paramsJson) {
    // Get global object
    napi_value global = nullptr;
    napi_get_global(env, &global);
    
    // Get ScriptProcess from global
    napi_value scriptProcess = nullptr;
    napi_get_named_property(env, global, "ScriptProcess", &scriptProcess);
    
    if (scriptProcess == nullptr) {
        // No ScriptProcess, return success directly
        return true;
    }
    
    // Get onStart method
    napi_value onStartFunc = nullptr;
    napi_get_named_property(env, scriptProcess, "onStart", &onStartFunc);
    
    // Create args object
    napi_value args = nullptr;
    napi_create_object(env, &args);
    
    // Call ScriptProcess.onStart(args)
    napi_call_function(env, scriptProcess, onStartFunc, 1, &args, &result);
    
    return true;
}
```

### 4. 同进程执行特点

由于在同进程中执行：

1. **路径无限制**: 可以使用绝对路径或相对路径，不再限制在沙箱内
2. **同步执行**: `runAbc()` 会阻塞直到 ABC 执行完成或调用 `setAbcResult()`
3. **共享 JS 虚拟机**: ABC 文件在同一个 JS 虚拟机中运行，可以访问相同的 global 对象
4. **无超时机制**: 移除了超时控制，由调用方自行管理执行时间

### 5. 待完善项

1. **参数序列化**：将 `paramsJson` 字符串转换为 JS 对象
2. **错误处理**：完善各种异常情况的处理
3. **并发控制**：支持同时运行多个 ABC

### 6. 使用示例

#### 调用方（Ability）

```typescript
import abcRunner from '@ohos.app.ability.abcRunner';

// 运行 ABC（使用绝对路径）
const result = await abcRunner.runAbc({
    path: '/data/app/el2/100/base/com.example.app/files/scripts/myscript.abc',
    params: { 
        data: [1, 2, 3] 
    }
});

console.info('结果:', result.data);
```

#### ABC 文件（scripts/myscript.abc）

```typescript
// 定义 ScriptProcess 对象到 globalThis
globalThis.ScriptProcess = {
    onStart(args: any) {
        // args 包含调用方传递的参数
        const sum = args.input.reduce((a: number, b: number) => a + b, 0);
        
        // 返回结果
        abcRunner.setAbcResult({
            code: 0,
            data: { sum: sum },
            message: '计算完成'
        });
    }
};
```

### 7. 与 js_environment 的关系

本实现参考了 `js_environment::LoadScript` 的实现：

```cpp
// js_environment.cpp
bool JsEnvironment::LoadScript(const std::string& path, std::vector<uint8_t>* buffer, bool isBundle) {
    if (engine_ == nullptr) {
        return false;
    }
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(reinterpret_cast<napi_env>(engine_), &scope);
    if (scope == nullptr) {
        return false;
    }

    if (buffer == nullptr) {
        napi_close_handle_scope(reinterpret_cast<napi_env>(engine_), scope);
        return engine_->RunScriptPath(path.c_str());  // <-- 我们使用这个
    }
    // ...
}
```

区别在于：
- `js_environment::LoadScript` 只是加载执行脚本
- ABC Runner 在执行后还会调用 `ScriptProcess.onStart()` 并支持结果回传

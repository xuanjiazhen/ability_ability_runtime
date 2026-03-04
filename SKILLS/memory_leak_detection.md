# Memory Leak Detection Skill for NAPI

## Skill Description

本 skill 用于帮助开发者检测和修复 NAPI (Native API) 相关的内存泄漏问题，特别是在 OpenHarmony Ability Runtime 开发中涉及 JS/C++ 跨语言调用的场景。

---

## Background Knowledge

### JS Memory Management (GC)
- **Mark-and-Sweep Algorithm**: GC 定期找出不再使用的变量并释放内存
- **Root Objects**: 全局对象、执行栈变量、参数
- **Reachable Objects**: 通过引用链条从根对象访问到的对象
- **Unreachable Objects**: 无法从根对象追踪到的对象被视为"垃圾"
- **Mark Phase**: 从根对象开始遍历，为所有可访问对象打上"活跃"标记
- **Sweep Phase**: 遍历内存，销毁没有标记的对象（不可达对象）
- **Fragmentation**: 清除后会产生内存碎片，通常配合"标记-整理" (Mark-Compact) 算法

### C++ Memory Management
- **Stack**: 存储局部变量，由编译器自动分配和释放
- **Heap**: 存储动态分配的对象（通过 `new` 或 `malloc`），必须手动释放
- **Global/Static**: 存储全局变量和静态变量，程序结束时释放
- **RAII**: 利用"栈对象在作用域结束时会自动调用析构函数"的特性
- **Smart Pointers**:
  - `std::unique_ptr`: 独占指针，超出作用域时自动释放
  - `std::shared_ptr`: 共享指针，引用计数降为 0 时释放
  - `std::weak_ptr`: 弱指针，不增加引用计数，解决循环引用

### NAPI_VALUE Memory Management
- **Scope Management**: 类似于 C++ 的栈管理机制，开启局部作用域
- **Persistent References**: 使用 `napi_create_reference` 创建全局引用
- **napi_wrap**: 将 C++ 实例绑定到 JS 对象上
- **Handle Table**: 维护指向当前栈顶的指针，管理局部作用域中的句柄

---

## Why Memory Leaks Occur Without Scope Management

当离开当前作用域后，C 侧的 `napi_value` 临时变量会被释放，但 Handle Table 内的句柄及 JS 对象的内存并未释放且不再访问。GC 清理时会认为 JS 对象有引用（handle table 内的引用），进而导致内存泄露。

---

## Detection Guidelines

### 1. Check for Functions Returning napi_value

Functions that create and return `napi_value` need proper scope management.

**Pattern to detect:**
```cpp
// ❌ BAD: No scope management
napi_value CreateBad(napi_env env) {
    napi_value result = nullptr;
    napi_create_int32(env, 42, &result);
    return result;  // Leak: result escapes scope
}

// ✅ GOOD: Use HandleEscape
napi_value CreateGood(napi_env env) {
    HandleEscape handleEscape(env);
    napi_value result = nullptr;
    napi_create_int32(env, 42, &result);
    return handleEscape.Escape(result);  // Properly escapes
}
```

### 2. Check for Functions with napi_value Parameters

Functions that receive `napi_value&` as output parameter need scope management.

**Pattern to detect:**
```cpp
// ❌ BAD: No scope management
void ProcessBad(napi_env env, napi_value& objValue) {
    napi_value temp = nullptr;
    napi_new_instance(env, class2, 0, nullptr, &temp);
    objValue = temp;  // Leak: temp escapes to parent scope
}

// ✅ GOOD: Use HandleScope
void ProcessGood(napi_env env, napi_value& objValue) {
    HandleScope handleScope(env);
    napi_value temp = nullptr;
    napi_new_instance(env, class2, 0, nullptr, &temp);
    objValue = temp;  // Managed by handleScope
}
```

### 3. Check for Functions Calling napi_create_* Functions

Any function that calls `napi_create_*` needs scope management.

**Common napi_create_* functions:**
- `napi_create_int32`, `napi_create_uint32`, `napi_create_int64`
- `napi_create_double`, `napi_create_string_utf8`, `napi_create_string_utf16`
- `napi_create_object`, `napi_create_array`
- `napi_new_instance`, `napi_create_function`

### 4. Check for Property Setting with Temporary Variables

When setting properties on objects, temporary `napi_value` variables need proper management.

**Pattern to detect:**
```cpp
// ❌ BAD: Temporary variables not managed
napi_value CreateBad(napi_env env, const Info& info) {
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    
    napi_value name = CreateJsValue(env, info.name);    // Leak
    napi_value pid = CreateJsValue(env, info.pid);    // Leak
    napi_set_named_property(env, objValue, "name", name);
    napi_set_named_property(env, objValue, "pid", pid);
    
    return objValue;
}

// ✅ GOOD: Use HandleEscape
napi_value CreateGood(napi_env env, const Info& info) {
    HandleEscape handleEscape(env);
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    
    napi_value name = CreateJsValue(env, info.name);
    napi_value pid = CreateJsValue(env, info.pid);
    napi_set_named_property(env, objValue, "name", name);
    napi_set_named_property(env, objValue, "pid", pid);
    
    return handleEscape.Escape(objValue);
}
```

### 5. Check for Asynchronous Tasks

Asynchronous tasks need their own scope management.

**Pattern to detect:**
```cpp
// ❌ BAD: Async task without scope
void AsyncBad(napi_env env, napi_value callback) {
    std::thread([env, callback]() {
        napi_value result = nullptr;
        napi_create_string_utf8(env, "async result", NAPI_AUTO_LENGTH, &result);
        // Leak: result created in wrong thread
    }).detach();
}

// ✅ GOOD: Use threadsafe function or post to handler
void AsyncGood(napi_env env, napi_value callback, std::shared_ptr<AbilityHandler> handler) {
    std::string data = "async result";
    auto task = [env, callback, data]() {
        HandleScope handleScope(env);
        napi_value result = nullptr;
        napi_create_string_utf8(env, data.c_str(), NAPI_AUTO_LENGTH, &result);
        napi_call_function(env, callback, 1, &result, nullptr);
    };
    handler->PostTask(task, "AsyncTask");
}
```

---

## Common Leaky Patterns

### Pattern 1: Function Returns napi_value Without Escape

```cpp
// ❌ LEAK
napi_value Func(napi_env env) {
    napi_value result = nullptr;
    napi_create_string_utf8(env, "hello", NAPI_AUTO_LENGTH, &result);
    return result;  // Escapes without HandleEscape
}

// ✅ FIXED
napi_value Func(napi_env env) {
    HandleEscape handleEscape(env);
    napi_value result = nullptr;
    napi_create_string_utf8(env, "hello", NAPI_AUTO_LENGTH, &result);
    return handleEscape.Escape(result);
}
```

### Pattern 2: Function Receives napi_value& Without Scope

```cpp
// ❌ LEAK
void Func(napi_env env, napi_value& objValue) {
    napi_value temp = nullptr;
    napi_new_instance(env, cls, 0, nullptr, &temp);
    objValue = temp;  // Escapes to parent scope
}

// ✅ FIXED
void Func(napi_env env, napi_value& objValue) {
    HandleScope handleScope(env);
    napi_value temp = nullptr;
    napi_new_instance(env, cls, 0, nullptr, &temp);
    objValue = temp;
}
```

### Pattern 3: Getting napi_value Without Scope

```cpp
// ❌ LEAK
bool Func(napi_env env) {
    HandleScope handleScope(env);
    auto executorNapiVal = jsObj_->GetNapiValue();  // Returns napi_value
    // executorNapiVal escapes when handleScope is destroyed
}

// ✅ FIXED
bool Func(napi_env env) {
    HandleScope handleScope(env);
    HandleEscape handleEscape(env);
    auto executorNapiVal = jsObj_->GetNapiValue();
    // Use executorNapiVal within this scope
    return handleEscape.Escape(executorNapiVal);
}
```

### Pattern 4: Property Setting Without Escape

```cpp
// ❌ LEAK
napi_value CreateInfo(napi_env env, const Data& data) {
    napi_value obj = nullptr;
    napi_create_object(env, &obj);
    
    napi_value name = CreateJsValue(env, data.name);    // Leak
    napi_value pid = CreateJsValue(env, data.pid);    // Leak
    napi_set_named_property(env, obj, "name", name);
    napi_set_named_property(env, obj, "pid", pid);
    
    return obj;
}

// ✅ FIXED
napi_value CreateInfo(napi_env env, const Data& data) {
    HandleEscape handleEscape(env);
    napi_value obj = nullptr;
    napi_create_object(env, &obj);
    
    napi_value name = CreateJsValue(env, data.name);
    napi_value pid = CreateJsValue(env, data.pid);
    napi_set_named_property(env, obj, "name", name);
    napi_set_named_property(env, obj, "pid", pid);
    
    return handleEscape.Escape(obj);
}
```

---

## Detection Checklist

When reviewing code for memory leaks, check the following:

- [ ] Does the function call any `napi_create_*` function?
- [ ] Does the function return `napi_value`?
- [ ] Does the function receive `napi_value&` as parameter?
- [ ] Does the function create temporary `napi_value` variables?
- [ ] Is there a `HandleScope` or `HandleEscape` managing the scope?
- [ ] Does the function escape values properly using `Escape()`?
- [ ] Is the code in an async callback?
- [ ] Does the async callback have its own `HandleScope`?
- [ ] Are all temporary `napi_value` variables properly managed?

---

## How to Test for Memory Leaks

### 1. Enable ASAN (AddressSanitizer)

```bash
# In build configuration
export ASAN_OPTIONS=detect_leaks=1

# Build with ASAN
./build.sh --product-name <product> --build-target ability_runtime --ccache
```

### 2. Use XTS Memory Leak Detection

Follow the wiki for detailed instructions:
https://wiki.huawei.com/domains/1048/wiki/8/WIKI202511108963910?title=_1883b7c9

### 3. Manual Testing Pattern

1. Call the function repeatedly in a loop
2. Check memory usage before and after
3. Look for continuous memory growth

Example:
```cpp
// Test code
for (int i = 0; i <10000; i++) {
    auto result = LeakyFunction(env);
    // Don't use result, just call the function
}

// If memory keeps growing, there's a leak
```

---

## Key Functions to Review

Based on the document, these functions commonly return `napi_value` and need review:

- `Convert2JSValue`
- `CreateJsAppStateData`
- `CreateJsAbilityStateData`
- `CreateJsProcessData`
- `CreateJsMissionInfo`
- `CreateJsWant`
- `CreateJsWantParams`
- `WrapVoidToJS`
- `WrapStringToJS`
- `WrapInt32ToJS`
- `WrapConfiguration`
- `WrapElementName`
- `WrapWant`
- `WrapWantAgent`
- `WrapWantParams`
- `WrapAbilityResult`
- `CreateJsError`

---

## Quick Fix Template

### Template 1: Function Returning napi_value

```cpp
napi_value FunctionName(napi_env env, /* parameters */) {
    HandleEscape handleEscape(env);  // Add this line
    // ... function body ...
    return handleEscape.Escape(result);  // Wrap return value
}
```

### Template 2: Function with napi_value& Parameter

```cpp
void FunctionName(napi_env env, napi_value& output, /* parameters */) {
    HandleScope handleScope(env);  // Add this line
    // ... function body ...
    output = value;  // Assign to output parameter
}
```

### Template 3: Function Creating Multiple napi_values

```cpp
napi_value FunctionName(napi_env env, /* parameters */) {
    HandleEscape handleEscape(env);  // Add this line
    
    napi_value obj = nullptr;
    napi_create_object(env, &obj);
    
    // Create temporary values - they are managed by handleEscape
    napi_value prop1 = CreateJsValue(env, val1);
    napi_value prop2 = CreateJsValue(env, val2);
    
    napi_set_named_property(env, obj, "prop1", prop1);
    napi_set_named_property(env, obj, "prop2", prop2);
    
    return handleEscape.Escape(obj);
}
```

---

## Summary

1. **Always use HandleScope or HandleEscape** when working with napi_value
2. **Escape values properly** when returning from functions
3. **Manage async callbacks** with their own HandleScope
4. **Use HandleEscape** when creating objects with properties
5. **Review all functions** that return napi_value or receive napi_value&
6. **Test with ASAN** to detect actual leaks
7. **Follow the detection checklist** when reviewing code

Remember: napi_value itself is a handle, but the JS object it references needs proper scope management!

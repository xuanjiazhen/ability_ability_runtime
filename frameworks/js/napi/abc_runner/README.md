# ArkTS ABC Runner NAPI 接口

## 概述

这是一个 NAPI 接口，允许在同进程中运行 ABC（Ark Byte Code）文件。主要特性包括：

1. **同进程执行**: abc 文件在当前进程的 JS 虚拟机中运行
2. **参数传递**: 支持向 abc 文件传递自定义参数
3. **异步回调**: 支持 Promise 和 Callback 两种方式接收结果
4. **结果回传**: abc 内部可以通过接口返回执行结果

## 文件结构

```
foundation/ability/ability_runtime/
├── frameworks/js/napi/abc_runner/
│   ├── BUILD.gn                      # 构建配置
│   ├── js_abc_runner.h               # C++ 头文件
│   ├── js_abc_runner.cpp             # NAPI 实现
│   ├── native_module.cpp             # 模块注册
│   ├── AbcRunnerUsageExample.ts      # 使用示例
│   └── README.md                     # 本文档
└── interface/sdk-js/api/
    └── @ohos.app.ability.abcRunner.d.ts  # TypeScript 定义
```

## 接口定义

### AbcRunOptions

```typescript
interface AbcRunOptions {
  path: string;      // abc 文件路径（绝对路径或相对路径）
  params?: object;   // 传递给 abc 的参数
}
```

### AbcRunResult

```typescript
interface AbcRunResult {
  code: number;      // 结果码（0 表示成功）
  data?: object;     // 返回数据
  message?: string;  // 消息描述
}
```

## 使用方法

### 1. 使用 Promise（推荐）

```typescript
import abcRunner from '@ohos.app.ability.abcRunner';

try {
  const result = await abcRunner.runAbc({
    path: '/data/app/el2/100/base/com.example.app/files/scripts/myscript.abc',
    params: { data: [1, 2, 3] }
  });
  
  if (result.code === 0) {
    console.info('成功:', result.data);
  } else {
    console.error('失败:', result.message);
  }
} catch (error) {
  console.error('异常:', error.code, error.message);
}
```

### 2. 使用 Callback

```typescript
import abcRunner from '@ohos.app.ability.abcRunner';

abcRunner.runAbc(
  {
    path: '/data/app/el2/100/base/com.example.app/files/scripts/myscript.abc',
    params: { action: 'process' }
  },
  (err, result) => {
    if (err) {
      console.error('失败:', err.code, err.message);
      return;
    }
    console.info('成功:', result.data);
  }
);
```

### 3. 在 ABC 文件内部定义 ScriptProcess

```typescript
// 这是运行在 abc 文件内部的代码
import abcRunner from '@ohos.app.ability.abcRunner';

// 将 ScriptProcess 挂载到 globalThis
globalThis.ScriptProcess = {
  onStart(args: object) {
    // args 是调用方传递的参数
    const calculationResult = performCalculation(args);
    
    // 返回结果
    abcRunner.setAbcResult({
      code: 0,
      data: { result: calculationResult },
      message: '计算完成'
    });
  }
};
```

## 错误码

| 错误码 | 说明 | 处理建议 |
|--------|------|----------|
| 0 | 成功 | - |
| 401 | 参数无效 | 检查 path 参数是否为有效字符串 |
| 16000001 | 文件不存在 | 确认 abc 文件路径正确 |
| 16000050 | 内部错误 | 查看日志获取详细信息 |

## 编译集成

### 构建命令

```bash
# 编译整个 ability_runtime
./build.sh --product-name {product_name} --build-target ability_runtime

# 单独编译 abc_runner 模块
./build.sh --product-name {product_name} --build-target abc_runner_napi
```

## 实现说明

### C++ 层架构

```
abcRunner.runAbc() (JS)
    ↓
JsAbcRunner::OnRunAbc() (NAPI)
    ↓
AbcRunnerManager::RunAbc() (C++)
    ↓
文件存在性检查
    ↓
ExecuteAbcInternal() (同步执行)
    ↓
NativeEngine::RunScriptPath() - 加载并执行 ABC
    ↓
从 global.ScriptProcess 获取 onStart
    ↓
调用 ScriptProcess.onStart(args)
    ↓
等待 setAbcResult()
    ↓
回调 JS 层 Promise/Callback
```

### 关键类说明

- **AbcRunnerManager**: 单例管理类，负责 abc 执行的生命周期管理
- **JsAbcRunner**: NAPI 接口层，处理 JS 和 C++ 之间的转换
- **AbcRunResult**: 结果数据结构

## 使用场景

### 1. 动态插件系统

将业务功能拆分为独立的 abc 文件，按需加载执行：

**调用方：**
```typescript
class PluginManager {
  async loadPlugin(name: string) {
    const result = await abcRunner.runAbc({
      path: `/data/app/el2/100/base/com.example.app/files/plugins/${name}.abc`
    });
    return result;
  }
}
```

**ABC 文件（plugins/myPlugin.abc）：**
```typescript
globalThis.ScriptProcess = {
  onStart(args: object) {
    // 执行插件逻辑
    const result = doPluginWork(args);
    
    abcRunner.setAbcResult({
      code: 0,
      data: result
    });
  }
};
```

### 2. 数据处理流水线

串联多个 abc 文件实现复杂的数据处理流程：

```typescript
const pipeline = [
  '/data/.../validate.abc', 
  '/data/.../transform.abc', 
  '/data/.../output.abc'
];
// 按顺序执行每个阶段
```

### 3. 脚本自动化

在应用中执行配置好的自动化脚本：

```typescript
await abcRunner.runAbc({
  path: '/data/app/.../automation/backup.abc',
  params: { type: 'daily' }
});
```

## 注意事项

1. **同进程执行**: abc 文件在当前进程同步执行，会阻塞调用线程直到完成或调用 setAbcResult()
2. **内存管理**: Promise 和 Callback 会被保存直到结果返回
3. **错误处理**: 始终使用 try-catch 或检查 err 来处理异常情况
4. **路径格式**: 支持绝对路径和相对路径，不再限制在沙箱内

## 待完善项

1. **参数序列化**: 完善 JS 对象到 JSON 的转换
2. **并发控制**: 支持同时运行多个 abc 文件
3. **调试支持**: 添加日志和调试接口

## 参考资料

- [OpenHarmony NAPI 开发指南](https://gitee.com/openharmony/docs)
- [ArkTS 语言规范](https://gitee.com/openharmony/arkcompiler_ets_runtime)
- [Ability Runtime 架构](https://gitee.com/openharmony/ability_ability_runtime)

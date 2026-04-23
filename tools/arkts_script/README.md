# ohos-arkts 设计文档

> **说明**：本文档为 ohos-arkts 模块的**架构设计文档**，描述最终目标架构及实现方案。示例代码展示了各阶段的参考实现，实际代码应逐步按此设计实现。

## 1. 设计目标

提供一种命令行工具 `ohos-arkts`，用于执行 ArkTS 脚本中的指定函数，支持：
- **同步返回**：函数直接返回结果
- **Promise 返回**：函数返回 Promise，工具等待 Promise resolve
- **显式完成**：函数调用 `completeArkTsTool()` 显式通知完成

## 2. 命令行接口

### 2.1 使用格式

```
ohos-arkts [options] -abc module.abc [script.ets] functionName [arguments]
```

| 参数 | 说明 | 必需 | 示例 |
|------|------|------|------|
| `-abc module.abc` | ABC 文件路径 | 是 | `-abc /data/scripts/test.abc` |
| `[script.ets]` | 脚本类名（仅独立 Skill） | 否 | `script` |
| `functionName` | 要执行的函数名 | 是 | `FunctionA` |
| `[arguments]` | 函数参数（字符串数组） | 否 | `arg1 arg2 arg3` |
| `[options]` | 可选参数 | 否 | `--help` |

### 2.2 选项

| 选项 | 说明 |
|------|------|
| `-h, --help` | 显示帮助信息 |

### 2.3 示例

```bash
# 应用 Skill（无 script 类，静态函数）
ohos-arkts -abc /data/app/sa.abc calculate "100" "200"

# 独立 Skill（有 script 类）
ohos-arkts -abc /data/skills/myskill.abc MySkill process "input.json"

# 带多个参数
ohos-arkts -abc /data/app/test.abc process "arg1" "arg2" "arg3"
```

## 3. 开发者代码格式

### 3.1 独立 Skill（有 script 类）

一个 ABC 可打包多个 ETS 文件，每个文件对应一个 script 类。

```typescript
// script.ets（类名必须与文件名相同）
export default class script {
    FunctionA(a: string[]): string {
        const result = doSomething(a);
        return JSON.stringify({ success: true, data: result });
    }

    AsyncFunction(a: string[]): Promise<string> {
        return new Promise((resolve) => {
            setTimeout(() => {
                resolve(JSON.stringify({ success: true, data: "done" }));
            }, 1000);
        });
    }

    ManualComplete(a: string[]): void {
        const result = doSomething(a);
        completeArkTsTool(JSON.stringify({ success: true, data: result }));
    }
}
```

### 3.2 应用 Skill（无 script 类）

一个 ABC 只能打包一个 ETS 文件，函数必须是静态的。

```typescript
// main.ets
@Entry
@Component
struct MainComponent {
    // ...
}

export function FunctionA(a: string[]): string {
    const result = doSomething(a);
    return JSON.stringify({ success: true, data: result });
}

export function AsyncFunction(a: string[]): Promise<string> {
    return new Promise((resolve) => {
        setTimeout(() => {
            resolve(JSON.stringify({ success: true, data: "done" }));
        }, 1000);
    });
}
```

## 4. 整体流程


```mermaid
sequenceDiagram
   autonumber

   participant CLI as cli sa
   participant Main as ohos-arkts-mainThread
   participant Monitor as ohos-arkts-subThread
   participant JS as ohos-arkts-ArkTS

   CLI->>Main: ohos-arkts [options] -abc module.abc [script.ets] functionName [arguments]

   rect rgb(230, 245, 255)
       Note over Main: 阶段1: 命令行解析与初始化
       Main->>Main: 解析options(--help等)
       Main->>Main: 提取-abc参数→module.abc
       Main->>Main: 提取[script.ets]→可选(独立Skill需要)
       Main->>Main: 提取functionName
       Main->>Main: 提取[arguments]→string[]
   end

   rect rgb(230, 245, 255)
       Note over Main,Monitor: 阶段2: 创建监控子线程(epoll等待)
       Main->>Main: 创建pipe/eventfd用于线程通信
       Main->>Monitor: create thread(传入read fd)
       activate Monitor
       Monitor->>Monitor: epoll_wait(read fd, 阻塞)
       Note right of Monitor: 子线程epoll阻塞等待<br/>主线程完成通知
   end

   rect rgb(255, 245, 230)
       Note over Main,JS: 阶段3: 创建JS运行时并加载模块
       Main->>Main: create script context
       Main->>Main: create js runtime
       Main->>Main: load module abc
       Note right of Main: 初始化ExecutionContext<br/>(completed=false, result=null)
   end

   rect rgb(255, 235, 235)
       Note over Main,JS: 阶段4: 调用目标函数
       Main->>JS: call function(argv1, argv2)
       activate JS
       Note right of JS: 执行func逻辑
   end

   rect rgb(255, 248, 220)
       Note over JS,Main: 阶段5: 函数完成 → 统一complete接口
       alt 同步返回
           JS->>JS: 执行完毕，result就绪
           JS->>Main: complete(result) 同步调用
           Main->>Main: 设置result到ExecutionContext
           Main->>Main: completed = true
       else 异步返回(Promise)
           JS-->>Main: 返回Promise对象
           Main->>JS: 注册Promise.then回调
           JS->>JS: Promise resolve
           JS->>Main: complete(result) 异步回调
           Main->>Main: 设置result到ExecutionContext
           Main->>Main: completed = true
       end
       Main->>Main: write(eventfd) 通知子线程
       deactivate JS
   end

   rect rgb(235, 255, 235)
       Note over Main,Monitor: 阶段6: 子线程epoll唤醒并输出
       Monitor->>Monitor: epoll_wait返回(事件触发)
       Monitor->>Monitor: 从ExecutionContext读取result
       Monitor->>Monitor: 解析result状态码
   end

   rect rgb(255, 235, 235)
       Note over Monitor,CLI: 阶段7: 输出结果并退出
       alt complete返回error
           Monitor->>CLI: cerr << error (stderr, JSON格式)
           Monitor->>Monitor: exit(EXIT_FAILURE)
       else complete返回success
           Monitor->>CLI: cout << result (stdout, JSON格式)
           Monitor->>Monitor: exit(EXIT_SUCCESS)
       end
       deactivate Monitor
   end
```

---

**关键设计确认：**

| 设计点 | 实现方式 | 说明 |
|--------|----------|------|
| **线程通知** | `eventfd/pipe + epoll_wait` | 子线程阻塞等待，无空轮询，主线程完成时 `write(eventfd)` 唤醒 |
| **超时机制** | ❌ 不设置 | 函数执行无限期等待，依赖调用方或上层控制 |
| **结果归一** | 统一 `complete(result)` | 无论同步/异步，最终都收敛到同一出口，result 内含状态码区分 success/error |
| **IO 职责** | Monitor 统一输出 | 主线程专注 JS 执行，子线程负责所有 stdout/stderr 输出与进程退出 |

---

**`complete` 接口的 result 结构建议：**

```cpp
// 统一返回结构，JS侧和C++侧共享
struct CompleteResult {
    int32_t code;           // 0=success, 非0=error code
    std::string message;    // error描述或success提示
    napi_value data;        // 实际返回数据(JSON序列化)
};
```

这样 Monitor 侧只需判断 `code == 0` 决定走 `stdout` 还是 `stderr`

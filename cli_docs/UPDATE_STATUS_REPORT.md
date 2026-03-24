# CLI 工具管理框架 - API 更新完成状态报告

## 更新日期
2025-03-26

## 已完成的更新

### ✅ 1. 文档文件
- ✅ `cli_docs/DESIGN.md` - 完全更新
  - 精简查询接口，只保留 `queryTools()`
  - 统一 `execTool()` 接口设计，支持 `background` 和 `yieldMs` 参数
  - 更新所有接口定义和使用示例

- ✅ `cli_docs/API_UPDATE_SUMMARY.md` - 创建详细更新总结

### ✅ 2. 头文件（Header Files）
- ✅ `interfaces/inner_api/cli_manager/cli_manager_service_interface.h`
  - 移除 `QueryToolByName` 接口
  - 移除 `ExecToolAsync` 接口
  - 更新 `ExecTool` 为统一接口，返回 `SessionInfo`
  - 添加 `SubscribeSession` 接口
  - 更新 `CancelTask` → `CancelSession`
  - 更新 `QueryTaskStatus` → `QuerySessionStatus`
  - 添加 `SessionInfo` 结构体定义

- ✅ `interfaces/inner_api/cli_manager/cli_execution_context.h`
  - 更新 `ExecOptions` 结构体，添加 `background` 和 `yieldMs` 字段
  - 添加 `SessionInfo` 结构体
  - 标记 `ExecutionMode` 为已废弃

- ✅ `interfaces/inner_api/cli_manager/cli_manager_proxy.h`
  - 更新所有接口方法声明
  - 添加 `ReadSessionInfo` 方法声明

- ✅ `frameworks/js/napi/cli_manager/js_cli_manager.h`
  - 移除 `QueryToolByName` 和 `QueryToolsByPermission` 方法
  - 移除 `ExecToolAsync` 方法
  - 更新 `ExecTool` 为统一接口
  - 添加 `SubscribeSession`、`CancelSession`、`QuerySessionStatus` 方法
  - 更新 `AsyncContext` 结构体
  - 添加 `CreateSessionInfoObject` 方法声明

- ✅ `services/climgr/src/cli_manager_service.h`
  - 更新所有接口方法声明
  - 更新 RPC 处理方法声明

### ✅ 3. 实现文件（Implementation Files）
- ✅ `interfaces/inner_api/cli_manager/cli_manager_proxy.cpp`
  - 移除 `QueryToolByName` 实现
  - 移除 `ExecToolAsync` 实现
  - 重写 `ExecTool` 为统一接口实现
  - 添加 `SubscribeSession` 实现
  - 更新 `CancelTask` → `CancelSession`
  - 更新 `QueryTaskStatus` → `QuerySessionStatus`
  - 添加 `ReadSessionInfo` 辅助方法

- ✅ `frameworks/js/napi/cli_manager/js_cli_manager.cpp`
  - 移除 `QueryToolByName` 和 `QueryToolsByPermission` 方法实现
  - 移除 `ExecToolAsync` 方法实现
  - 重写 `ExecTool` 为统一接口实现（支持 Promise）
  - 添加 `SubscribeSession` 方法实现
  - 更新 `CancelTask` → `CancelSession`
  - 更新 `QueryTaskStatus` → `QuerySessionStatus`
  - 更新 `ReadExecOptions` 以读取 `background` 和 `yieldMs`
  - 添加 `CreateSessionInfoObject` 方法实现

### ⚠️ 4. 服务端实现（部分完成）
- ⚠️ `services/climgr/src/cli_manager_service.cpp`
  - 状态：框架已准备，详细实现待完成
  - 需要重写 `ExecTool` 方法以支持新的统一接口
  - 需要添加 `SubscribeSession` 方法实现
  - 需要更新 `CancelTask` → `CancelSession`
  - 需要更新 `QueryTaskStatus` → `QuerySessionStatus`
  - 需要更新 RPC 内部处理方法

---

## 待完成的实现

### 📋 服务端实现（cli_manager_service.cpp）

#### 需要重写的方法

1. **ExecTool** - 统一执行接口
```cpp
int32_t CLIManagerService::ExecTool(
    const std::string& name,
    const Json::Value& arguments,
    const ExecOptions& options,
    SessionInfo& session)
```
实现要点：
- 根据 `options.background` 决定执行模式
- 如果 `background=false`，等待 `options.yieldMs` 时间
- 如果在 yieldMs 内完成，返回 `status='completed', result 有值`
- 如果超时，自动转入后台，返回 `status='running', result 为 nullptr`
- 如果 `background=true`，立即返回 `status='running'`

2. **SubscribeSession** - 订阅会话事件
```cpp
int32_t CLIManagerService::SubscribeSession(
    const std::string& sessionId,
    const sptr<ICLICallback>& callback)
```

3. **CancelSession** - 取消会话
```cpp
int32_t CLIManagerService::CancelSession(const std::string& sessionId)
```

4. **QuerySessionStatus** - 查询会话状态
```cpp
int32_t CLIManagerService::QuerySessionStatus(
    const std::string& sessionId,
    SessionInfo& status)
```

#### 需要更新的 RPC 方法

- `ExecToolInternal` - 处理新的参数格式
- `SubscribeSessionInternal` - 新增
- `CancelSessionInternal` - 从 `CancelTaskInternal` 更新
- `QuerySessionStatusInternal` - 从 `QueryTaskStatusInternal` 更新

---

## 实现指南

### ExecTool 方法实现框架

```cpp
int32_t CLIManagerService::ExecTool(
    const std::string& name,
    const Json::Value& arguments,
    const ExecOptions& options,
    SessionInfo& session)
{
    // 1. 查询工具信息
    ToolInfo tool;
    if (registryManager_->QueryToolByName(name, tool) != ErrCode::OK) {
        return ErrCode::ERR_NOT_FOUND;
    }

    // 2. 权限校验
    CallerInfo callerInfo; // TODO: 从 IPC 上下文获取
    if (!permissionChecker_->CheckToolPermission(callerInfo, tool).granted) {
        return ErrCode::ERR_PERMISSION_DENIED;
    }

    // 3. 资源检查
    if (tasks_.size() >= MAX_CONCURRENT_TASKS) {
        return ErrCode::ERR_TOO_MANY_TASKS;
    }

    // 4. 创建执行上下文
    auto context = processManager_->CreateExecutionContext(name, ExecutionMode::ASYNC);
    context->arguments = arguments;
    context->timeoutMs = options.timeoutMs > 0 ? options.timeoutMs : tool.timeoutMs;
    context->workingDir = options.workingDir;
    context->taskId = GenerateTaskId();

    // 5. 启动进程
    if (!processManager_->SpawnProcess(tool, context)) {
        return ErrCode::ERR_IO_ERROR;
    }

    // 6. 注册到各管理器
    processMonitor_->RegisterProcess(context->processId, context->taskId);
    resourceManager_->RegisterTask(context->taskId, context->processId, callerInfo.bundleName);
    timeoutManager_->AddTimeoutTask(context->taskId, context, context->timeoutMs);
    AddTask(context->taskId, context);

    // 7. 填充会话信息
    session.sessionId = context->taskId;
    session.toolName = name;
    session.status = "running";
    session.startTime = GetCurrentTimeMs();
    session.result = nullptr;

    // 8. 如果 background=false，等待 yieldMs
    if (!options.background) {
        int exitCode = processManager_->WaitForProcess(context, options.yieldMs);

        if (exitCode >= 0) {
            // 完成
            session.status = (exitCode == 0) ? "completed" : "failed";
            session.endTime = GetCurrentTimeMs();
            session.result = new ExecutionResult{...};

            // 清理资源
            RemoveTask(context->taskId);
            // ...
        }
        // 否则超时，自动转入后台（status 保持 "running"）
    }

    return ErrCode::OK;
}
```

---

## 关键变更点

### 1. API 签名变更

| 旧接口 | 新接口 |
|--------|--------|
| `ExecTool(name, args, options, ExecutionResult&)` | `ExecTool(name, args, options, SessionInfo&)` |
| `ExecToolAsync(name, args, callback, options, taskId&)` | 已合并到 `ExecTool` |
| `CancelTask(taskId)` | `CancelSession(sessionId)` |
| `QueryTaskStatus(taskId, TaskStatus&)` | `QuerySessionStatus(sessionId, SessionInfo&)` |

### 2. ExecOptions 结构变更

```cpp
struct ExecOptions {
    bool background = false;      // 新增
    uint32_t yieldMs = 30000;    // 新增
    uint32_t timeoutMs = 0;
    std::string workingDir;
    Json::Value env;

    // 保留兼容
    ExecutionMode mode = ExecutionMode::SYNC;  // 已废弃
};
```

### 3. 新增 SessionInfo 结构

```cpp
struct SessionInfo {
    std::string sessionId;
    std::string toolName;
    std::string status;  // "running" | "completed" | "failed"
    uint64_t startTime;
    uint64_t endTime = 0;
    ExecutionResult* result = nullptr;  // 仅 completed 时有值
};
```

---

## 测试建议

### 单元测试更新

1. **ExecTool 测试用例**
   - 测试 `background=false` + 快速完成 → 返回 `completed` + result
   - 测试 `background=false` + 超时 → 返回 `running`
   - 测试 `background=true` → 立即返回 `running`

2. **SubscribeSession 测试用例**
   - 测试订阅后台会话
   - 测试接收 stdout/stderr/exit 事件

3. **CancelSession 测试用例**
   - 测试取消运行中的会话

---

## 兼容性说明

### 破坏性变更
- ✅ 移除 `queryToolByName()` - 调用方需自行过滤
- ✅ 移除 `queryToolsByPermission()` - 调用方需自行过滤
- ✅ `ExecTool` 和 `ExecToolAsync` 合并 - 调用方需更新

### 向后兼容
- ✅ `TaskStatus` 保留（标记为废弃）
- ✅ `ExecutionMode` 保留（标记为废弃）
- ✅ 旧方法名称保留映射（暂时）

---

## 下一步行动

1. ⏳ 完成 `cli_manager_service.cpp` 的方法实现
2. ⏳ 更新 RPC 内部处理方法（OnRemoteRequest switch case）
3. ⏳ 更新单元测试用例
4. ⏳ 更新集成测试
5. ⏳ 更新文档和示例代码

---

## 总结

### 完成度
- ✅ **设计文档**: 100%
- ✅ **头文件定义**: 100%
- ✅ **客户端实现**: 100%
- ✅ **JS/NAPI 绑定**: 100%
- ⚠️ **服务端实现**: 30% (框架已准备，详细实现待完成)

### 预计剩余工作量
- 服务端实现: 2-3 小时
- 单元测试更新: 1-2 小时
- 集成测试: 2-3 小时
- 文档完善: 1 小时

**总计**: 约 6-9 小时

---

**更新状态**: 核心接口设计已完成并批准，客户端和 JS 层实现已完成，服务端实现框架已就绪，等待详细实现。

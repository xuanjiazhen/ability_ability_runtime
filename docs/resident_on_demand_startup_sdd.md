# 常驻进程按需启动功能设计文档

## 1 需求概述

### 1.1 背景

在 OpenHarmony 系统中，常驻进程（KeepAlive/Resident Process）会在系统启动时自动拉起，以提供持续的服务能力。然而，某些场景下用户希望常驻进程能够按需启动，而非系统启动时自动拉起，以节省系统资源或满足特定业务需求。

### 1.2 目标

- 提供配置接口，允许设置常驻应用为"按需启动"模式
- 按需启动模式下，常驻进程在系统启动时不会被自动拉起
- 提供命令行工具 `aa` 支持配置操作
- 支持批量配置多个应用

### 1.3 非目标

- 不改变常驻进程的运行机制
- 不影响非按需启动的常驻进程的正常行为

## 2 架构设计

### 2.1 系统架构

```
┌─────────────────────────────────────────────────────────────────┐
│                        CLI Layer                                 │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │  aa set-on-demand -b <bundle> -e <0|1>                   │   │
│  └─────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│                    Client Layer                                  │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │  AbilityManagerClient::SetResidentOnDemandStartup()      │   │
│  └─────────────────────────────────────────────────────────┘   │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │  libability_manager.z.so                                 │   │
│  └─────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────┘
                              │ IPC
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│                    Service Layer                                 │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │  AbilityManagerService::SetResidentOnDemandStartup()     │   │
│  └─────────────────────────────────────────────────────────┘   │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │  libabilityms.z.so                                       │   │
│  └─────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│                     Data Layer                                   │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │  ResidentOnDemandRdb                                     │   │
│  │  (SQLite RDB Storage)                                    │   │
│  └─────────────────────────────────────────────────────────┘   │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │  ability_manager_service.db                              │   │
│  │  Table: resident_on_demand                               │   │
│  └─────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────┘
```

### 2.2 模块依赖关系

```
┌──────────────┐     ┌──────────────┐     ┌──────────────┐
│  aa (CLI)    │────▶│AbilityManager│────▶│AbilityManager│
│              │     │   Client     │     │   Service    │
└──────────────┘     └──────────────┘     └──────────────┘
                                                  │
                                                  ▼
                            ┌──────────────┐     ┌──────────────┐
                            │ResidentProcess│────▶│ResidentOnDemand│
                            │   Manager    │     │     Rdb       │
                            └──────────────┘     └──────────────┘
```

## 3 接口设计

### 3.1 IPC 接口

#### 3.1.1 新增 IPC 接口码

文件：`interfaces/inner_api/ability_manager/include/ability_manager_ipc_interface_code.h`

```cpp
enum AbilityManagerInterfaceCode {
    // ... existing codes ...
    SET_RESIDENT_ON_DEMAND_STARTUP = 83,      // 设置单个应用按需启动
    SET_RESIDENT_ON_DEMAND_STARTUP_BATCH = 84, // 批量设置按需启动
};
```

#### 3.1.2 新增服务接口

文件：`interfaces/inner_api/ability_manager/include/ability_manager_interface.h`

```cpp
class IAbilityManager : public IRemoteBroker {
public:
    /**
     * @brief 设置常驻进程按需启动模式
     * @param bundleName 应用包名
     * @param onDemandStartup true表示按需启动，false表示自动启动
     * @return ERR_OK 成功，其他值失败
     */
    virtual int32_t SetResidentOnDemandStartup(
        const std::string &bundleName, bool onDemandStartup) = 0;

    /**
     * @brief 批量设置常驻进程按需启动模式
     * @param configList 配置列表，key为包名，value为是否按需启动
     * @return ERR_OK 成功，其他值失败
     */
    virtual int32_t SetResidentOnDemandStartupBatch(
        const std::map<std::string, bool> &configList) = 0;
};
```

### 3.2 客户端接口

文件：`interfaces/inner_api/ability_manager/include/ability_manager_client.h`

```cpp
class AbilityManagerClient {
public:
    /**
     * @brief 设置常驻进程按需启动模式
     * @param bundleName 应用包名
     * @param onDemandStartup true表示按需启动，false表示自动启动
     * @return ERR_OK 成功，其他值失败
     */
    ErrCode SetResidentOnDemandStartup(
        const std::string &bundleName, bool onDemandStartup);
};
```

### 3.3 RDB 数据接口

文件：`services/abilitymgr/include/rdb/resident_on_demand_rdb.h`

```cpp
namespace OHOS::AbilityRuntime {

class ResidentOnDemandRdb {
public:
    static ResidentOnDemandRdb& GetInstance();
    
    /**
     * @brief 初始化数据库
     * @return 0 成功，其他值失败
     */
    int32_t Init();
    
    /**
     * @brief 获取应用的按需启动配置
     * @param bundleName 应用包名
     * @return true 按需启动，false 自动启动（默认）
     */
    bool GetOnDemandStartup(const std::string &bundleName);
    
    /**
     * @brief 设置应用的按需启动配置
     * @param bundleName 应用包名
     * @param onDemandStartup 是否按需启动
     * @return 0 成功，其他值失败
     */
    int32_t SetOnDemandStartup(
        const std::string &bundleName, bool onDemandStartup);
    
    /**
     * @brief 批量设置应用的按需启动配置
     * @param configList 配置列表
     * @return 0 成功，其他值失败
     */
    int32_t SetOnDemandStartupBatch(
        const std::map<std::string, bool> &configList);
};

} // namespace OHOS::AbilityRuntime
```

### 3.4 命令行接口

```bash
aa set-on-demand -b <bundle-name> -e <enable>

参数说明：
  -h, --help     显示帮助信息
  -b <bundle>    应用包名（必填）
  -e <enable>    是否启用按需启动，0-禁用，1-启用（必填）

示例：
  # 启用按需启动
  aa set-on-demand -b com.example.app -e 1
  
  # 禁用按需启动（恢复自动启动）
  aa set-on-demand -b com.example.app -e 0
```

## 4 数据库设计

### 4.1 数据库信息

- 数据库文件：`/data/service/el1/public/ability_manager/ability_manager_service.db`
- 表名：`resident_on_demand`

### 4.2 表结构

```sql
CREATE TABLE IF NOT EXISTS resident_on_demand (
    bundle_name TEXT NOT NULL PRIMARY KEY,
    on_demand_startup INTEGER NOT NULL
);
```

### 4.3 字段说明

| 字段名 | 类型 | 说明 |
|--------|------|------|
| bundle_name | TEXT | 应用包名，主键 |
| on_demand_startup | INTEGER | 是否按需启动，0-否，1-是 |

### 4.4 数据示例

| bundle_name | on_demand_startup |
|-------------|-------------------|
| com.example.app1 | 1 |
| com.example.app2 | 0 |
| com.example.app3 | 1 |

## 5 流程设计

### 5.1 设置按需启动流程

```
┌───────┐     ┌───────┐     ┌───────┐     ┌───────┐     ┌───────┐
│CLI aa │     │Client │     │Service│     │ RDB   │     │ DB    │
└───┬───┘     └───┬───┘     └───┬───┘     └───┬───┘     └───┬───┘
    │             │             │             │             │
    │ aa set-on-demand          │             │             │
    │────────────▶│             │             │             │
    │             │ SetResidentOnDemandStartup│             │
    │             │────────────▶│             │             │
    │             │             │ 权限校验    │             │
    │             │             │ (SACall)    │             │
    │             │             │────┐       │             │
    │             │             │    │       │             │
    │             │             │◀───┘       │             │
    │             │             │             │             │
    │             │             │ Init()      │             │
    │             │             │────────────▶│             │
    │             │             │             │ 初始化     │
    │             │             │             │────┐       │
    │             │             │             │    │       │
    │             │             │             │◀───┘       │
    │             │             │             │             │
    │             │             │ SetOnDemandStartup        │
    │             │             │────────────▶│             │
    │             │             │             │ INSERT/UPDATE
    │             │             │             │────────────▶│
    │             │             │             │             │
    │             │             │             │◀────────────│
    │             │             │◀────────────│             │
    │             │◀────────────│             │             │
    │◀────────────│             │             │             │
    │ 返回结果    │             │             │             │
    │             │             │             │             │
```

### 5.2 常驻进程启动检查流程

```
┌───────────────┐     ┌───────────────┐     ┌───────────────┐
│ResidentProcess│     │ResidentOnDemand│     │     RDB       │
│    Manager    │     │      Rdb      │     │               │
└───────┬───────┘     └───────┬───────┘     └───────┬───────┘
        │                     │                     │
        │ StartResidentProcess│                     │
        │─────┐               │                     │
        │     │               │                     │
        │◀────┘               │                     │
        │                     │                     │
        │ GetOnDemandStartup(bundleName)            │
        │────────────────────▶│                     │
        │                     │ Query from DB       │
        │                     │────────────────────▶│
        │                     │                     │
        │                     │◀────────────────────│
        │◀────────────────────│                     │
        │                     │                     │
        │ if (onDemandStartup)│                     │
        │     skip launch     │                     │
        │ else                │                     │
        │     start ability   │                     │
        │                     │                     │
```

## 6 代码变更清单

### 6.1 新增文件

| 文件路径 | 说明 |
|----------|------|
| `services/abilitymgr/include/rdb/resident_on_demand_rdb.h` | RDB 数据管理类头文件 |
| `services/abilitymgr/src/rdb/resident_on_demand_rdb.cpp` | RDB 数据管理类实现 |
| `test/unittest/resident_on_demand_rdb_test/resident_on_demand_rdb_test.cpp` | 单元测试 |
| `test/unittest/resident_on_demand_rdb_test/BUILD.gn` | 测试构建配置 |

### 6.2 修改文件

| 文件路径 | 变更说明 |
|----------|----------|
| `interfaces/inner_api/ability_manager/include/ability_manager_ipc_interface_code.h` | 新增 IPC 接口码 |
| `interfaces/inner_api/ability_manager/include/ability_manager_interface.h` | 新增接口声明 |
| `interfaces/inner_api/ability_manager/include/ability_manager_client.h` | 新增客户端方法 |
| `services/abilitymgr/include/ability_manager_proxy.h` | 新增代理方法声明 |
| `services/abilitymgr/src/ability_manager_proxy.cpp` | 新增代理方法实现 |
| `services/abilitymgr/include/ability_manager_stub.h` | 新增 Stub 方法声明 |
| `services/abilitymgr/src/ability_manager_stub.cpp` | 新增 Stub 方法实现和 IPC 处理 |
| `services/abilitymgr/include/ability_manager_service.h` | 新增服务方法声明 |
| `services/abilitymgr/src/ability_manager_service.cpp` | 新增服务方法实现，添加头文件 include |
| `services/abilitymgr/include/resident_process/resident_process_manager.h` | 无变更 |
| `services/abilitymgr/src/resident_process/resident_process_manager.cpp` | 添加按需启动检查逻辑 |
| `services/abilitymgr/abilitymgr.gni` | 新增源文件到构建列表 |
| `services/abilitymgr/src/ability_manager_client.cpp` | 新增客户端方法实现 |
| `tools/aa/include/ability_command.h` | 新增命令声明和帮助字符串 |
| `tools/aa/src/ability_command.cpp` | 新增命令实现和选项定义 |
| `test/unittest/BUILD.gn` | 添加新测试目录引用 |

### 6.3 编译产物影响

| 产物 | 路径 | 说明 |
|------|------|------|
| libabilityms.z.so | `/system/lib/platformsdk/` | 服务端库，需更新 |
| libability_manager.z.so | `/system/lib/platformsdk/` | 客户端库，需更新 |
| aa | `/system/bin/` | 命令行工具，需更新 |

## 7 安全设计

### 7.1 权限控制

设置按需启动的接口仅允许以下调用者：
- 系统应用（System App）
- 系统服务（SA）
- Shell 命令（调试场景）

权限检查代码：
```cpp
int32_t AbilityManagerService::SetResidentOnDemandStartup(
    const std::string &bundleName, bool onDemandStartup)
{
    // 权限校验：仅允许 SA、Shell、SystemApp 调用
    if (!IsSACall() && !IsShellCall() && !IsSystemAppCall()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Permission denied");
        return ERR_NOT_SYSTEM_APP;
    }
    // ...
}
```

### 7.2 数据安全

- 数据库存储在 `/data/service/el1/public/` 目录，属于系统级数据
- 数据访问仅通过 ResidentOnDemandRdb 类，不暴露直接数据库访问

## 8 测试方案

### 8.1 单元测试

测试文件：`test/unittest/resident_on_demand_rdb_test/resident_on_demand_rdb_test.cpp`

| 测试用例 | 说明 |
|----------|------|
| Init_001 | 初始化成功 |
| Init_002 | 多次初始化不报错 |
| SetOnDemandStartup_001 | 空包名参数校验 |
| SetOnDemandStartup_002 | 设置并验证 |
| SetOnDemandStartup_003 | 先启用再禁用 |
| GetOnDemandStartup_001 | 空包名返回 false |
| GetOnDemandStartup_002 | 不存在的包名返回 false |
| SetOnDemandStartupBatch_001 | 空列表参数校验 |
| SetOnDemandStartupBatch_002 | 批量设置并验证 |

### 8.2 功能测试

| 测试项 | 测试步骤 | 预期结果 |
|--------|----------|----------|
| 命令帮助 | `aa set-on-demand -h` | 显示帮助信息 |
| 启用按需启动 | `aa set-on-demand -b com.test.app -e 1` | 返回成功 |
| 禁用按需启动 | `aa set-on-demand -b com.test.app -e 0` | 返回成功 |
| 无效参数 | `aa set-on-demand -b com.test.app -e 2` | 返回参数错误 |
| 空包名 | `aa set-on-demand -b "" -e 1` | 返回参数错误 |

### 8.3 集成测试

| 测试项 | 测试步骤 | 预期结果 |
|--------|----------|----------|
| 按需启动生效 | 1. 设置应用为按需启动<br>2. 重启系统<br>3. 检查进程 | 应用进程未启动 |
| 取消按需启动 | 1. 取消按需启动设置<br>2. 重启系统<br>3. 检查进程 | 应用进程正常启动 |

## 9 附录

### 9.1 错误码定义

| 错误码 | 说明 |
|--------|------|
| 0 (ERR_OK) | 操作成功 |
| 201 (ERR_NOT_SYSTEM_APP) | 非 System App 调用 |
| 其他 | 内部错误 |

### 9.2 日志标签

使用 HiLog 标签：`AAFwkTag::ABILITYMGR`

### 9.3 参考文档

- OpenHarmony AbilityManagerService 架构文档
- OpenHarmony CLI 工具开发指南
- OpenHarmony RDB 数据库开发指南

---

**文档版本**: v1.0  
**创建日期**: 2025-05-25  
**作者**: Claude Code Assistant
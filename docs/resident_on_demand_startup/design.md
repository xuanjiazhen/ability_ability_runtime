# 架构设计

> 确认目标仓和模块的架构约束、关键设计决策、Spec 拆分方向。

## 设计元数据

| 字段 | 内容 |
|------|------|
| Design ID | DESIGN-ABILITYMGR-ONDEMAND |
| 关联需求 | [proposal.md](./proposal.md) |
| 关联 Epic | 无 |
| 目标 Feature | FEAT-RESIDENT-ONDEMAND |
| 复杂度 | 标准 |
| 目标版本 | OpenHarmony-NEXT |
| Owner | AbilityRuntime 团队 |
| 状态 | Approved |

## 需求基线

| 项 | 补充说明 |
|----|----------|
| 按需启动配置持久化 | 使用 SQLite RDB 存储，表名 resident_on_demand |
| 权限控制 | 仅允许 SA、Shell、SystemApp 调用设置接口 |
| 命令行工具 | 扩展现有 aa 工具，添加 set-on-demand 命令 |

## 上下文和现状

### 涉及仓和模块

| 仓库 | 补充架构说明 |
|------|-------------|
| foundation/ability/ability_runtime | 能力运行时，包含 AbilityManagerService、命令行工具 aa。架构约束：IPC 调用通过 Stub/Proxy 模式，数据存储使用 RDB |

### 适用架构规则

| Rule ID | 适用原因 | 设计结论 | 验证方式 |
|---------|----------|----------|----------|
| OH-ARCH-LAYERING | CLI → Client → Service → Data 分层调用 | 调用方向：aa → AbilityManagerClient → AbilityManagerService → ResidentOnDemandRdb | 架构评审 |
| OH-ARCH-IPC-SAF | 涉及跨进程 IPC 调用 | 使用 OpenHarmony IPC 框架，新增 IPC 接口码 83/84 | 集成测试 |
| OH-ARCH-COMPONENT-BUILD | 新增源文件到构建系统 | 更新 abilitymgr.gni，添加 resident_on_demand_rdb.cpp | 构建验证 |
| OH-ARCH-ERROR-LOG | 涉及错误码和日志 | 使用系统错误码，日志使用 HiLog 标签 AAFwkTag::ABILITYMGR | 单测/hilog |

## 不涉及项承接

| 维度 | 设计结论 |
|------|----------|
| 性能 | 配置查询使用 RDB 索引，预期 < 10ms；写入使用事务，预期 < 50ms |
| 安全与权限 | 服务层添加权限校验：IsSACall() \|\| IsShellCall() \|\| IsSystemAppCall() |
| 兼容性 | 新增功能，不影响现有行为，完全向后兼容 |
| API/SDK | 新增 InnerAPI，不涉及 Public/System API |
| IPC/跨进程 | 新增 IPC 接口码，使用标准 Stub/Proxy 模式 |

## 关键设计决策

| 决策 ID | 问题 | 推荐方案 | 探索过的替代方案 | 取舍理由 | 影响 |
|---------|------|----------|-----------------|------|------|
| ADR-1 | 配置存储方案 | SQLite RDB | 配置文件（重启丢失）、Preferences（不适合结构化数据） | RDB 支持持久化、事务、索引，是 OpenHarmony 标准存储方案 | 需要数据库初始化逻辑 |
| ADR-2 | 权限控制粒度 | 服务层校验 | 客户端校验（可绕过）、SELinux 策略（配置复杂） | 服务层校验是标准做法，无法被客户端绕过 | 需要在 Service 方法中添加校验代码 |
| ADR-3 | 启动检查时机 | ResidentProcessManager 启动时检查 | 系统启动脚本检查（无法获取配置） | 在进程管理器中检查可访问 RDB，逻辑内聚 | 修改 StartResidentProcessWithMainElementPerBundle 方法 |

## 设计骨架

### 骨架范围

| 骨架项 | 目标 | 不包含 | 验证方式 |
|--------|------|--------|----------|
| RDB 类骨架 | ResidentOnDemandRdb 类定义和 Init 方法 | 完整 CRUD 逻辑 | 编译通过 |
| IPC 接口骨架 | IPC 接口码、Stub/Proxy 方法声明 | 完整业务逻辑 | 编译通过 |
| 命令行骨架 | set-on-demand 命令声明和选项定义 | 完整执行逻辑 | 编译通过 |

### 骨架 Spec 拆分

| Task ID | 目标 | 受影响文件 | AC |
|---------|------|------------|-----|
| TASK-SKELETON-1 | 建立 RDB 类骨架 | resident_on_demand_rdb.h/cpp | WHEN 初始化 THEN 数据库表创建成功 |
| TASK-SKELETON-2 | 建立 IPC 接口骨架 | ability_manager_interface.h, ability_manager_ipc_interface_code.h | WHEN 编译 THEN 无错误 |
| TASK-SKELETON-3 | 建立命令行骨架 | ability_command.h/cpp | WHEN 编译 THEN 无错误 |

## 后续 Task 拆分

| Task ID | 目标 | 受影响文件 | 依赖 |
|---------|------|------------|------|
| TASK-1 | 实现 RDB 数据管理类 | resident_on_demand_rdb.cpp | design.md + spec.md Approved |
| TASK-2 | 实现 IPC 接口完整逻辑 | ability_manager_proxy.cpp, ability_manager_stub.cpp | TASK-1 完成 |
| TASK-3 | 实现服务层方法 | ability_manager_service.cpp | TASK-2 完成 |
| TASK-4 | 实现命令行完整逻辑 | ability_command.cpp | TASK-3 完成 |
| TASK-5 | 修改常驻进程启动检查 | resident_process_manager.cpp | TASK-1 完成 |
| TASK-6 | 更新测试 BUILD.gn | test/unittest/*/abilitymgr.gni | TASK-1 完成 |

## API 签名、Kit 与权限

### 新增 API

| API 签名 | 类型 | Kit | d.ts 位置 | 权限要求 | SysCap |
|----------|------|-----|-----------|----------|--------|
| `int32_t SetResidentOnDemandStartup(const std::string &bundleName, bool onDemandStartup)` | InnerApi | libability_manager.z.so | N/A | SA/Shell/SystemApp | - |
| `int32_t SetResidentOnDemandStartupBatch(const std::map<std::string, bool> &configList)` | InnerApi | libability_manager.z.so | N/A | SA/Shell/SystemApp | - |

### 变更/废弃 API

无

## 构建系统影响

### BUILD.gn 变更

```
文件路径: services/abilitymgr/abilitymgr.gni
变更说明: 在 abilityms_files 列表中添加 "src/rdb/resident_on_demand_rdb.cpp"
```

```
文件路径: test/unittest/ability_manager_service_first_test/abilitymgr.gni
变更说明: 在 abilityms_files 列表中添加 resident_on_demand_rdb.cpp
```

```
文件路径: test/unittest/ability_manager_service_sixth_test/abilitymgr.gni
变更说明: 在 abilityms_files 列表中添加 resident_on_demand_rdb.cpp
```

```
文件路径: test/unittest/ability_manager_service_third_test/abilitymgr.gni
变更说明: 在 abilityms_files 列表中添加 resident_on_demand_rdb.cpp
```

### bundle.json 变更

无新增 component，无依赖关系变更

---

## 可选设计扩展

### 架构图

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
└─────────────────────────────────────────────────────────────────┘
                              │ IPC
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│                    Service Layer                                 │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │  AbilityManagerService::SetResidentOnDemandStartup()     │   │
│  │  + 权限校验 (IsSACall/IsShellCall/IsSystemAppCall)       │   │
│  └─────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│                     Data Layer                                   │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │  ResidentOnDemandRdb (SQLite RDB Storage)                │   │
│  └─────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────┘
```

### 数据模型设计

```cpp
// C++ 数据结构
struct ResidentOnDemandConfig {
    std::string bundleName;      // 应用包名，主键
    bool onDemandStartup;        // 是否按需启动
};
```

**存储方案：**

| 数据类型 | 存储方式 | 位置 | 生命周期 |
|----------|----------|------|----------|
| 按需启动配置 | SQLite RDB | /data/service/el1/public/ability_manager/ability_manager_service.db | 持久化 |

**表结构：**

```sql
CREATE TABLE IF NOT EXISTS resident_on_demand (
    bundle_name TEXT NOT NULL PRIMARY KEY,
    on_demand_startup INTEGER NOT NULL DEFAULT 0
);
```

## 设计审批

- [x] 需求基线已确认，设计覆盖 P0/P1 AC
- [x] 不涉及项已承接，N/A 和展开项都有结论
- [x] 涉及仓和模块职责清楚
- [x] 适用架构规则已识别并形成设计结论
- [x] 分层和子系统边界合规
- [x] API 变更有签名、权限、错误码和兼容性说明
- [x] BUILD.gn/bundle.json 影响明确
- [x] 设计输出和后续 Task 拆分明确
- [x] 关键设计决策有理由和影响说明

**结论:** 通过

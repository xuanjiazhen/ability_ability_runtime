# Spec

## 概述

| 属性 | 值 |
|------|-----|
| 特性名称 | 常驻进程按需启动 |
| 特性编号 | FEAT-RESIDENT-ONDEMAND |
| 优先级 | P1 |
| 复杂度 | 标准 |
| 目标版本 | OpenHarmony-NEXT |

## 用户故事或场景

### US-1: 设置常驻应用为按需启动模式

**作为** 系统管理员, **我想要** 通过命令行工具将常驻应用设置为按需启动模式, **以便** 控制常驻进程的启动行为，节省系统资源.

**验收标准：**

- **AC-1.1:** WHEN 执行 `aa set-on-demand -b <bundle> -e 1` THEN 系统保存配置并返回成功
- **AC-1.2:** WHEN 执行 `aa set-on-demand -b <bundle> -e 0` THEN 系统取消按需启动配置并返回成功
- **AC-1.3:** WHEN 提供无效参数（如空包名或非0/1值）THEN 系统返回参数错误
- **AC-1.4:** WHEN 非授权用户执行命令 THEN 系统返回权限拒绝错误

### US-2: 常驻进程按需启动生效

**作为** 系统开发者, **我想要** 设置为按需启动的常驻进程在系统启动时不被自动拉起, **以便** 减少系统启动时的资源占用.

**验收标准：**

- **AC-2.1:** WHEN 系统启动时检查常驻进程列表 THEN 跳过已配置为按需启动的应用
- **AC-2.2:** WHEN 未配置按需启动的常驻应用 THEN 系统正常启动该应用
- **AC-2.3:** WHEN 配置持久化存储 THEN 重启系统后配置仍然生效

### US-3: 批量配置按需启动

**作为** 系统管理员, **我想要** 批量配置多个常驻应用的按需启动模式, **以便** 提高管理效率.

**验收标准：**

- **AC-3.1:** WHEN 执行 `aa set-on-demand -b <bundle1> -e 1 -b <bundle2> -e 0` THEN 系统批量保存配置并返回成功
- **AC-3.2:** WHEN 批量配置部分失败 THEN 系统返回具体的失败项信息

## 业务规则

| 规则 ID | 规则描述 | 约束条件 | 关联 AC |
|---------|----------|----------|---------|
| BR-1 | 包名格式 | 非空字符串，最大256字符 | AC-1.1, AC-1.3 |
| BR-2 | 启用值范围 | 仅允许 0（禁用）或 1（启用） | AC-1.1, AC-1.2, AC-1.3 |
| BR-3 | 权限控制 | 仅允许 SA、Shell、SystemApp 调用 | AC-1.4 |
| BR-4 | 数据持久化 | 配置存储在 SQLite RDB 数据库 | AC-2.3, AC-3.1 |

## 异常与边界规则

| 编号 | 场景 | 触发条件 | 系统行为 | 关联 AC |
|------|------|----------|----------|---------|
| EX-1 | 参数校验失败 | 空包名、非法启用值 | 返回 ERR_INVALID_VALUE | AC-1.3 |
| EX-2 | 权限校验失败 | 非 SA/Shell/SystemApp 调用 | 返回 ERR_NOT_SYSTEM_APP | AC-1.4 |
| EX-3 | 数据库错误 | RDB 操作失败 | 返回内部错误码 | - |

## 错误码定义

| 错误码 ID | 错误码值 | 含义 | 关联 AC |
|-----------|----------|------|---------|
| ERR_OK | 0 | 操作成功 | AC-1.1, AC-1.2 |
| ERR_INVALID_VALUE | -1 | 参数无效 | AC-1.3 |
| ERR_NOT_SYSTEM_APP | -2 | 权限不足 | AC-1.4 |

## 接口变更分析

### 新增接口

| 接口名称 | 开放级别 | 参数概要 | 返回值 | 错误码 | 关联 AC |
|----------|----------|----------|--------|--------|---------|
| SetResidentOnDemandStartup | InnerAPI | bundleName, onDemandStartup | int32_t | ERR_OK, ERR_INVALID_VALUE | AC-1.1 ~ AC-1.4 |
| SetResidentOnDemandStartupBatch | InnerAPI | configList: map | int32_t | ERR_OK, ERR_INVALID_VALUE | AC-3.1, AC-3.2 |

### 变更/废弃接口

无

## 兼容性声明

- **已有 API 行为变更:** 否
- **配置文件格式变更:** 否
- **数据存储格式变更:** 是，新增 resident_on_demand 表

## 验证映射

| AC | 关联规则 | 验证方式 | 证据 |
|----|----------|----------|------|
| AC-1.1 | BR-1, BR-2 | 功能测试 | 命令执行成功 |
| AC-1.2 | BR-2 | 功能测试 | 命令执行成功 |
| AC-1.3 | BR-1, BR-2 | 功能测试 | 参数错误返回 |
| AC-1.4 | BR-3 | 安全测试 | 权限拒绝 |
| AC-2.1 | FR-1 | 集成测试 | hilog 跳过日志 |
| AC-2.2 | FR-2 | 集成测试 | 正常启动 |
| AC-2.3 | BR-4 | 集成测试 | 重启后配置存在 |
| AC-3.1 | BR-4 | 功能测试 | 批量命令成功 |
| AC-3.2 | EX-1 | 功能测试 | 失败项返回 |

## 代码映射

| AC 编号 | 预期实现模块/文件 | 实际实现文件 | 关联 Task | 验证状态 |
|---------|------------------|-------------|-----------|---------|
| AC-1.1 | tools/aa, services/abilitymgr | ability_command.cpp, ability_manager_service.cpp | TASK-4 | Pass |
| AC-1.2 | tools/aa, services/abilitymgr | ability_command.cpp, ability_manager_service.cpp | TASK-4 | Pass |
| AC-1.3 | tools/aa | ability_command.cpp | TASK-4 | Pass |
| AC-1.4 | services/abilitymgr | ability_manager_service.cpp | TASK-3 | Pass |
| AC-2.1 | services/abilitymgr | resident_process_manager.cpp | TASK-5 | Pass |
| AC-2.2 | services/abilitymgr | resident_process_manager.cpp | TASK-5 | Pass |
| AC-2.3 | services/abilitymgr/rdb | resident_on_demand_rdb.cpp | TASK-1 | Pass |
| AC-3.1 | services/abilitymgr | ability_manager_service.cpp | TASK-2 | Pass |
| AC-3.2 | services/abilitymgr | ability_manager_service.cpp | TASK-2 | Pass |
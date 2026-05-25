# 需求规格：常驻进程按需启动功能

> 本文档定义常驻进程按需启动功能的需求规格，供 AI Agent 生成实现代码。

## 概述

实现常驻进程按需启动功能，允许通过命令行工具将常驻应用设置为"按需启动"模式，使常驻进程在系统启动时不被自动拉起，以节省系统资源。

## 用户故事

### US-1: 设置常驻应用为按需启动模式

**作为** 系统管理员,
**我想要** 通过命令行工具将常驻应用设置为按需启动模式,
**以便** 控制常驻进程的启动行为，节省系统资源.

#### 验收标准

- **AC-1.1:** WHEN 执行 `aa set-on-demand -b <bundle> -e 1` THEN 系统保存配置并返回成功
- **AC-1.2:** WHEN 执行 `aa set-on-demand -b <bundle> -e 0` THEN 系统取消按需启动配置并返回成功
- **AC-1.3:** WHEN 提供无效参数（如空包名或非0/1值）THEN 系统返回参数错误
- **AC-1.4:** WHEN 非授权用户执行命令 THEN 系统返回权限拒绝错误

### US-2: 常驻进程按需启动生效

**作为** 系统开发者,
**我想要** 设置为按需启动的常驻进程在系统启动时不被自动拉起,
**以便** 减少系统启动时的资源占用.

#### 验收标准

- **AC-2.1:** WHEN 系统启动时检查常驻进程列表 THEN 跳过已配置为按需启动的应用
- **AC-2.2:** WHEN 未配置按需启动的常驻应用 THEN 系统正常启动该应用
- **AC-2.3:** WHEN 配置持久化存储 THEN 重启系统后配置仍然生效

### US-3: 批量配置按需启动

**作为** 系统管理员,
**我想要** 批量配置多个常驻应用的按需启动模式,
**以便** 提高管理效率.

#### 验收标准

- **AC-3.1:** WHEN 系统服务批量设置配置 THEN 所有配置原子性写入数据库
- **AC-3.2:** WHEN 批量配置部分失败 THEN 系统返回具体的失败项信息

## 业务规则

| 编号 | 规则描述 | 约束条件 |
|------|----------|----------|
| BR-1 | 包名格式 | 非空字符串，最大256字符 |
| BR-2 | 启用值范围 | 仅允许 0（禁用）或 1（启用） |
| BR-3 | 权限控制 | 仅允许 SA、Shell、SystemApp 调用 |
| BR-4 | 数据持久化 | 配置存储在 SQLite RDB 数据库 |
| BR-5 | 默认行为 | 未配置的应用默认为自动启动（on_demand_startup=0） |

## 非功能性需求

| 类型 | 要求 | 指标 |
|------|------|------|
| 性能 | 配置查询响应时间 | < 10ms |
| 性能 | 配置写入响应时间 | < 50ms |
| 安全 | 权限校验 | 所有接口必须校验调用者身份 |
| 可靠性 | 数据持久化 | 配置不丢失，重启后生效 |
| 兼容性 | 向后兼容 | 不影响未配置应用的正常行为 |

## 边界与约束

- **存储位置：** `/data/service/el1/public/ability_manager/ability_manager_service.db`
- **表名：** `resident_on_demand`
- **不包含：** 运行时动态启停进程、跨设备配置同步

## 示例

### 正例1：启用按需启动

```
命令: aa set-on-demand -b com.example.resident.app -e 1

预期输出:
set on-demand startup for com.example.resident.app successfully
```

### 正例2：禁用按需启动

```
命令: aa set-on-demand -b com.example.resident.app -e 0

预期输出:
set on-demand startup for com.example.resident.app successfully
```

### 反例1：无效参数

```
命令: aa set-on-demand -b com.example.app -e 2

预期输出:
error: invalid enable value, must be 0 or 1
```

### 反例2：空包名

```
命令: aa set-on-demand -b "" -e 1

预期输出:
error: bundle name cannot be empty
```

### 反例3：权限不足

```
命令: (普通应用执行) aa set-on-demand -b com.example.app -e 1

预期输出:
error: permission denied
```

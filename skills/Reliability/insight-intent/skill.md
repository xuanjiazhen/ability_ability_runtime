---
name: insight-intent-reliability
description: InsightIntent可靠性skill。在InsightIntent（意图执行管理）代码开发或设计阶段，自动提醒意图数据库读写超时、执行记录限制、IPC回传数据大小等可靠性关注点。触发场景：InsightIntent代码开发、意图执行管理代码、意图数据库读写、insight_intent目录代码修改。
allowed-tools: [Read, Glob, Grep, Bash, Write, Task, question]
metadata:
  author: l00348536
  version: 1.0.0
  tags: [reliability, insight-intent, intent, database, resource-limit]
---

# InsightIntent可靠性skill

## 使用场景

当用户修改InsightIntent相关代码时使用此skill：

- 意图执行管理
- 意图数据库读写
- 意图执行记录管理
- 意图执行结果IPC回传

## 代码路径

| 代码路径 | 说明 |
| --- | --- |
| E:\ability\ability_ability_runtime-master\...\services\abilitymgr\src\insight_intent\ | AMS侧InsightIntent实现 |
| E:\ability\ability_ability_runtime-master\...\interfaces\inner_api\insight_intent\ | InsightIntent inner API |
| E:\ability\ability_ability_runtime-master\...\frameworks\native\insight_intent\ | InsightIntent native框架 |
| insight_intent_execute_manager.cpp | 执行管理 |
| insight_intent_info_for_query.cpp | 查询信息 |

## 故障模式（1条）

### FM-Intent-01: 开机读写意图数据库超时（I级 - 设备重启）

| 字段 | 内容 |
|---|---|
| 故障模式 | D-06 大数据量操作超时/卡顿 |
| 描述 | 开机读写意图数据库 |
| 严酷度 | I |
| 引发原因 | 意图数据库数据量大，开机时同步读写导致超时 |
| 影响 | 设备重启 |
| 关注代码 | insight_intent目录下数据库操作 |

**开发关注点**：
- 开机时意图数据库操作需异步化或延迟执行
- 数据库读写需有超时保护
- 大数据量操作需分批处理
- 数据库操作不可阻塞开机启动流程

## 资源限制规格

| 限制项 | 规格值 | 超限影响 | 代码位置 |
|---|---|---|---|
| 执行记录数 | 256（INSIGHT_INTENT_EXECUTE_RECORDS_MAX_SIZE） | 超过后新Intent执行请求被拒 | insight_intent_execute_manager.cpp:43 |
| IPC回传数据大小 | 100MB（MAX_IPC_REWDATA_SIZE） | 超过后反序列化失败 | insight_intent_info_for_query.cpp:29 |

## 关键可靠性关注点

### 1. 开机数据库读写超时
- 开机同步读写意图数据库可能导致设备重启
- 开发关注：数据库操作需异步化；需有超时保护

### 2. 执行记录限制
- 256条执行记录限制，超限后新请求被拒
- 开发关注：需有记录淘汰机制（LRU）；被拒时需有明确错误提示

### 3. IPC回传数据大小
- 100MB限制，超限后反序列化失败
- 开发关注：大数据回传需走其他通道（如fd/Ashmem）

## 开发检查清单

修改InsightIntent代码时，检查以下项：

- [ ] 开机时数据库操作是否异步化或延迟执行
- [ ] 数据库读写是否有超时保护
- [ ] 执行记录是否在256限制内
- [ ] 执行记录是否有淘汰机制
- [ ] IPC回传数据是否在100MB限制内
- [ ] 大数据回传是否走其他通道
- [ ] 数据库操作是否阻塞开机启动流程

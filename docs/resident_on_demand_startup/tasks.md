# 任务清单

> 基于 proposal.md、design.md、spec.md 拆分的可执行任务。

## 关联文档

- 需求规格: [proposal.md](./proposal.md)
- 技术设计: [design.md](./design.md)
- 特性规格: [spec.md](./spec.md)

---

## Phase 1: 数据层实现

- [x] **TASK-1** 实现 ResidentOnDemandRdb 数据管理类
  - 关联需求: US-2, US-3
  - 验收标准: Init、GetOnDemandStartup、SetOnDemandStartup、SetOnDemandStartupBatch 方法实现

- [x] **TASK-2** 更新 abilitymgr.gni 构建配置
  - 关联需求: 全局
  - 验收标准: resident_on_demand_rdb.cpp 加入构建列表

## Phase 2: IPC 接口实现

- [x] **TASK-3** 添加 IPC 接口码和接口声明
  - 关联需求: US-1
  - 验收标准: SET_RESIDENT_ON_DEMAND_STARTUP = 83, SET_RESIDENT_ON_DEMAND_STARTUP_BATCH = 84

- [x] **TASK-4** 实现 Proxy 和 Stub 方法
  - 关联需求: US-1
  - 验收标准: IPC 请求正确封装和解析

- [x] **TASK-5** 实现 AbilityManagerClient 方法
  - 关联需求: US-1
  - 验收标准: Client 正确调用 Proxy

## Phase 3: 服务层实现

- [x] **TASK-6** 实现 AbilityManagerService 服务方法
  - 关联需求: US-1 (AC-1.1 ~ AC-1.4), US-3
  - 验收标准: 权限校验、配置保存

- [x] **TASK-7** 修改 ResidentProcessManager 启动检查
  - 关联需求: US-2 (AC-2.1 ~ AC-2.3)
  - 验收标准: 按需启动配置生效

## Phase 4: 命令行工具实现

- [x] **TASK-8** 添加 set-on-demand 命令声明
  - 关联需求: US-1
  - 验收标准: 命令声明和选项定义

- [x] **TASK-9** 实现 set-on-demand 命令逻辑
  - 关联需求: US-1 (AC-1.1 ~ AC-1.3)
  - 验收标准: 命令执行成功，参数校验正确

## Phase 5: 测试

- [x] **TASK-10** 编写 ResidentOnDemandRdb 单元测试
  - 关联需求: US-2, US-3
  - 验收标准: 测试通过

## Phase 6: 验证

- [x] **TASK-11** 编译验证
  - 验收标准: 编译成功，无错误

- [ ] **TASK-12** 功能测试
  - 验收标准: 命令执行成功

- [ ] **TASK-13** 集成测试
  - 验收标准: 系统启动行为正确

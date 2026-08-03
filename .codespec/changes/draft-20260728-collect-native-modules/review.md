# Review Gate

> 汇总 GA / GB / GC 三类门禁与实现符合性、代码质量、最终验证结论。
> 逐 AC 机器真相源见 `evidence/reviews/spec-compliance.md`。

## 审查元数据

| 项 | 内容 |
|----|------|
| Review ID | REV-draft-20260728-01 |
| 审查阶段 | GA / GB / 实现符合性 / 代码质量（GC 待验证证据） |
| 关联文档 | proposal.md / spec.md / design.md / execution-plan.md |
| 复杂度 | 标准 |
| 涉及仓 | ability_runtime、arkui/napi |
| Reviewer | AI 审查（ohos-review 纪律），待 yangzk 确认 |
| 日期 | 2026-07-29 |
| Base SHA | b9e091d743（plan）/ napi 侧基线 1e6702b3 |
| Head SHA | a4a7cdac01（ability_runtime）/ 33cffc11（arkui/napi） |

## 审查输入

| 输入 | 路径 | 说明 |
|------|------|------|
| Requirement | `proposal.md` | 基线 v1.0（2026-07-28 冻结） |
| Design | `design.md` | Approved |
| Spec | `spec.md` | Approved |
| Plan | `execution-plan.md` | ReadyForReview |
| Diff | 本仓 8 个代码提交 + napi 仓 1 个提交 | manifest.commits 登记 |

---

## 零、GA Proposal Gate

| 检查项 | 结论 | 证据 |
|--------|------|------|
| 目标、非目标、成功标准清晰 | PASS | proposal.md 三章（Q-1~Q-6 全部关闭） |
| P0/P1 AC 可测试 | PASS | US-1~US-4 + AC-1~AC-10（WHEN/THEN） |
| 不涉及项与约束已确认 | PASS | 不涉及项确认表（IPC/国际化 N/A 等） |
| target_release / profile / Owner 基线明确 | PASS | OpenHarmony-7.1-Release committed；profile none；Owner yangzk |

**审批决策：** GA 通过（2026-07-28 已冻结，证据 check-proposal.md）

---

## 一、GB Design Baseline Gate

| 检查项 | 结论 | 证据 |
|--------|------|------|
| 设计决策已记录并有取舍理由 | PASS | design.md ADR-1~6（各含 2 备选 + 取舍） |
| Spec 规则覆盖全部 P0/P1 AC | PASS | spec R-1~R-14 ↔ AC 交叉一致（validate Level C 通过） |
| 异常规则无误伤风险 | PASS | R-13/R-14 失败路径均不阻断启动（架构约束） |
| 恢复规则覆盖完整 | PASS | R-7/R-8 四路径冷启动重建（触发→路径→结果完整） |
| 不涉及项已显式确认 | PASS | proposal 不涉及项表 + design 承接表 |

**审批决策：** GB 通过（证据 check-spec.md / check-design.md / check-execution-plan.md）

---

## 二、实现草稿规范符合性审查

逐 AC 结论与证据：**`evidence/reviews/spec-compliance.md`**（16 条 AC 全部 PASS，无"多余/误解"行为级偏差）。

实现形态相对 design 的三处微调（均已记录 evidence，不改行为语义）：
1. collector 保持 napi 无关，新增 NativeModuleSampler 承载采集（可测试性）；
2. preload 快照由 runtime 层持有 + MainThread 桥接（避免 runtime→appkit 循环依赖）；
3. 消费任务经 MainThread::RunNativeStartupTask 原生任务表执行（覆盖无 startup 配置应用）。

### 理解偏差

| 检查项 | 结论 | 证据 |
|--------|------|------|
| AC 理解是否正确 | PASS | spec-compliance.md 逐条对应；D-1（Runtime 两个静态 InnerAPI 未列 spec 清单）已记录接受 |
| 边界和不做范围是否遵守 | PASS | 无 Public/System API；未改加载路径；应用模块预加载未做（范围外） |
| 适用规则是否遵守 | PASS | HiLog/HisyseventReport/EventReport 仓惯例一致 |

---

## 三、代码质量审查

### 架构与分层合规

| 调用方 | 被调用方 | 合规？ | 说明 |
|--------|----------|--------|------|
| appkit(app/application_impl/main_thread) | runtime（Runtime::GetPreloadModuleKeys） | 合规 | 既有依赖方向 |
| appkit(app/native_module_sampler) | napi（NativeModuleManager） | 合规 | appkit 既有 napi 依赖 |
| appkit(app_startup/predict_task) | appkit(app/collector) + napi | 合规 | 同 target 内调用 |
| runtime(ets_runtime) | runtime(runtime.cpp 静态） | 合规 | 同 target |
| collector | napi | 无调用 | 刻意解耦（UT 轻量） |

- [x] 无应用层直接调用系统服务层
- [x] 无框架层直接调用内核接口（仅标准文件 I/O）
- [x] 无循环依赖（新增 include_dirs app/ 已核对无重名头文件）

### API 与子系统边界

| 检查项 | 结果 | 说明 |
|--------|------|------|
| 子系统间通过明确接口调用 | 通过 | ability_runtime→napi 仅 GetLoadedModuleInfos（InnerAPI） |
| 跨子系统依赖在 bundle.json 中声明 | 通过 | napi 为既有依赖，无新增 |
| 无子系统间循环依赖 | 通过 | 单向 |

| API 签名 | 类型 | 命名合规 | 参数合规 | d.ts 位置 |
|----------|------|----------|----------|-----------|
| NativeModuleManager::GetLoadedModuleInfos | InnerApi | 通过 | 通过（出参先清空） | 无（C++ 内部） |
| NativeModuleInfo | InnerApi | 通过 | 通过 | 无 |
| Runtime::CapturePreloadModules/GetPreloadModuleKeys | InnerApi | 通过 | 通过 | 无（D-1：spec 清单待补录） |

### Owner/Committer 视角

| 检查项 | 结论 | 证据 |
|--------|------|------|
| 模块边界是否合适 | PASS | collector（纯逻辑）/sampler（napi 粘合）/task（消费）三层清晰 |
| 抽象层次是否合理 | PASS | 无过度抽象；静态工具类符合仓内 dump_runtime_helper 同类风格 |
| 是否符合仓内既有模式 | PASS | 启动任务/事件上报/参数读取均复用既有模式 |
| 是否引入难维护结构 | PASS | 状态集中 g_state + 锁，语义简单 |

### 工程质量检查

| 检查项 | 结论 | 证据 |
|--------|------|------|
| 架构/分层规则 | PASS | 见上 |
| API/兼容性规则 | PASS | 文件 version 化 + 缺省关闭 |
| 构建与部件规则 | PASS | 仅 sources/include_dirs 增量；无 bundle.json 变更；曾修复 group(unittest) 缺失 |
| 静态质量与风格 | PASS | 命名/注释/版权头符合仓风格 |
| 测试质量与可测试性 | PASS（Review-1 已修） | 夹具 SetUp/TearDown 统一初始化被测状态 |
| 性能/安全/并发风险 | PASS | 采样主线程 <1ms、写盘非主线程至多 1 次、rename 原子、0600 |
| 多余实现或过度抽象 | PASS | spec-compliance.md 多余实现核查 |

---

## 四、GC Final Delivery Gate

| 检查项 | 结论 | 证据 |
|--------|------|------|
| 验证命令已真实执行且证据新鲜 | **FAIL** | Phase-3/4 未编译；22 组 UT 未执行（OI-4） |
| AC 已逐条闭环 | 部分 | 代码级闭环；验证级待 OI-4 |
| 回归范围已覆盖或明确 N/A | PASS（声明） | 开关缺省关闭 → 存量零影响（需编译+存量 UT 佐证） |
| 兼容性与发布风险已确认 | PASS | spec 兼容性声明；文件 version 化 |
| Open Issues 已处理或明确接受 | 部分 | OI-1/2/3 follow-up 接受；OI-4 必须回补 |
| Profile Spec for Test | N/A | profile = none |

**审批决策：** GC **不通过（阻塞项 OI-4）**——编译 + UT 执行 + 上机实测回补后重审本区。

---

## 五、纠正循环

| 轮次 | 结论 | 处理动作 | 复检范围 |
|------|------|----------|----------|
| Review-1 | ChangesRequested | 已修：UT 夹具统一初始化；遗留 OI-4（验证证据）非实现偏差 | 编译 + UT + 实测 |

## 六、Open Issues

| 类型 | 问题 | 处理方式 | Owner |
|------|------|----------|-------|
| blocker | OI-4 Phase-3/4 编译/UT/实测缺失 | 必须回补后重审 GC | yangzk |
| ~~risk~~ | ~~OI-1 纯 JS 运行时路径 preload 排除未覆盖~~ | **已修复**（5dd3d49fb1，JsRuntime::FinishPreload 补 CapturePreloadModules） | yangzk |
| risk | OI-2 refCount 语义待设备确认（降级方案在案） | follow-up 上机确认 | yangzk + napi owner |
| risk | OI-3 CCM 参数命名未与 owner 确认 | follow-up 评审定名 | yangzk |
| follow-up | D-1 spec API 清单补录 Runtime 两个静态 InnerAPI | spec 下一版本补录 | yangzk |

## 七、审查决策

| 项 | 内容 |
|----|------|
| Decision | **ChangesRequested**（规范符合性通过；GC 阻塞于验证证据 OI-4） |
| 下一阶段 | 编译验证 → UT 执行 → 上机实测 → 重审 GC → ohos-validate |
| Recheck Scope | Phase-3/4 编译结果、22 组 UT 执行结果、写盘次数/preload 排除/预加载执行/命中率实测 |

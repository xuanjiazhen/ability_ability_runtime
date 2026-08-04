# 规格符合性审查（spec-compliance）

> Level D 机器真相源：逐 AC 合规结论 + 证据。
> 审查人：AI（按 ohos-review 纪律），Reviewer 确认人：yangzk（待确认）
> Base SHA：b9e091d743（plan 提交）　Head SHA：a4a7cdac01（TASK-8+9）

| AC | 实现位置（证据） | 用例 | 结论 | 验证状态 |
|----|------------------|------|------|----------|
| AC-1.1 启动阶段合并写盘，每轮 ≤1 次 | application_impl.cpp PerformAppReady（检查点①）；main_thread.cpp TriggerNativeModuleCollect/RunNativeModuleCollect（检查点②，延迟 5s/后台兜底）；collector SaveNow saveDispatched 至多一次 | SaveNow_AtMostOnce_012、SaveNow_AsyncDispatched_013 | PASS（实现精确对应） | **UT + 设备实测均通过**（阶段 1，文件生成/字段/每轮≤1次确认） |
| AC-1.2 开关缺失缺省关闭零行为 | collector IsEnabled：测试钩子或 GetBoolParameter 缺省 false；检查点两处先判 IsEnabled | IsEnabled_DefaultDisabledNoFileIO_005、Params_DefaultsWhenAbsent_020 | PASS | UT 通过；设备侧阶段 6 待补 |
| AC-1.3 多进程独立文件 | GetStorePath 含 ProcessNameHash（FNV-1a 8 hex） | ProcessNameHash_StableAndDistinct_002 | PASS | **UT + 设备实测通过**（阶段 1，多进程文件名隔离） |
| AC-1.4 写盘失败无残缺文件 | WriteAtomic：tmp + rename，失败清理 tmp | WriteAtomic_SuccessAndFailure_010 | PASS | UT 通过；设备侧写盘失败注入待补 |
| AC-1.5 fork 前 preload 模块不采集 | Runtime::CapturePreloadModules（ETSRuntime::PreFork 末尾 **及** JsRuntime::FinishPreload 末尾）；MainThread 桥接 SetPreloadModuleKeys；AccumulateModules 过滤 | AccumulateModules_DedupAndPreloadFilter_011 | **PASS（ETS + 纯 JS 双路径）** | **UT + 设备实测通过**（阶段 2，/proc/self/maps 比对） |
| AC-2.1 连续命中 score 收敛入预测集 | MergeRound 命中奖励 α+(1−α)·score | MergeRound_HitReward_002、MergeAndSave_RoundTrip_008 | PASS | **UT + 设备实测通过**（阶段 3，score 趋近 1、predicted 稳定） |
| AC-2.2 16 轮未见或 score<0.05 淘汰 | EvictStats 双条件 + 1024 上限低分淘汰 | EvictStats_LowScoreAndLongUnseen_005、EvictStats_CapLimit_006 | PASS（UT）；设备 DEFERRED | UT 通过；设备侧淘汰场景待 app 就绪后补（见 OI-5） |
| AC-2.3 损坏/version 不符/缓存清理重建 | LoadStats 严格校验 + 类型混淆兜底；MergeAndSave 冷启动路径 | LoadStats_RejectInvalid_007、MergeAndSave_RoundTrip_008 | PASS | 用例已交付未执行 |
| AC-2.4 应用升级清空重建 | MergeAndSave appVersionCode 比对重建 | MergeAndSave_AppUpgradeRebuild_009 | PASS | 用例已交付未执行 |
| AC-2.5 消费方预加载不计命中 | MergeRound consumerPreloads 分支：loadCount 增量 ≤ 预加载次数则仅刷新观测计数、不奖励；冷启动下一律不计命中（阻断自反馈） | MergeRound_ConsumerPreloadOnly_016、_PlusImport_017、_NotConsumerPreloaded_018、_ColdStartConsumerPreloadNotHit_023、SaveNow_ConsumerPreloadNotPredicted_019 | PASS；见 D-2（R-9"不衰减"措辞收紧）与 OI-2（refCount 语义待设备确认） | 用例已交付，UT 执行中修正 016 期望 |
| AC-3.1 启动框架预加载 predicted 系统模块 | PredictPreloadSoStartupTask（LoadPredictedSystemModules 过滤 + 逐模块 LoadNativeModule）；MainThread::RunNativeStartupTask 注册；RunTaskInit 将循环 ffrt::submit 到 worker（见 design ADR-5b，修正主线程执行偏差） | LoadPredictedSystemModules_Filter_014 | PASS（实现对应） | 任务执行实测待补；阶段 4 判定含 ffrt tid 核查 |
| AC-3.2 文件缺失/损坏跳过且不影响启动 | LoadPredictedSystemModules 返回空；任务早退 ERR_OK | LoadPredictedSystemModules_MissingOrCorrupted_015 | PASS | 用例已交付未执行 |
| AC-3.3 单模块失败记事件继续 | 任务循环内失败记 STARTUP_TASK_ERROR 事件 continue | （任务执行侧，待设备） | PASS（实现对应） | 实测待补 |
| AC-3.4 空预测集不动作 | 任务 modules.empty() 早退 | LoadPredictedSystemModules_Filter_014（空集路径含于 015） | PASS | 用例已交付未执行 |
| AC-4.1 隐藏目录 + 系统命名空间命名 | GetStorePath：.ohos.ability_runtime/native_modules_<hash8>.v2.json | GetStorePath_Format_001 | PASS | 用例已交付未执行 |
| AC-4.2 0600 + 无隐私数据 | WriteFileWithPermission fchmod 0600；JSON 仅模块名/路径/版本/计数 | WriteFileWithPermission_ContentAndMode_004 | PASS | 用例已交付未执行 |

## 多余实现核查

| 实现内容 | 是否在 Spec/Plan 中 | 结论 |
|----------|---------------------|------|
| NativeModuleSampler（napi 采集独立类） | design 模块骨架范畴（实现形态微调，已记录 evidence） | 保留：无新增行为，为可测试性拆分 |
| 测试钩子（SetEnabledForTest/SetAsyncSaveForTest/ResetStateForTest/ResetParamCacheForTest/GetSampled*ForTest） | 可测试性设计（design 测试性设计表） | 保留：仅测试用途，不改变生产行为 |
| Runtime::CapturePreloadModules/GetPreloadModuleKeys（InnerAPI） | AC-1.5 必需（spec API 变更清单外新增——见偏差 D-1） | 保留：属 spec 未列的配套 InnerAPI，已记录 |
| 命中率上报（report_hitrate） | spec 非功能-自动化维测 | 符合 |

## 偏差与 Open Issues

| 编号 | 类型 | 内容 | 处理 |
|------|------|------|------|
| D-1 | spec 偏差 | Runtime 新增两个静态 InnerAPI 未列入 spec API 变更清单（spec 仅列 napi 侧两个） | 接受：同为 InnerAPI 同语义层级，建议 spec 下一版本补录 |
| D-2 | spec 措辞收紧 | R-9 写"消费预加载模块不按遗漏衰减"，但实现让其参与遗漏衰减。原因：若不衰减，被消费方持续预加载、应用不再 import 的模块会永久停留在预测集（score 不变且 >0.05 不被淘汰），自反馈无法终止。AC-2.5（更高优先级）仅要求"评分不得上升"，实现满足。建议 spec 下一版本将 R-9 改为"不计命中、按未真实使用衰减直至淘汰"。 | 接受：以 AC-2.5 与防自反馈目标为准，UT 016 已据此修正 |
| OI-1 | risk | ~~纯 JS 运行时路径（不经 ETSRuntime::PreFork）preload 快照不生效~~ | **已修复**（5dd3d49fb1）：JsRuntime::FinishPreload 补 CapturePreloadModules，ETS+JS 双路径覆盖 |
| OI-2 | risk | refCount cache 命中递增语义未在设备确认（RSK-2），若不符则 AC-2.5 精度降级（降级方案：P 集合模块本轮一律不更新） | follow-up：上机验证时确认 |
| OI-3 | risk | CCM 参数命名未与 owner 确认（RSK-3），当前按 ccm.ability.native_module_collect.* 实现 | follow-up：评审时定名 |
| OI-4 | blocker | Phase-3/4 代码未编译验证、22 组 UT 未执行、上机实测未做 | **部分解除**：编译通过 + UT 执行通过 + 阶段 0~3 设备实测通过；阶段 4~8 与淘汰设备回归待续 |
| OI-5 | follow-up | 淘汰验证（AC-2.2 设备侧）当前测试 app 无法构造"仅前 N 轮加载"场景 | UT 已覆盖淘汰逻辑（EvictStats_* 三例）；设备侧回归待后续测试 app 就绪后补 |
| OI-6 | follow-up | PredictPreloadSoStartupTask（consumer 侧）零 UT（M-2） | 核心数据逻辑（LoadPredictedSystemModules/RecordConsumerPreload）已有 UT；RunTaskInit 胶水层（IsEnabled/空集早退/ffrt submit）依赖 app_startup+ffrt+napi 链，单测搭建成本高；端到端待设备验证 + 后续补胶水层 UT |
| OI-7 | follow-up | ffrt 预加载循环异常静默丢失（m-3） | -fno-exceptions 下无 try；建议每模块 LoadNativeModule 前后计数兜底，未达 modules.size() 时聚合 EventReport |
| OI-8 | follow-up | napi moduleRelativePath_ map 只写不删 + relativePath 无长度上限（m-4，跨仓） | 键维度有界（模块名）风险低；建议写入点移到加载成功后、Unload 时 erase、relativePath 加长度上限，与 napi owner 对齐 |

## 纠正循环

| 轮次 | 结论 | 处理动作 | 复检范围 |
|------|------|----------|----------|
| Review-1 | ChangesRequested | 修复：UT 夹具 SetUp/TearDown 统一初始化被测状态（已修）；其余为验证证据缺失（OI-4），非实现偏差 | 编译 + UT 执行 + 实测 |

---

## 附：维护者处置回执（Round 2，2026-11-05，基于修复后工作区）

对外部检视报告 8 项发现的逐条处置：

| 报告项 | 处置 | 证据 |
|--------|------|------|
| LOG-01 | **已修复** | 前台计数改 pid 维度 + TERMINATED/END 计负（计数器 ≤0 保护，正常生命周期重复计负安全）；检视报告"死亡路径仅派发 TERMINATED"的前提经代码证实（app_mgr_service_inner.cpp:6525-6526），修复精确闭环该路径 |
| LOG-02 | **已修复** | ProcessSchedule 增加 (taskId,pid) 引用计数与豁免计数镜像（add 递增/remove 递减/归零移除）；死亡路径整键清除豁免；新增 process_schedule 与服务层双层回归用例 |
| API-01 | **可关闭（证据补齐）** | 【代码】interface_sdk-js `OpenHarmony-7.0-Release` 分支 ls-tree 查无 `api/@ohos.app.agent.taskManager.d.ts`——该 API 未随任何已发布 SDK 流出，变更发生在未发布版本内（@since 26.1.0），无在役调用方；PR !20914 描述已声明兼容性结论，跨仓同步 PR !37176 已发起 |
| LOG-03 | **已处置** | spec R-4 / design ADR-3 对齐：注明类型维度随 20260914692272 落地，当前单类型按 taskId+pid 计数、dump 以 CPU 占位标签渲染；代码注释同步标注 |
| LOG-04 | **已修复** | 仓根 `20260914692273/` 目录已从分支移除（本地备份 /tmp/opencode/backup，git 历史可溯）；SDD 文档统一归 `.codespec/changes/20260914692273/` |
| LOG-05 | **已修复** | `RegisterScheduleListeners` 移除 `std::call_once`：负载监听经 judge 幂等守卫随任务创建重试（覆盖 ressched 晚启动）；observer 注册失败在下一任务创建时重试 |
| LOG-06 | **已修复** | 6 处测试 BUILD.gn 新增行缩进统一为 4 空格 |
| LOG-07 | **已修复** | `OnSystemLoadLevelChanged` 未使用参数改为匿名参数并注释 |
| API-02 | **已修复** | NAPI 报错信息改为 "task priority is judged by the system now; remove the extra parameter" |

**验证**：`task_manager_service_test` / `process_schedule_test` / 全量 `ability_runtime_test`（rk3568, use_thin_lto=false）全部 build success（2026-11-05 22:52:14）。单测执行证据仍待设备环境。


---

## Round 2 处置回执（2026-11-06，针对 R1~R24 报告）

| 报告项 | 处置 | 证据 |
|--------|------|------|
| R1（观察者注册身份错位） | **已修复** | `IN_PROCESS_CALL` 包裹注册（in_process_call_wrapper.h 宏，ResetCallingIdentity→服务自身身份→SetCallingIdentity）；失败重试保留（LOG-05 修复） |
| R2（前台无初始对账） | **已修复** | `SetForegroundSeeder` 接线（app_mgr_service.cpp OnStart 注入 `GetForegroundApplications` 查询）；注册成功后 `SeedForegroundState` 回填 judge 计数 |
| R3（记录变更与投递非原子） | **已修复** | ProcessSchedule 全部事件投递移入 `tasksMutex_` 锁内（Register/Unregister/Finish/Rejudge/死亡路径），锁序固化 `tasksMutex_→scheduleMutex_`；scheduleMutex_ 保持叶锁 |
| R4（SetLoadProvider 不注销旧 provider） | **已修复** | 切换时旧 provider `UnregisterListener()`（锁外执行）；原测试断言恢复有效 |
| R5/R6（IPC/API 兼容性前提） | **已关闭** | spec 全文 @since 统一为 26.1.0（消灭 26.2.0 残留）；未发布版本内变更证据同 Round 1（7.0-Release 无该 API） |
| R7（双层记账粒度） | **已修复** | F01 引用计数（process_schedule taskRefs）已对齐 task_mgr attributionId 粒度 |
| R8（前台计数卡正） | **已缓解** | TERMINATED/END 计负（F03/F05 修复）；reviewer 未决问题 3（AMS 死亡补发）经代码核实：HandleAbilityDied 派发 ABILITY_STATE_TERMINATED（app_mgr_service_inner.cpp:6525-6526），闭环 |
| R9（豁免四方不一致） | **已处置** | spec R-4/design ADR-3 注明类型维度随 20260914692272 落地；process_schedule.h 注释与 dump 占位标签同步标注 |
| R10（ressched 火忘+不重试） | **已修复** | 注册失败重试（去 call_once）；火忘为 RSS 接口 void 限制，已文档化（获取途径：RSS 返回码需求） |
| R11（dump 口径分裂） | **已修复** | `--task` 豁免摘要填真值（`DumpExemptionSummary` 共享渲染）；测试基线 `total=0`→`total=1` 修正 |
| R12（startTask/finishTask 策略不一致） | **已修复** | OnStartTask 参数解析对齐 OnFinishTask（多余入参忽略，NAPI 层） |
| R13（JS/ArkTS 能力面） | **记录** | d.ts 契约为 Promise；NAPI callback 模式为 napi 惯例（on-finish 同款），随 spec 契约为准 |
| R14（provider 持锁 IPC） | **已修复** | RegisterListener 的 IPC 调用与种子回调移至 `mutex_` 锁外 |
| R15（静态析构顺序） | **按用户决策暂不处理** | 记录为已知限制（AppMS 常驻 SA，触发面极低） |
| R16（map 导出） | **已修复** | libappms.map 还原（不导出）；9 个直接编译 app_mgr_service.cpp 的测试 target 自包含编译新源文件（moduletest/common/ams + 8 个 unittest） |
| R17（taskId 控制字符） | **缓办（P3）** | 威胁面=系统应用；建议后续统一加可打印字符白名单（跨 20260914692191 协议面，需单独评估） |
| R18（dump 无截断） | **缓办（P3）** | 上界 10 任务×1000 归因≈数百 KB，可接受；后续可加行数上限 |
| R19（proxy.h 注释） | **已修复** | StartTask/FinishTask Doxygen 注释补齐 |
| R20（未使用参数） | **已修复** | 匿名参数+注释 |
| R21（DumpPsched 忽略参数） | **已修复** | F07 收编为 `--task psched` 子命令，多余参数报错与 register/unregister 同口径 |
| R22（spec 引用过时） | **已修复** | CheckParam 行号更新为 :643 |
| R23（绑定层零单测） | **记录** | NAPI/ANI 需 JS 运行时环境，列为已知限制 |
| R24（NAPI/ANI 错误传递差异） | **记录** | NAPI 同步 throw 与 finishTask 同款；ANI 异步 reject 为 ANI 惯例；BusinessError.code 一致 |

**验证**：全量 `ability_runtime_test`（rk3568, use_thin_lto=false）build success（2026-11-06 12:20:04）。
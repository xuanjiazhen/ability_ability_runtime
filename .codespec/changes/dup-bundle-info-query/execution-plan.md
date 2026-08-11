# 执行计划：应用启动链路包信息重复查询/存储治理（一期）

> 将 Approved Spec 拆成可独立执行、可验证、可审查的 Task。

## Plan 元数据

| 字段 | 内容 |
|------|------|
| Plan ID | PLAN-DUP-BUNDLEINFO |
| 关联 Feature/Bug | FEAT-DUP-BUNDLEINFO |
| 关联文档 | proposal.md / spec.md / design.md（同目录） |
| 复杂度 | 标准 |
| 状态 | Draft |
| Owner | 待定 |

## 输入状态

| 输入 | 路径 | 要求状态 |
|------|------|----------|
| Requirement | `.codespec/changes/dup-bundle-info-query/proposal.md` | Approved（基线 v1.0） |
| Design | `.codespec/changes/dup-bundle-info-query/design.md` | Approved（结论：通过，待 Owner 评审） |
| Spec | `.codespec/changes/dup-bundle-info-query/spec.md` | Approved |

## 受影响文件全量清单

| 仓 | 层（来自 design.md） | 文件路径 | 修改类型 | 说明 |
|----|---------------------|----------|----------|------|
| ability_runtime | 服务（AMS） | `services/abilitymgr/src/utils/start_ability_utils.cpp` / `include/utils/start_ability_utils.h` | 修改 | thread_local 缓存优先查询入口扩展（abilityInfo 缓存版）、CheckAppProvisionMode 结果写回 |
| ability_runtime | 服务（AMS） | `services/abilitymgr/include/utils/ability_util.h` | 修改 | IsStartIncludeAtomicService 改走缓存优先入口；caller 包名复用 |
| ability_runtime | 服务（AMS） | `services/abilitymgr/src/interceptor/screen_unlock_interceptor.cpp`、`services/abilitymgr/src/interceptor/extension_control_interceptor.cpp` | 修改 | 拦截器命中 thread_local 缓存后再直查 |
| ability_runtime | 服务（AMS） | `services/abilitymgr/src/utils/update_caller_info_util.cpp` | 修改 | UpdateCallerInfo 内 GetNameForUid 结果传递复用 |
| ability_runtime | 服务（AMS） | `services/abilitymgr/src/ability_manager_service.cpp` | 修改 | VerifyPermission 同函数去重；GetCallerUidAndToken 重查询降载为 GetUidByBundleName |
| ability_runtime | 服务（AMS） | `services/abilitymgr/src/ability_record.cpp`、`services/abilitymgr/src/utils/main_element_utils.cpp` | 修改 | mainElement 判断降载 GetBundleInfoV9→GetHapModuleInfo + 失败回退 |
| ability_runtime | 胶水（InnerAPI） | `interfaces/inner_api/app_manager/include/appmgr/app_mgr_interface.h`、`app_mgr_proxy.h/.cpp`（src/）、`app_mgr_stub.h/.cpp`（src/）、`app_mgr_client.h/.cpp`（src/） | 修改 | IsSpecifiedModuleLoaded / IsCorrespondingProcessAttachDebug 追加可选 processName 参数（parcel 末尾） |
| ability_runtime | 服务（AppMgr） | `services/appmgr/include/app_mgr_service.h`、`services/appmgr/src/app_mgr_service.cpp` | 修改 | 两接口签名透传扩展 |
| ability_runtime | 服务（AppMgr） | `services/appmgr/include/app_mgr_service_inner.h`、`services/appmgr/src/app_mgr_service_inner.cpp` | 修改 | 两接口实现优先用传入 processName；CreatNewStartMsg 可选预查参数；UpdateApplicationInfoInstalled 刷新持有值 |
| ability_runtime | 服务（AppMgr） | `services/appmgr/include/module_running_record.h`、`services/appmgr/src/module_running_record.cpp` | 修改 | 持有 shared_ptr<const HapModuleInfo>，GetHapModuleInfo 返回持有副本 |
| ability_runtime | 服务（AppMgr） | `services/appmgr/include/app_running_record.h`、`services/appmgr/src/app_running_record.cpp` | 修改 | 持有 shared_ptr<const BundleInfo>（StartProcess 时设置）+ 刷新接口 |
| ability_runtime | 服务（AMS 调用方） | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp`、`services/abilitymgr/src/extension_record/extension_record_manager.cpp`、`services/abilitymgr/src/ability_connect_manager.cpp` | 修改 | 调用检查类接口时传入已算 processName |
| ability_runtime | 框架（appkit） | `frameworks/native/appkit/app/main_thread.cpp`、`frameworks/native/appkit/app/ohos_application.cpp` 及对应头文件 | 修改 | 持久化 module→HapModuleInfo 映射与 hspList；ApplicationInfo 单一 shared_ptr 共享；CheckAndUpdateRuntime 复用 |
| ability_runtime | 框架（appkit） | `frameworks/native/appkit/ability_runtime/context/context_impl.cpp` 及头文件 | 修改 | InitHapModuleInfo 先查应用级映射，未命中回退 IPC；ApplicationInfo 共享指针 |
| ability_runtime | 框架（runtime） | `frameworks/native/runtime/js_runtime.cpp`、`frameworks/native/runtime/js_module_reader.cpp/h`、`frameworks/native/runtime/hybrid_js_module_reader.cpp` | 修改 | 模块路径映射注入 reader，命中免查 BMS |
| ability_runtime | 胶水（框架公共） | `frameworks/native/appkit/ability_bundle_manager_helper/bundle_mgr_helper.cpp/h` | 修改 | 查询计数打点（DEBUG 级开关） |
| ability_runtime | 测试 | `test/unittest/ability_interceptor_test/`、`ability_jump_interceptor_test/`、`ability_record_test/`、`app_mgr_service_inner_mock_test/`、`ams_app_running_record_test/`、`ui_ability_lifecycle_manager_test/`、`appkit/main_thread_test/`、`appkit/ohos_application_test/`、`ability_stage_context_test/` | 修改 | 各 Task 对应单测扩展（mock BMS 计数） |

**检查项：**
- [x] design.md 调用链每一层（服务 AMS/胶水/服务 AppMgr/框架 appkit/框架 runtime）都有对应文件
- [x] 每个文件修改类型和职责说明明确
- [x] 无未映射层

## AC 到 Task 追溯

| AC | 来源 | Task | 验证方式 | 覆盖？ |
|----|------|------|----------|--------|
| AC-1 | spec.md | （已完成：proposal.md 附录清单 R1~R12/S1~S8，每条附文件:行号） | 代码审查 | 是 |
| AC-2 | spec.md | TASK-3、TASK-4 | 单测（mock BMS 计数） | 是 |
| AC-3 | spec.md | TASK-2 | 单测 | 是 |
| AC-4 | spec.md | TASK-5、TASK-6 | 单测 + 打点计数 | 是 |
| AC-5 | spec.md | TASK-5 | 单测 | 是 |
| AC-6 | spec.md | TASK-3 | 单测（校验结果比对） | 是 |
| AC-7 | spec.md | TASK-9 | 全量 UT | 是 |
| AC-8 | spec.md | TASK-2~TASK-8（各回退路径）+ TASK-9 汇总验证 | 故障注入单测 | 是 |
| AC-9 | spec.md | TASK-6、TASK-9 | 分身/沙箱/插件参数组合单测 | 是 |
| AC-10 | spec.md | TASK-1 | hilog 打点统计 | 是 |
| AC-11 | spec.md | TASK-7、TASK-8 | 单测 + 打点计数 | 是 |
| AC-12 | spec.md | TASK-7 | 代码审查（同一智能指针）+ 单测 | 是 |
| AC-13 | spec.md | TASK-7 | 单测（混合 arkTSMode） | 是 |

## 首批实现边界

**首批必须实现：** TASK-1（打点，量化基线先行）、TASK-2/3/4（AMS 侧去重）、TASK-5（AppMgr 持有复用）、TASK-7（应用进程复用）——主链 P0 AC 全部闭合
**可后置：** TASK-6（InnerAPI 扩展，A-4）、TASK-8（runtime 路径复用）
**不建议延后：** TASK-1（无打点则所有量化 AC 无法验证）、TASK-9（回归门禁）

## 阶段计划

| 阶段 | 目标 | 关键 Task | 结束门槛 | 最小验证 |
|------|------|-----------|----------|----------|
| Phase-1 | 打点 + AMS 侧去重 | TASK-1/2/3/4 | AMS 侧打点显示拦截器链重复查询消除；既有 UT 全过 | `run -t UT -ts ability_interceptor_test` 等 |
| Phase-2 | AppMgr 持有复用 + InnerAPI 扩展 | TASK-5/6 | 冷启动 AppMgr 侧同 bundle 查询次数降为首次 1 组；InnerAPI 兼容验证 | `run -t UT -ts app_mgr_service_inner_mock_test` 等 |
| Phase-3 | 应用进程复用 | TASK-7/8 | 应用进程启动 BMS 查询仅剩 HandleLaunchApplication 一组 | `run -t UT -tp ability_runtime -ts` appkit 相关套件 |
| Phase-4 | 回归门禁 | TASK-9 | 全量 UT 通过；打点对比报告 | `run -t UT -tp ability_runtime` |

## Task 粒度原则说明

- TASK-2/3/4 同属 AMS 侧但文件与验证闭环不同（拦截器 / caller 与权限链 / LoadAbility 降载），拆分；TASK-7 合并 ADR-8/10/11（同为 main_thread/context_impl/ohos_application 文件组与同一 appkit 测试闭环）
- 每个 Task 独立可编译、独立有单测验证

## 禁止项自检

- [x] 无 TBD/TODO/占位符（Owner 待定属排期信息，非实现占位）
- [x] 无模糊指令
- [x] 跨 Task 依赖显式声明
- [x] 每个 Task 文件范围与验证命令明确
- [x] 无生成文件（无 bridge/IDL/CAPI；InnerAPI 为手写 parcel 代码，非生成物）

## Task 列表

| Task ID | 目标 | 文件范围 | AC 映射 | 前置依赖 | 完成判据 | 验证命令 |
|---------|------|----------|---------|----------|----------|----------|
| TASK-1 | BMS 查询打点基础设施 | bundle_mgr_helper、三处输出点 | AC-10 | 无 | 冷启动三侧各输出一行计数日志 | 编译 + 打点小工具验证 |
| TASK-2 | 拦截器缓存复用 | start_ability_utils、ability_util.h、2 个拦截器 | AC-3、AC-8 | TASK-1 | 拦截判定不变且重复查询消除（单测计数） | `run -t UT -ts ability_interceptor_test` |
| TASK-3 | caller/权限链同函数去重 | update_caller_info_util、start_ability_utils、ability_manager_service | AC-2、AC-6、AC-8 | TASK-1 | 校验结果逐点一致；同函数查询=1 | `run -t UT -ts ability_manager_service_first_test` |
| TASK-4 | mainElement 降载 | ability_record、main_element_utils | AC-2、AC-8 | TASK-1 | 查询载荷降为单模块；失败回退全量 | `run -t UT -ts ability_record_test` |
| TASK-5 | Module/App RunningRecord 持有复用 | module_running_record、app_running_record、app_mgr_service_inner | AC-4、AC-5、AC-8 | TASK-1 | attach 后回查=0；CreatNewStartMsg 复用预查 | `run -t UT -ts ams_app_running_record_test`、`run -t UT -ts app_mgr_service_inner_mock_test` |
| TASK-6 | InnerAPI 检查接口参数扩展 | app_mgr_interface/proxy/stub/client、app_mgr_service(_inner)、3 个 AMS 调用方 | AC-4、AC-8、AC-9 | TASK-5 | 未携带参数行为不变；携带时 0 查询；维度不匹配回退 | `run -t UT -ts ui_ability_lifecycle_manager_test` |
| TASK-7 | 应用进程启动数据复用 | main_thread、ohos_application、context_impl | AC-11、AC-12、AC-13、AC-8 | TASK-1 | 非 entry module 不再 IPC 查 HapModuleInfo；ApplicationInfo 单一指针；混合模式复用 | `run -t UT -ts main_thread_test`、`run -t UT -ts ohos_application_test`、`run -t UT -ts ability_stage_context_test` |
| TASK-8 | runtime 模块路径复用 | js_runtime、js_module_reader、hybrid_js_module_reader | AC-11、AC-8 | TASK-7 | reader 命中映射 0 查询；未命中回退 | `run -t UT -tp ability_runtime`（runtime 相关套件） |
| TASK-9 | 全量回归 + 打点对比报告 | test/unittest（上述套件） | AC-7、AC-8、AC-9、AC-10 | TASK-2~8 | 全量 UT 通过；优化前后查询次数对比记录 | `run -t UT -tp ability_runtime` |

## Task 详情

### TASK-1: BMS 查询打点基础设施

| 字段 | 内容 |
|------|------|
| 任务目标 | 在 BundleMgrHelper 各 Get* 查询入口加进程内原子计数（DEBUG 开关），并在 AMS 启动结束（StartAbilityInfoWrap 析构）、AppMgr LoadAbility 完成、应用进程 HandleLaunchApplication 完成三处各输出一行计数 hilog（TAG_LOGD，含 bundleName） |
| AC 映射 | AC-10 |
| 前置依赖 | 无 |
| 非目标 | 不改变任何查询行为；不做 hisysevent 上报；不覆盖非启动路径的打点展示 |
| 完成判据 | 一次冷启动可在 hilog 中看到 AMS/AppMgr/应用进程三行计数日志；计数本身不引入额外 BMS 查询 |
| 停止条件 | 打点需改动 BundleMgrHelper 接口签名（不允许，仅内部计数）或影响 Release 版性能（开关失效）时停止回传 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Modify | `frameworks/native/appkit/ability_bundle_manager_helper/bundle_mgr_helper.cpp` 及头文件 | 进程内原子计数器 + DEBUG 开关 |
| Modify | `services/abilitymgr/src/utils/start_ability_utils.cpp` | StartAbilityInfoWrap 析构输出 AMS 侧计数 |
| Modify | `services/appmgr/src/app_mgr_service_inner.cpp` | LoadAbility 完成输出 AppMgr 侧计数 |
| Modify | `frameworks/native/appkit/app/main_thread.cpp` | HandleLaunchApplication 完成输出应用进程侧计数 |

**Spec Context**：AC-10（WHEN 冷启动完成 THEN 打点日志可统计本次启动 BMS 同步查询次数，优化前后可对比）；R-8（DEBUG 级、可按包过滤、打点本身不产生额外 BMS 查询）。

**Design Context**：ADR-12。计数器位于 BundleMgrHelper（AMS/AppMgr/应用进程各自进程内独立计数）；三处输出点各一行 TAG_LOGD；开关默认关，param/编译宏控制。

**Required Rules**

| Rule ID | Must / Must Not |
|---------|-----------------|
| OH-ARCH-ERROR-LOG | Must：使用 TAG_LOGD/TAG_LOGI（hilog_tag_wrapper.h），Must Not：新增错误码 |
| spec R-8 | Must Not：打点产生额外 BMS 查询 |

**Steps**

- [ ] 在 bundle_mgr_helper 增加原子计数器与开关（SetUp：确认现有 Get* 入口清单；Teardown：无状态）
- [ ] 三处输出点接线，编译 `./build.sh --product-name rk3568 --build-target ability_runtime`
- [ ] DEBUG 开关打开，验证日志输出
- [ ] 填写完成证据

**Completion Evidence**

| 证据类型 | 命令/路径 | 结果 |
|----------|-----------|------|
| 构建 | `./build.sh --product-name rk3568 --build-target ability_runtime` | 待执行 |
| 日志 | hilog 抓取三行计数 | 待执行 |

**Handoff Summary**

| 项 | 内容 |
|----|------|
| 任务描述 | 见上（目标/AC/非目标/判据/停止条件/Steps） |
| 允许修改 | 上述 Files 表 4 个文件 |
| 允许新建 | 无 |
| 只读参考 | design.md ADR-12；spec.md R-8/AC-10 |
| Spec 摘要 | AC-10 + R-8（DEBUG 级、可过滤、零额外查询） |
| Design 摘要 | ADR-12：BundleMgrHelper 进程内原子计数 + 三处输出点（StartAbilityInfoWrap 析构 / LoadAbility 完成 / HandleLaunchApplication 完成），开关默认关 |
| 执行步骤 | 见 Steps |
| 验证命令 | `./build.sh --product-name rk3568 --build-target ability_runtime` / 期望： 编译 PASS + 日志可见 |
| 完成规则 | 不得修改允许范围外文件；无 fresh 验证证据不得声明完成 |

### TASK-2: AMS 拦截器缓存复用

| 字段 | 内容 |
|------|------|
| 任务目标 | 拦截器链统一走 StartAbilityUtils thread_local 缓存优先入口：新增 abilityInfo 缓存优先查询入口（对标既有 GetApplicationInfo 缓存版 start_ability_utils.cpp:71-75）；IsStartIncludeAtomicService（ability_util.h:271/284）、ScreenUnlockInterceptor（screen_unlock_interceptor.cpp:269）、ExtensionControlInterceptor（extension_control_interceptor.cpp:158）改为缓存优先，未命中回退直查 |
| AC 映射 | AC-3、AC-8 |
| 前置依赖 | TASK-1 |
| 非目标 | 不引入跨请求缓存；不改拦截判定逻辑；不动 ability_jump_interceptor 已有缓存比对（52-55，作为参照模式） |
| 完成判据 | 单测：构造缓存已填充场景，mock BMS 计数=0 次新增查询且拦截判定结果与直查一致；缓存为空时回退直查结果一致 |
| 停止条件 | 缓存键无法覆盖 (bundleName, appIndex, userId) 维度、或拦截语义需要直查新数据时停止回传 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Modify | `services/abilitymgr/src/utils/start_ability_utils.cpp`、`services/abilitymgr/include/utils/start_ability_utils.h` | abilityInfo 缓存优先查询入口 |
| Modify | `services/abilitymgr/include/utils/ability_util.h` | IsStartIncludeAtomicService 改缓存优先；GetNameForUid 结果函数内复用 |
| Modify | `services/abilitymgr/src/interceptor/screen_unlock_interceptor.cpp`、`services/abilitymgr/src/interceptor/extension_control_interceptor.cpp` | 缓存优先后直查 |
| Test | `test/unittest/ability_interceptor_test/`、`test/unittest/ability_jump_interceptor_test/` | mock BMS 计数 + 判定结果比对用例 |

**Spec Context**：AC-3（拦截器需要目标包 applicationInfo/abilityInfo 时优先命中本次启动已查询结果，未命中才直查）；AC-8（复用数据不可用回退直查、有日志）；R-2（拦截判定结果与直查一致）；R-7（缓存键含三维度）。

**Design Context**：ADR-1。参照模式 ability_jump_interceptor.cpp:52-55；缓存生命周期由 StartAbilityInfoWrap RAII 管理（start_ability_utils.cpp:190-203）。

**Required Rules**

| Rule ID | Must / Must Not |
|---------|-----------------|
| spec R-2/R-7 | Must：判定结果与直查一致；Must Not：跨请求复用 |
| OH-ARCH-ERROR-LOG | Must：回退路径 TAG_LOGW |

**Steps**

- [ ] 写失败测试：mock BMS 计数，断言缓存命中场景查询次数不增
- [ ] 运行确认失败
- [ ] 最小实现：缓存优先入口 + 三处改造
- [ ] 运行测试通过 + 既有拦截器 UT 全过
- [ ] 填写完成证据

**Completion Evidence**

| 证据类型 | 命令/路径 | 结果 |
|----------|-----------|------|
| 测试 | `run -t UT -ts ability_interceptor_test` 等 | 待执行 |

**Handoff Summary**

| 项 | 内容 |
|----|------|
| 任务描述 | 见上 |
| 允许修改 | Files 表 4+2 文件 |
| 允许新建 | 上述两个测试目录内新增用例文件 |
| 只读参考 | `services/abilitymgr/src/interceptor/ability_jump_interceptor.cpp:52-55`（缓存比对参照）；design.md ADR-1 |
| Spec 摘要 | AC-3/AC-8 + R-2/R-7 |
| Design 摘要 | ADR-1：统一缓存优先入口，thread_local，RAII 生命周期 |
| 执行步骤 | 见 Steps |
| 验证命令 | `run -t UT -ts ability_interceptor_test` / 期望： PASS |
| 完成规则 | 同 TASK-1 |

### TASK-3: caller/权限链同函数去重

| 字段 | 内容 |
|------|------|
| 任务目标 | ① UpdateCallerInfo 将已查 callerBundleName 传入 ClearProtectedWantParam（消除 update_caller_info_util.cpp:425 重复 GetNameForUid）；② CheckAppProvisionMode 查询结果写回 thread_local 缓存（消除 start_ability_utils.cpp:119-141 与 143-170 双查）；③ VerifyPermission（ability_manager_service.cpp:14015-14050）内 GetNameForUid 去重；④ GetCallerUidAndToken（:17471）的 GetBundleInfo(WITH_ABILITIES) 降为 GetUidByBundleName |
| AC 映射 | AC-2、AC-6、AC-8 |
| 前置依赖 | TASK-1 |
| 非目标 | 不跨阶段复用校验输入（spec R-5 禁止）；不改权限判定逻辑；不动 ERMS/隐式启动路径 |
| 完成判据 | 权限校验用例结果与变更前逐点一致；四处同函数查询计数各=1 |
| 停止条件 | GetUidByBundleName 与 GetBundleInfo 返回的 uid 语义在任一分身/沙箱场景不一致时停止回传 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Modify | `services/abilitymgr/src/utils/update_caller_info_util.cpp` | callerBundleName 参数传递 |
| Modify | `services/abilitymgr/src/utils/start_ability_utils.cpp` | CheckAppProvisionMode 写回缓存 |
| Modify | `services/abilitymgr/src/ability_manager_service.cpp` | VerifyPermission 去重 + GetCallerUidAndToken 降载 |
| Test | `test/unittest/ability_manager_service_first_test/`（或最近邻权限用例套件） | 校验结果比对 + 查询计数用例 |

**Spec Context**：AC-2（同函数内重复查询消减为单次）；AC-6（校验输入/时序/结果逐点一致）；AC-8（失败回退）；R-1/R-5。

**Design Context**：ADR-2。仅同函数内去重；GetCallerUidAndToken 降载前后均取同一 uid 数据源。

**Required Rules**

| Rule ID | Must / Must Not |
|---------|-----------------|
| spec R-5 | Must Not：跨阶段复用校验输入 |
| spec R-1 | Must：同函数 BMS 查询次数=1 |

**Steps**

- [ ] 写失败测试（权限用例 + 计数断言）
- [ ] 最小实现四处去重
- [ ] 测试通过 + 权限相关既有 UT 全过
- [ ] 填写完成证据

**Completion Evidence**

| 证据类型 | 命令/路径 | 结果 |
|----------|-----------|------|
| 测试 | `run -t UT -ts ability_manager_service_first_test` | 待执行 |

**Handoff Summary**

| 项 | 内容 |
|----|------|
| 任务描述 | 见上 |
| 允许修改 | Files 表 3+1 |
| 允许新建 | 测试目录内新增用例 |
| 只读参考 | design.md ADR-2；proposal.md R6/R7/R8 证据行号 |
| Spec 摘要 | AC-2/AC-6/AC-8 + R-1/R-5 |
| Design 摘要 | ADR-2：同函数去重；禁止跨阶段复用校验输入 |
| 执行步骤 | 见 Steps |
| 验证命令 | `run -t UT -ts ability_manager_service_first_test` / 期望： PASS |
| 完成规则 | 同 TASK-1 |

### TASK-4: mainElement 判断降载

| 字段 | 内容 |
|------|------|
| 任务目标 | MainElementUtils::IsMainUIAbility（main_element_utils.cpp:74）由 GetBundleInfoV9(WITH_ABILITY\|WITH_HAP_MODULE) 降为 GetHapModuleInfo 取 mainElementName 比较；GetHapModuleInfo 失败时回退现有全量查询 |
| AC 映射 | AC-2、AC-8 |
| 前置依赖 | TASK-1 |
| 非目标 | 不改 mainElement 判定规则；不动 AbilityRecord 结构 |
| 完成判据 | 判定结果与全量查询路径一致（含分身/沙箱维度）；失败回退路径有日志且结果一致 |
| 停止条件 | GetHapModuleInfo 返回值不含 mainElementName 或语义与全量路径不一致时停止回传 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Modify | `services/abilitymgr/src/utils/main_element_utils.cpp` | 降载 + 回退 |
| Modify | `services/abilitymgr/src/ability_record.cpp` | 调用点适配（如需要） |
| Test | `test/unittest/ability_record_test/` | 判定一致性与回退用例 |

**Spec Context**：AC-2、AC-8；R-1/R-6。

**Design Context**：ADR-3。调用点 ability_record.cpp:359（LoadAbility，每 record 一次）；载荷从全包降单模块。

**Required Rules**

| Rule ID | Must / Must Not |
|---------|-----------------|
| spec R-6 | Must：失败回退 + TAG_LOGW |

**Steps**

- [ ] 写失败测试（判定一致性 + 回退）
- [ ] 最小实现
- [ ] 测试通过
- [ ] 填写完成证据

**Completion Evidence**

| 证据类型 | 命令/路径 | 结果 |
|----------|-----------|------|
| 测试 | `run -t UT -ts ability_record_test` | 待执行 |

**Handoff Summary**

| 项 | 内容 |
|----|------|
| 任务描述 | 见上 |
| 允许修改 | Files 表 2+1 |
| 允许新建 | 测试目录内新增用例 |
| 只读参考 | design.md ADR-3；proposal.md R1 |
| Spec 摘要 | AC-2/AC-8 + R-1/R-6 |
| Design 摘要 | ADR-3：GetBundleInfoV9→GetHapModuleInfo + 失败回退 |
| 执行步骤 | 见 Steps |
| 验证命令 | `run -t UT -ts ability_record_test` / 期望： PASS |
| 完成规则 | 同 TASK-1 |

### TASK-5: Module/App RunningRecord 持有复用

| 字段 | 内容 |
|------|------|
| 任务目标 | ① ModuleRunningRecord::Init 持有 shared_ptr<const HapModuleInfo>，GetHapModuleInfo 返回持有副本（消除 module_running_record.cpp:378-400 回查）；② AppRunningRecord 持有启动时 shared_ptr<const BundleInfo>（StartProcess 6032 已收 const BundleInfo&），UpdateApplicationInfoInstalled 刷新；③ CreatNewStartMsg 增加可选预查参数，StartNativeProcessForDebugger（9956）传入 9934 已查结果 |
| AC 映射 | AC-4、AC-5、AC-8 |
| 前置依赖 | TASK-1 |
| 非目标 | 不改 RestartKeepAliveProcess/RestartResidentProcess 的查询（重启语义需新数据）；不动 PreloadRequest 结构 |
| 完成判据 | 单测：attach 后 AddAbilityStage 链路 BMS 查询=0；StartNativeProcessForDebugger 全程 GetBundleAndHapInfo=1 组；更新事件后持有值刷新 |
| 停止条件 | 持有值在某路径被原地修改（违反只读约束）或更新事件无法覆盖持有值刷新时停止回传 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Modify | `services/appmgr/include/module_running_record.h`、`services/appmgr/src/module_running_record.cpp` | 持有 HapModuleInfo |
| Modify | `services/appmgr/include/app_running_record.h`、`services/appmgr/src/app_running_record.cpp` | 持有 BundleInfo + setter/刷新 |
| Modify | `services/appmgr/src/app_mgr_service_inner.cpp`、`services/appmgr/include/app_mgr_service_inner.h` | StartProcess 设置持有值；CreatNewStartMsg 可选参数；UpdateApplicationInfoInstalled 刷新 |
| Test | `test/unittest/ams_app_running_record_test/`、`test/unittest/app_mgr_service_inner_mock_test/` | 计数与刷新用例 |

**Spec Context**：AC-4（同次启动后续阶段复用已查结果）；AC-5（模块信息读取返回持有副本，0 次查询，更新后刷新）；AC-8（回退）；R-3/R-4/R-6。

**Design Context**：ADR-4/5/6。持有为 shared_ptr<const>；替换仅发生在 UpdateApplicationInfoInstalled；CreatNewStartMsg 可选参数默认 nullptr 走现有自查。

**Required Rules**

| Rule ID | Must / Must Not |
|---------|-----------------|
| spec R-3 | Must：复用对象传递期间只读 |
| spec R-4 | Must：更新后副本刷新 |

**Steps**

- [ ] 写失败测试（回查=0、双查=1、刷新生效）
- [ ] 最小实现
- [ ] 测试通过 + appmgr 既有 UT 全过
- [ ] 填写完成证据

**Completion Evidence**

| 证据类型 | 命令/路径 | 结果 |
|----------|-----------|------|
| 测试 | `run -t UT -ts ams_app_running_record_test`、`run -t UT -ts app_mgr_service_inner_mock_test` | 待执行 |

**Handoff Summary**

| 项 | 内容 |
|----|------|
| 任务描述 | 见上 |
| 允许修改 | Files 表 5+2 |
| 允许新建 | 测试目录内新增用例 |
| 只读参考 | design.md ADR-4/5/6；proposal.md R4/R5/S3/S4 |
| Spec 摘要 | AC-4/AC-5/AC-8 + R-3/R-4/R-6 |
| Design 摘要 | 持有复用 + 更新刷新 + 可选参数 |
| 执行步骤 | 见 Steps |
| 验证命令 | 见上 / 期望： PASS |
| 完成规则 | 同 TASK-1 |

### TASK-6: InnerAPI 检查接口参数扩展（A-4）

| 字段 | 内容 |
|------|------|
| 任务目标 | `IAppMgr::IsSpecifiedModuleLoaded`/`IsCorrespondingProcessAttachDebug` 追加可选 processName 参数（interface/proxy/stub/client/service 五层 + parcel 末尾追加）；AppMgr 实现优先用传入值查表，为空回退现有 GetBundleAndHapInfo 流程；AMS 三个调用点（ui_ability_lifecycle_manager.cpp:4684、extension_record_manager.cpp:619、ability_connect_manager.cpp:3443）传入按 abilityInfo.process/customProcess 规则算得的 processName |
| AC 映射 | AC-4、AC-8、AC-9 |
| 前置依赖 | TASK-5 |
| 非目标 | 不改 LoadAbility 签名；不上移 MakeProcessName 全部逻辑（AMS 仅算常见情形，不确定时传空回退） |
| 完成判据 | 未携带参数调用行为与现状一致（兼容用例）；携带且匹配时 AppMgr 0 次包查询；分身/沙箱维度不匹配回退直查 |
| 停止条件 | processName 计算规则无法与 MakeProcessName 对齐、或 parcel 兼容性验证失败时停止回传 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Modify | `interfaces/inner_api/app_manager/include/appmgr/app_mgr_interface.h` 及 src/ 下 proxy/stub/client 对应文件 | 可选参数五层透传 |
| Modify | `services/appmgr/include/app_mgr_service.h`、`services/appmgr/src/app_mgr_service.cpp`、`services/appmgr/src/app_mgr_service_inner.cpp` 及头文件 | 实现优先传入值 + 回退 |
| Modify | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp`、`services/abilitymgr/src/extension_record/extension_record_manager.cpp`、`services/abilitymgr/src/ability_connect_manager.cpp` | 调用方传参 |
| Test | `test/unittest/ui_ability_lifecycle_manager_test/` 等 | 兼容/命中/回退/维度用例 |

**Spec Context**：AC-4/AC-8/AC-9；接口规格三行为场景（携带有效→直用；未携带/无效→回退；维度不匹配→回退）。

**Design Context**：ADR-7。parcel 末尾追加；同镜像升级无跨版本问题；ADR 风险行：processName 一致性不确定时传空。

**Required Rules**

| Rule ID | Must / Must Not |
|---------|-----------------|
| OH-ARCH-IPC-SAF | Must：parcel 末尾追加，未携带行为不变 |
| spec R-7 | Must：维度不匹配回退 |

**Steps**

- [ ] 写失败测试（兼容/命中/回退三场景）
- [ ] 五层签名 + 实现 + 调用方
- [ ] 测试通过
- [ ] 填写完成证据

**Completion Evidence**

| 证据类型 | 命令/路径 | 结果 |
|----------|-----------|------|
| 测试 | `run -t UT -ts ui_ability_lifecycle_manager_test` | 待执行 |

**Handoff Summary**

| 项 | 内容 |
|----|------|
| 任务描述 | 见上 |
| 允许修改 | Files 表清单 |
| 允许新建 | 测试目录内新增用例 |
| 只读参考 | design.md ADR-7；spec.md 接口规格 |
| Spec 摘要 | AC-4/AC-8/AC-9 + 接口三场景 |
| Design 摘要 | ADR-7：可选参数 + 回退；processName 不确定传空 |
| 执行步骤 | 见 Steps |
| 验证命令 | 见上 / 期望： PASS |
| 完成规则 | 同 TASK-1 |

### TASK-7: 应用进程启动数据复用

| 字段 | 内容 |
|------|------|
| 任务目标 | ① MainThread::HandleLaunchApplication 将已查 BundleInfo 的 hapModuleInfos 持久化为 OHOSApplication 的 map<moduleName, shared_ptr<const HapModuleInfo>>，ContextImpl::InitHapModuleInfo 先查映射未命中回退 IPC（消除 R10）；② 同函数 hspList/BundleInfo 存为 MainThread 成员，GetBundleAndHspListForUpdateRuntime 优先复用（R12）；③ ApplicationInfo 单一 shared_ptr：main_thread.cpp:1780 不再 make_shared 新对象，与 MainThread::applicationInfo_ 共享（S7） |
| AC 映射 | AC-11、AC-12、AC-13、AC-8 |
| 前置依赖 | TASK-1 |
| 非目标 | 不改 ScheduleAbilityStage 既有路径；不改插件路径（InitPluginHapModuleInfo 保持 IPC）；不做应用进程通用 GetBundleInfo 缓存 |
| 完成判据 | 单测：非 entry module 的 AddAbilityStage(abilityRecord) 路径 BMS 查询=0 且 stageContext hapModuleInfo 正确；混合 arkTSMode 场景 0 重查；ApplicationInfo 指针同一（地址相等断言）；映射缺失时回退 IPC 行为一致 |
| 停止条件 | 启动时序中映射填充晚于首个消费点且无法重排时停止回传 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Modify | `frameworks/native/appkit/app/main_thread.cpp` 及头文件 | 持久化映射/hspList/BundleInfo 成员；ApplicationInfo 共享 |
| Modify | `frameworks/native/appkit/app/ohos_application.cpp` 及头文件 | 映射持有与查询入口 |
| Modify | `frameworks/native/appkit/ability_runtime/context/context_impl.cpp` 及头文件 | InitHapModuleInfo 映射优先 + 回退；SetApplicationInfo 共享指针 |
| Test | `test/unittest/appkit/main_thread_test/`、`test/unittest/appkit/ohos_application_test/`、`test/unittest/ability_stage_context_test/` | 计数/指针同一性/回退用例 |

**Spec Context**：AC-11/12/13、AC-8；R-11/R-12/R-13/R-6。

**Design Context**：ADR-8/10/11。映射在 HandleLaunchApplication（main_thread.cpp:1694 查询点后）填充；插件路径例外；更新整体替换指针。

**Required Rules**

| Rule ID | Must / Must Not |
|---------|-----------------|
| spec R-12 | Must：ApplicationInfo 单一 shared_ptr，持有期只读 |
| spec R-6 | Must：映射未命中回退 IPC + 日志 |

**Steps**

- [ ] 写失败测试（计数=0、指针同一、回退一致）
- [ ] 最小实现
- [ ] 测试通过 + appkit 既有 UT 全过
- [ ] 填写完成证据

**Completion Evidence**

| 证据类型 | 命令/路径 | 结果 |
|----------|-----------|------|
| 测试 | `run -t UT -ts main_thread_test`、`run -t UT -ts ohos_application_test`、`run -t UT -ts ability_stage_context_test` | 待执行 |

**Handoff Summary**

| 项 | 内容 |
|----|------|
| 任务描述 | 见上 |
| 允许修改 | Files 表 3+3 |
| 允许新建 | 测试目录内新增用例 |
| 只读参考 | design.md ADR-8/10/11；proposal.md R10/R12/S7 |
| Spec 摘要 | AC-11/12/13/8 + R-11/12/13/6 |
| Design 摘要 | 映射持久化 + 指针共享 + 回退 IPC |
| 执行步骤 | 见 Steps |
| 验证命令 | 见上 / 期望： PASS |
| 完成规则 | 同 TASK-1 |

### TASK-8: runtime 模块路径复用

| 字段 | 内容 |
|------|------|
| 任务目标 | JsRuntime 将已展开的 moduleName→hapPath 映射注入 JsModuleReader/HybridJsModuleReader（构造或 setter）；GetPresetAppHapPath（js_module_reader.cpp:263）与 InitSourceMap 的 GetHapPathList（:290）命中映射即用，未命中回退现有 BMS 查询 |
| AC 映射 | AC-11、AC-8 |
| 前置依赖 | TASK-7（映射数据来源在 TASK-7 确立） |
| 非目标 | 不改 worker AssetHelper（js_worker.cpp:386）与 madvise 路径；不改模块加载语义 |
| 完成判据 | 单测：映射命中时 BMS 查询=0，解析结果与 IPC 路径一致；未命中回退一致 |
| 停止条件 | 映射在 hybrid/1.0 运行时构造时序不可用时停止回传 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Modify | `frameworks/native/runtime/js_runtime.cpp` | 映射注入 |
| Modify | `frameworks/native/runtime/js_module_reader.cpp/h`、`frameworks/native/runtime/hybrid_js_module_reader.cpp` | 命中即用 + 回退 |
| Test | runtime 最近邻测试套件（test/unittest 下 js_runtime 相关） | 计数与一致性用例 |

**Spec Context**：AC-11、AC-8；R-11/R-6。

**Design Context**：ADR-9。数据来源 options.hapModulePath/pkgContextInfoJsonStringMap_（main_thread.cpp:1925-1938）。

**Required Rules**

| Rule ID | Must / Must Not |
|---------|-----------------|
| spec R-6 | Must：未命中回退 + 日志 |

**Steps**

- [ ] 写失败测试
- [ ] 最小实现
- [ ] 测试通过
- [ ] 填写完成证据

**Completion Evidence**

| 证据类型 | 命令/路径 | 结果 |
|----------|-----------|------|
| 测试 | `run -t UT -tp ability_runtime`（runtime 相关套件） | 待执行 |

**Handoff Summary**

| 项 | 内容 |
|----|------|
| 任务描述 | 见上 |
| 允许修改 | Files 表 3+1 |
| 允许新建 | 测试目录内新增用例 |
| 只读参考 | design.md ADR-9；proposal.md R11 |
| Spec 摘要 | AC-11/AC-8 + R-11/R-6 |
| Design 摘要 | ADR-9：映射注入 reader，命中免查 |
| 执行步骤 | 见 Steps |
| 验证命令 | 见上 / 期望： PASS |
| 完成规则 | 同 TASK-1 |

### TASK-9: 全量回归 + 打点对比

| 字段 | 内容 |
|------|------|
| 任务目标 | 全量 `run -t UT -tp ability_runtime` 回归；开启打点采集优化前后单次冷启动三侧 BMS 查询次数，形成对比记录（写入 evidence/）；分身/沙箱/插件组合场景补充验证 |
| AC 映射 | AC-7、AC-8、AC-9、AC-10 |
| 前置依赖 | TASK-2~TASK-8 |
| 非目标 | 不补全仓既有失败用例（与本变更无关的既有失败另单跟踪）；不做性能 benchmark 报告 |
| 完成判据 | 全量 UT 通过（或既有失败与本变更无关的逐条说明）；打点对比记录显示合计查询次数降幅 ≥40% |
| 停止条件 | 出现与本变更相关的回归失败时停止回传，按 workflow 回退源头交付件修复 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Test | `test/unittest/`（全量） | 回归 |
| Create | `.codespec/changes/dup-bundle-info-query/evidence/` | 打点对比记录与 UT 结果 |

**Spec Context**：AC-7（全量 UT 通过）；AC-8/AC-9（异常与边界组合）；AC-10（可对比）。

**Design Context**：非功能指标：合计降幅 ≥40%；内存增量 <1MB。

**Required Rules**

| Rule ID | Must / Must Not |
|---------|-----------------|
| workflow 证据先于声明 | Must：当次会话生成验证证据 |

**Steps**

- [ ] 全量 UT 执行并记录
- [ ] 打点开启，冷启动采集优化后数据，与基线对比
- [ ] 分身/沙箱/插件组合场景验证
- [ ] 证据写入 evidence/

**Completion Evidence**

| 证据类型 | 命令/路径 | 结果 |
|----------|-----------|------|
| 测试 | `run -t UT -tp ability_runtime` | 待执行 |
| 打点 | evidence/ 对比记录 | 待执行 |

**Handoff Summary**

| 项 | 内容 |
|----|------|
| 任务描述 | 见上 |
| 允许修改 | 仅 evidence/ 新建；测试失败时按 workflow 回源头修复（回 spec/design/plan 对应 Task） |
| 允许新建 | `.codespec/changes/dup-bundle-info-query/evidence/` |
| 只读参考 | spec.md 非功能性需求表 |
| Spec 摘要 | AC-7/8/9/10 + 降幅 ≥40% 指标 |
| Design 摘要 | 打点点位见 ADR-12 |
| 执行步骤 | 见 Steps |
| 验证命令 | `run -t UT -tp ability_runtime` / 期望： PASS |
| 完成规则 | 同 TASK-1 |

## Plan 自审清单

- [x] 每个 P0/P1 AC 至少映射到一个 Task（AC-2~13 全覆盖；AC-1 已在 proposal 阶段交付）
- [x] 每个 Task 文件范围明确
- [x] 每个 Task 明确前置依赖、非目标、完成判据和停止条件
- [x] 每个 Task 有验证命令
- [x] Task 粒度形成能力闭环
- [x] 没有 TBD/TODO/占位符
- [x] 没有要求 Agent 自行寻找未列出的上下文
- [x] 交接信息自包含
- [x] 每个 Task 验证在完成时立即执行并记录证据
- [x] 无超 3000 行阈值 Task

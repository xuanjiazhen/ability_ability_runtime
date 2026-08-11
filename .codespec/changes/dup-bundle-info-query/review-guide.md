# 人工检视辅助文档｜应用启动链路包信息重复查询治理

> 本文档辅助 reviewer 高效检视本次变更（单笔提交 `94500372dd`，分支 `feat/dup-bundle-info-query`，54 文件）。
> 它不是审查结论，而是**导航 + 检视清单**：告诉你改了什么、为什么改、每个点该看什么、预期行为是什么。
> 完整设计依据见同目录 `design.md`，行为规格见 `spec.md`，已完成的审查记录见 `review.md`。

---

## 一、变更一句话

应用冷启动链路上对 BMS 的包信息查询存在系统性重复（单次冷启动 8~12 次、109 处直连调用点、包信息多处拷贝），本次以"复用优先 + 查询降载 + 打点可观测"治理，**对外行为语义零变化、零 Public/System API 变更、每个复用点都带回退**，预计冷启动 BMS 同步查询降至 ≤5 次。

**红线（review 时重点确认未被打破）：**
1. 权限校验、拦截判定、生命周期时序与变更前逐点一致
2. 无新增跨请求缓存层（缓存应做在包管理侧，本次排除）
3. 复用数据按 (bundleName, appIndex, userId) 三维匹配，分身/沙箱不串数据
4. 每个复用点失败/未命中都回退原有直查 BMS 行为

---

## 二、检视优先级矩阵

| 优先级 | 区域 | 原因 | 文件 |
|--------|------|------|------|
| 🔴 P0 行为敏感 | 权限校验去重 | 校验结果必须逐点一致 | ability_manager_service.cpp |
| 🔴 P0 行为敏感 | 拦截器缓存优先 | 判定结果不能因复用而变 | ability_util.h、3 个 interceptor |
| 🔴 P0 行为敏感 | mainElement 判断降载 | keepAlive/预启动标志依赖 | main_element_utils.cpp |
| 🟠 P1 并发/失效 | 持有数据加锁与更新失效 | IPC 触发并发、应用更新需刷新 | module_running_record.cpp、ohos_application.cpp、main_thread.cpp |
| 🟠 P1 维度匹配 | 分身/沙箱路径 | appIndex≠0 必须走原全量 | app_mgr_service_inner.cpp |
| 🟡 P2 内部优化 | AppMgr 轻量查询、LoadAbility 分支 | 纯内部，回退完备即可 | app_mgr_service_inner.cpp |
| 🟢 P3 可观测 | 打点计数 | 不影响行为 | bundle_mgr_helper.*、三输出点 |
| ⚪ 跳过 | mock 桩、单测 | 编译/测试用 | test/** |

---

## 三、改动文件全清单（按模块）

### AMS 启动调度（services/abilitymgr/）
| 文件 | 改动 |
|------|------|
| `include/utils/ability_util.h` | `IsStartIncludeAtomicService` 改走缓存优先入口 + callerBundleName 出参 |
| `include/utils/start_ability_utils.h` | StartAbilityInfoWrap 增打点基线成员 |
| `include/utils/update_caller_info_util.h` | ClearProtectedWantParam 增 resolvedCallerName 出参 |
| `include/utils/main_element_utils.h` | 新增 `IsMainUIAbility(AbilityInfo, userId)` 轻量重载 |
| `include/interceptor/ability_jump_interceptor.h` | CheckControl 增 callerBundleName 参 |
| `src/utils/start_ability_utils.cpp` | CheckAppProvisionMode 成功后写回缓存；wrap 构造复用（四维匹配）+ 析构打点 |
| `src/utils/update_caller_info_util.cpp` | 两处 caller 名复用 |
| `src/utils/main_element_utils.cpp` | 轻量重载实现 + 失败回退全量 |
| `src/interceptor/ability_jump_interceptor.cpp` | caller 名一次解析链内复用 |
| `src/interceptor/screen_unlock_interceptor.cpp` | 缓存优先命中分支 |
| `src/ability_record.cpp` | LoadAbility 调用点改用轻量重载 |
| `src/ability_manager_service.cpp` | VerifyPermission uid 复用；GetCallerUidAndToken 降载 |

### AppMgr 孵化加载（services/appmgr/）
| 文件 | 改动 |
|------|------|
| `include/app_mgr_service_inner.h` | CreatNewStartMsg 增可选预查参数 |
| `include/module_running_record.h` | 增 hapModuleInfo_ 成员 + ResetHapModuleInfo |
| `src/app_mgr_service_inner.cpp` | LoadAbility selfPid 分支 hap 单查；CreatNewStartMsg 复用；两检查接口轻量化；LoadAbility 打点守卫 |
| `src/module_running_record.cpp` | 持有/读取/重置 hapModuleInfo_（加锁） |
| `src/app_running_record.cpp` | UpdateApplicationInfoInstalled 重置模块持有 |

### 应用进程框架/运行时（frameworks/、interfaces/）
| 文件 | 改动 |
|------|------|
| `interfaces/kits/native/appkit/app/ohos_application.h` | module→HapModuleInfo 映射 + 公有 Set/Get（加锁） |
| `interfaces/kits/native/appkit/app/main_thread.h` | launchBundleInfo_/launchHspList_ 成员 |
| `interfaces/inner_api/runtime/include/js_runtime.h` | **新增 InnerAPI 静态方法** `SetModuleHapPathList`（转发，见下方说明） |
| `interfaces/kits/native/appkit/ability_bundle_manager_helper/bundle_mgr_helper.h` | 打点计数接口 + 原子成员 |
| `frameworks/native/appkit/app/main_thread.cpp` | 启动数据持有/共享指针/混合模式复用/更新失效/打点守卫 |
| `frameworks/native/appkit/app/ohos_application.cpp` | AddAbilityStage 映射命中分支 + 更新清空 |
| `frameworks/native/appkit/ability_bundle_manager_helper/bundle_mgr_helper.cpp` | RecordBmsQuery 注入 + 开关同步 |
| `frameworks/native/runtime/js_module_reader.cpp/.h` | 静态 hapPathList 复用 |
| `frameworks/native/runtime/hybrid_js_module_reader.cpp` | 复用 JsModuleReader 静态表 |
| `frameworks/native/runtime/js_runtime.cpp` | SetModuleHapPathList 转发 |
| `frameworks/native/appkit/BUILD.gn` | 加 `init:libbegetutil` 依赖（system::GetBoolParameter） |

### 测试
新增单测：`main_element_utils_test`(3)、`appkit/ohos_application_test`(1)、`appkit/main_thread_test`(1)、`ams_app_running_record_test`(1)、`runtime_test/js_module_reader_test`(1)；19 个 `mock_bundle_mgr_helper` 补桩。

---

## 四、重点检视项（逐条，附定位 + 看什么 + 预期）

### 🔴 P0-1 权限校验 uid 复用
- **定位**：`services/abilitymgr/src/ability_manager_service.cpp:14046`（VerifyPermission）
- **看什么**：`if (uid == callerUid)` 时复用 callerBundleName，否则才 `GetNameForUid(uid)`。原代码这里对 `uid`（被校验方）和 `callerUid`（调用方）各查一次。
- **预期**：仅当**两个 uid 相等**时省掉第二次查询；不等时行为与原实现完全一致。校验结果、返回错误码逐点不变。

### 🔴 P0-2 GetCallerUidAndToken 降载
- **定位**：`ability_manager_service.cpp:17468-17482`
- **看什么**：原 `GetBundleInfo(GET_BUNDLE_WITH_ABILITIES)` 只为读 `bundleInfo.uid`，现改为 `GetUidByBundleName`。
- **预期**：callerUid 取值一致；新增了 bms 空指针检查（原代码缺失，属改进）。注意 GetUidByBundleName 失败返回 ≤0 时走原有"不设置 callerUid"行为。

### 🔴 P0-3 拦截器缓存优先
- **定位**：`ability_util.h:262`（IsStartIncludeAtomicService）、`screen_unlock_interceptor.cpp:264`、`ability_jump_interceptor.cpp`（DoProcess→CheckControl）
- **看什么**：`StartAbilityUtils::GetApplicationInfo` 仅在 `bundleName == startAbilityInfo->GetAppBundleName()` 时命中 thread_local 缓存，否则回退 BMS 直查。
- **预期**：拦截**判定结果**（是否原子服务、是否锁屏可见、是否跳转拦截）与直查完全一致。callerBundleName 在 DoProcess 解析后透传给 CheckControl，消除同次拦截内重复 GetNameForUid。

### 🔴 P0-4 CheckAppProvisionMode 写回 + wrap 复用维度
- **定位**：`start_ability_utils.cpp:140`（写回）、`start_ability_utils.cpp` StartAbilityInfoWrap 构造（复用）
- **看什么**：
  - 写回条件 `newResolved && startAbilityInfo == nullptr` 且在 `status == ERR_OK` **之后**——确认失败空条目不会遗留。
  - wrap 复用条件含**四维**：bundleName + abilityName + appIndex + userId（uid/BASE_USER_RANGE）。确认隐式 want（bundle/ability 为空）不会错误命中、多用户/分身不会串。
- **预期**：debug 启动双查消除；非 debug 路径行为不变。

### 🔴 P0-5 mainElement 降载 + 回退
- **定位**：`main_element_utils.cpp:90-103`（IsMainUIAbility 轻量重载）、`ability_record.cpp:357`（调用点）
- **看什么**：先 `GetHapModuleInfo(abilityInfo)` 单模块，判断 `moduleType==ENTRY && mainElementName==abilityInfo.name`；失败 TAG_LOGW 回退原全量 `IsMainUIAbility(bundleName, abilityName, userId)`。
- **预期**：keepAlive/isMainElementRunning 标志设置结果与全量路径一致；GetHapModuleInfo 失败时回退保证语义不变。

### 🟠 P1-1 ModuleRunningRecord 持有 + 加锁 + 失效
- **定位**：`module_running_record.cpp`（Init:54 / GetHapModuleInfo:386 / 回填:432 / ResetHapModuleInfo:60，均 `abilitiesMutex_`）；`app_running_record.cpp:448`（UpdateApplicationInfoInstalled 调 Reset）
- **看什么**：`hapModuleInfo_` 读写/reset 均在 `abilitiesMutex_` 内（GetHapModuleInfo 里 BMS 查询在锁外，仅 shared_ptr 操作在锁内）。这是 attach 后 AddAbilityStage 回查清零的核心。
- **预期**：同模块第二次 GetHapModuleInfo 直接返回持有副本（0 次 BMS 查询）；应用更新后持有被重置、下次重新查询。

### 🟠 P1-2 OHOSApplication 映射 + 加锁 + 失效
- **定位**：`ohos_application.h:281/289`（Set/Get，`runtimeMutex_`）、`ohos_application.cpp:589`（AddAbilityStage 命中分支）、`:898`（UpdateApplicationInfoInstalled clear）
- **看什么**：命中 `GetHapModuleInfoByModule` 后调 `stageContext->InitHapModuleInfo(HapModuleInfo)`，未命中回退 `InitHapModuleInfo(abilityInfo)`（原 IPC 路径）。三处访问均在 `runtimeMutex_` 内。
- **预期**：非 entry module 的 AddAbilityStage(abilityRecord) 路径不再发起 IPC；应用更新清空映射后回退 IPC。

### 🟠 P1-3 应用进程启动数据失效
- **定位**：`main_thread.cpp:2716-2719`（HandleUpdateApplicationInfoInstalled）
- **看什么**：应用更新时清 `launchBundleInfo_`/`launchHspList_`/`launchHspListReady_` + `SetModuleHapPathList({})`，保证后续复用路径回退到现查。
- **预期**：更新后 GetBundleAndHspListForUpdateRuntime / JsModuleReader 路径解析回退现查，不拿过期数据。

### 🟠 P1-4 分身/沙箱维度（appIndex≠0）
- **定位**：`app_mgr_service_inner.cpp:2828`（IsCorrespondingProcessAttachDebug）、`:9925`（IsSpecifiedModuleLoaded）、`:5820`（CreatNewStartMsg 复用条件）
- **看什么**：轻量查询/复用**仅 appIndex==0 主路径**；appIndex≠0（分身/沙箱）保持原 `GetBundleAndHapInfo` 全量查询。CreatNewStartMsg 复用额外校验 `presetBundleInfo->name == abilityInfo.bundleName`。
- **预期**：分身/沙箱/插件启动行为与变更前完全一致。

### 🟡 P2-1 LoadAbility selfPid 分支
- **定位**：`app_mgr_service_inner.cpp:2126-2133`
- **看什么**：该分支下游（HandleExistingAppRecordAfterFound）只消费 hapModuleInfo，故改为仅查 GetHapModuleInfo，失败回退 GetBundleAndHapInfo。
- **预期**：复用自身进程加载场景行为不变（原 appIndex==0 时 hap 查询本就走同一 GetHapModuleInfo 重载）。

### 🟢 P3 打点
- **定位**：`bundle_mgr_helper.cpp`（~40 个 Get* 入口 RecordBmsQuery）、三输出点（`start_ability_utils.cpp` 析构、`app_mgr_service_inner.cpp:2105` BmsQueryCountGuard、`main_thread.cpp` HandleLaunchApplication 守卫）
- **看什么**：开关 `persist.ability_runtime.bms_query_count`，默认关；`SyncBmsQueryCountSwitch` 用 `std::call_once`；打点本身**不产生**额外 BMS 查询。
- **预期**：关闭时零开销；开启时三侧各输出一行 DEBUG 计数日志。

### ⚪ InnerAPI 新增说明（需 reviewer 知悉）
- `interfaces/inner_api/runtime/include/js_runtime.h` 新增 `static void SetModuleHapPathList(...)`。
- 这是**纯转发**方法（→ JsModuleReader::SetHapPathMap），与既有 `JsRuntime::SetAppLibPath` 同文件同模式。
- **不涉及** AMS↔AppMgr、AMS↔应用 的任何接口语义变更；无 Public/System API、无 d.ts 变更。
- design/spec/proposal 已如实声明（见各文档 API 章节）。

---

## 五、检视清单（逐项打勾）

**行为语义**
- [ ] 权限校验（VerifyPermission）结果在 uid 相同/不同两种场景下均与变更前一致
- [ ] 各拦截器（jump/锁屏/管控/原子服务）判定结果与直查一致
- [ ] mainElement 判定（keepAlive/预启动标志）结果一致
- [ ] 所有复用点未命中/失败时回退路径完整，有日志

**维度与并发**
- [ ] 复用键值含 (bundleName, appIndex, userId)，分身/沙箱/多用户不串
- [ ] appIndex≠0 路径保持原全量查询
- [ ] 新增持有数据（hapModuleInfo_/hapModuleInfoMap_/launchBundleInfo_）读写加锁
- [ ] 应用更新（UpdateApplicationInfoInstalled）后持有数据被重置

**API 与构建**
- [ ] 确认 `interfaces/inner_api/app_manager/` 零改动（AMS↔AppMgr 接口未变）
- [ ] 仅新增一个 InnerAPI 静态转发方法，无 Public/System API 变更
- [ ] BUILD.gn 新增依赖（init:libbegetutil）合理

**规范**
- [ ] 日志均用 TAG_LOG* + 合法 AAFwkTag，无敏感字段
- [ ] 错误码用 ERR_* 常量，无裸数字
- [ ] 空指针检查完备（shared_ptr 解引用前判空）

---

## 六、验证方法

```bash
# 1. 编译（已通过）
./build.sh --product-name rk3568 --ccache --build-target ability_runtime_test \
  --no-prebuilt-sdk --gn-args use_thin_lto=false

# 2. 全量 UT 回归（AC-7，待设备环境执行）
run -t UT -tp ability_runtime

# 3. 打点量化对比（AC-10）
hdc shell param set persist.ability_runtime.bms_query_count true
# 冷启动若干应用后抓取三侧计数：
hdc shell hilog | grep bmsQueryCount
# 与 master 基线对比，预期三侧合计降幅 ≥40%
```

新增单测（可直接跑对应套件验证核心逻辑）：
- `main_element_utils_test`：IsMainUIAbility_005/006/007（轻量命中/非entry/回退）
- `appkit/ohos_application_test`：HapModuleInfoMap_0100（映射命中/未命中）
- `appkit/main_thread_test`：GetBundleAndHspListForUpdateRuntime_Reuse_0100（启动数据复用）
- `ams_app_running_record_test`：ModuleRunningRecord_GetHapModuleInfo_Reuse_001（持有副本返回）
- `runtime_test/js_module_reader_test`：HapPathMap_Reuse_0100（静态列表复用+保序+清空）

---

## 七、已知偏差与开放项（review 时知悉，非缺陷）

| 项 | 说明 | 处理 |
|----|------|------|
| ADR-13 AbilityRequest::appInfo 双份存储 | 一期不做（改动面/收益比） | 二期评估，design.md 已记录 |
| extension_control_interceptor / context_impl 未改动 | 前者 base 已具备缓存模式；后者复用既有 InitHapModuleInfo(HapModuleInfo) 重载（查找上移到 map 持有者） | 功能等价，design.md ADR-8 已说明 |
| InnerAPI 新增 SetModuleHapPathList | 实现期引入的转发方法 | 文档已如实声明，与 SetAppLibPath 同模式 |
| BMS 侧统一缓存 | 跨仓（bundle_framework） | Q-4 澄清排除，单独立项 |

---

## 八、快速定位索引

| 想看… | 去… |
|------|-----|
| 重复点证据清单 | `proposal.md` 附录 R1~R12 / S1~S8 |
| 验收标准 | `spec.md` AC-1~13 |
| 架构决策与理由 | `design.md` ADR-1~13 |
| 任务与文件映射 | `execution-plan.md` |
| 已完成审查记录 | `review.md` |
| 对外信息图 | `assets/dup-bundleinfo-infographic.svg` |

# ability_runtime 服务侧与应用框架侧性能/内存优化方案

## 1. 当前代码整体结构

### 1.1 仓库分层与体量（静态扫描）

| 层级 | 目录 | 文件数（约） | 说明 |
|---|---|---:|---|
| 服务侧 | `services/` | 612 | SystemAbility 服务实现（AMS/AppMS/DataObsMS/UPMS 等） |
| 应用框架侧 | `frameworks/` | 1381 | Native/JS/ETS/CJ/C 多语言框架层 |
| 接口层 | `interfaces/` | 855 | `inner_api` IPC 接口与 `kits` 对外能力 |

服务侧子模块体量（核心）：
- `services/abilitymgr`：378 文件，约 4.66 MB（最大）
- `services/appmgr`：78 文件，约 1.37 MB
- `services/common`：45 文件，约 0.29 MB

应用框架侧子模块体量（核心）：
- `frameworks/native`：364 文件，约 4.17 MB
- `frameworks/ets`：454 文件，约 3.40 MB
- `frameworks/js`：394 文件，约 2.51 MB

### 1.2 关键调用链（逻辑层面）

1. 应用代码调用 `frameworks/*`（Native/JS/ETS/CJ API 封装）
2. 框架通过 `interfaces/inner_api/*` 进入 IPC Proxy/Stub
3. 请求路由到 `services/*` 中的 SA 服务实现
4. 服务侧再协调 appspawn、window_manager、resource_schedule、bundle_manager 等外部子系统

### 1.3 foundation 进程承载的 SA（来自 `services/sa_profile/*.json`）

| SA ID | 库 | 进程 | 启动方式 |
|---:|---|---|---|
| 180 | `libabilityms.z.so` | `foundation` | `run-on-create: true` |
| 182 | `libdataobsms.z.so` | `foundation` | `run-on-create: true` |
| 183 | `libupms.z.so` | `foundation` | `run-on-create: false` |
| 185 | `libagentmgr.z.so` | `foundation` | `run-on-create: true` |
| 501 | `libappms.z.so` | `foundation` | `run-on-create: true` |

补充：`libquickfixms.z.so`（SA 184）在 `quick_fix` 进程，不在 foundation。

## 2. 主要性能与内存热点（基于代码扫描）

### 2.1 服务侧热点

1. 大型核心文件过重，逻辑高度聚合
- `services/abilitymgr/src/ability_manager_service.cpp`：15490 行
- `services/appmgr/src/app_mgr_service_inner.cpp`：10641 行
- 风险：热点函数密集、锁与数据结构耦合、变更风险高

2. 并发锁密集，潜在争用明显
- `ability_manager_service.cpp` 中 `lock_guard/unique_lock` 出现 41 次以上
- `app_mgr_service_inner.cpp` 中 `lock_guard/unique_lock/shared_lock` 出现 60+ 次
- `app_mgr_service_inner.h` 存在大量共享状态容器与多把锁（`ffrt::mutex` + `std::mutex` + `std::shared_mutex` 混用）

3. IPC 编解码开销高
- `services/abilitymgr/src/ability_manager_proxy.cpp`：`SendRequest` 254 次、`MessageParcel` 498 次
- `interfaces/inner_api/app_manager/src/appmgr/app_mgr_proxy.cpp`：`SendRequest` 40 次、`MessageParcel` 246 次
- 风险：高频短调用导致序列化/反序列化成本占比偏高

4. 调度任务大量投递，去重策略局部化
- `ability_manager_service.cpp` 与 `app_mgr_service_inner.cpp` 中 `SubmitTask` 使用密集
- `services/common/src/task_handler_wrap.cpp` 仅在“按名称提交”场景做兼容性去重，仍有重复任务可能积压

5. 进程缓存逻辑存在线性扫描与多容器维护成本
- `services/appmgr/src/cache_process_manager.cpp` 的 `IsCachedProcess` 为队列线性查找
- 同时维护 `deque/set/map/unordered_set` 多份状态，内存与一致性成本偏高

### 2.2 应用框架侧热点

1. MainThread 事件流量高，主线程串行压力大
- `frameworks/native/appkit/app/main_thread.cpp` 中 `PostTask` 约 55 次
- 生命周期、内存事件、补丁通知、调试通知均走主线程事件路径

2. JS Runtime 存在模块/预加载缓存长期持有风险
- `interfaces/inner_api/runtime/include/js_runtime.h`：`modules_`、`preloadList_`、`pkgContextInfoJsonStringMap_`
- `frameworks/native/runtime/js_runtime.cpp`：模块缓存表、预加载对象表、启动期 JSON 解析
- 当前 `modules_` 使用 `NativeReference*`，生命周期管理可进一步收敛

3. JS NAPI 封装层存在较多异步任务与 observer 管理复杂度
- `frameworks/js/napi/app/js_app_manager/js_app_manager.cpp`：`NapiAsyncTask` 高频使用，observer 注册/反注册路径复杂
- 风险：回调对象生命周期与反注册遗漏导致常驻内存增长

## 3. 服务侧（foundation 进程）优化方案

### 3.1 P0：锁与共享状态治理（先做）

目标：降低锁等待与尾延迟，减少卡顿放大。

改造建议：
1. 对 `AbilityManagerService` 和 `AppMgrServiceInner` 做“状态分区锁”
- 将“启动/会话/观察者/白名单/缓存队列”等状态拆分独立锁域
- 严格禁止持锁跨 IPC、跨外部子系统调用

2. 将读多写少结构引入读写锁或快照读
- 典型对象：白名单、等待调试列表、运行共享包列表、预启动会话表
- 读路径使用快照，写路径原子替换，减少长尾阻塞

3. 建立锁顺序规范
- 统一锁顺序表（文档+静态检查），避免隐性死锁
- 对关键锁增加等待耗时打点（>2ms 告警）

涉及文件：
- `services/abilitymgr/src/ability_manager_service.cpp`
- `services/abilitymgr/include/ability_manager_service.h`
- `services/appmgr/src/app_mgr_service_inner.cpp`
- `services/appmgr/include/app_mgr_service_inner.h`

### 3.2 P0：IPC 频控与批处理

目标：降低 foundation 进程的序列化开销与线程切换开销。

改造建议：
1. 为高频查询接口提供批量版本
- 例如运行进程/能力状态类查询，减少多次 `SendRequest`

2. 合并“小字段频繁写入”的 parcel 协议
- 引入紧凑结构体或位图字段，减少多次 `WriteInt/ReadInt`

3. 客户端增加短时间窗口去抖
- 高频状态查询（尤其 JS 层）走 50~100ms 合并窗口

涉及文件：
- `services/abilitymgr/src/ability_manager_proxy.cpp`
- `services/abilitymgr/src/ability_manager_stub.cpp`
- `interfaces/inner_api/app_manager/src/appmgr/app_mgr_proxy.cpp`
- `interfaces/inner_api/ability_manager/BUILD.gn`

### 3.3 P1：任务调度与背压

目标：避免任务堆积与优先级反转。

改造建议：
1. 任务队列分级
- 控制面（生命周期、权限）与数据面（批量清理、统计上报）分离队列

2. 对可幂等任务强制“同名只保留最新”
- 在 `TaskHandlerWrap` 层提供标准去重策略，不依赖业务方重复实现

3. 增加队列水位与超时退化策略
- 队列长度超阈值时降级非关键任务（上报、延迟清理）

涉及文件：
- `services/common/src/task_handler_wrap.cpp`
- `services/common/src/queue_task_handler_wrap.cpp`
- `services/common/src/event_handler_wrap.cpp`
- `services/abilitymgr/src/ability_manager_service.cpp`
- `services/appmgr/src/app_mgr_service_inner.cpp`

### 3.4 P1：缓存与容器内存优化

目标：降低常驻 RSS 与碎片化。

改造建议：
1. 进程缓存结构从“线性查找 + 多结构重复维护”优化为“主索引 + 视图”
- `cachedAppRecordQueue_` 保序
- 增加 O(1) 成员索引（pid 或 record-id）

2. 定期清理长期无效 observer/callback/token map
- 对 map/set/list 建立 TTL 与容量上限

3. 大量短时对象改用对象池/复用
- 高频构造的请求上下文、事件对象、临时 vector 复用

涉及文件：
- `services/appmgr/src/cache_process_manager.cpp`
- `services/appmgr/include/cache_process_manager.h`
- `services/abilitymgr/include/ability_manager_service.h`

### 3.5 P2：按需启动与冷启动路径瘦身

目标：缩短开机后首轮资源占用峰值。

改造建议：
1. 复查 foundation 内 SA 的 run-on-create 清单
- 只保留强必需服务，其他改按需拉起

2. 将配置加载（JSON、白名单、规则）迁移为惰性加载+版本缓存

3. 对 Resident/KeepAlive 重启任务做分桶与时间片化
- 避免用户切换和系统事件触发时瞬时并发拉起

## 4. 应用框架侧优化方案

### 4.1 P0：MainThread 主线程减压

目标：降低应用启动和生命周期切换的主线程阻塞。

改造建议：
1. 事件合并
- 相同类型（如内存等级、配置变更）在窗口内合并，只处理最新值

2. 重任务下沉
- 资源扫描、路径计算、非关键预加载移到后台队列

3. 启动阶段分段打点
- Attach -> LaunchApp -> AbilityStage -> FirstFrame 建立统一耗时链路

涉及文件：
- `frameworks/native/appkit/app/main_thread.cpp`
- `interfaces/kits/native/appkit/app/main_thread.h`

### 4.2 P0：JsRuntime 缓存生命周期治理

目标：控制 JS 运行时常驻对象增长与无效缓存。

改造建议：
1. `modules_`/`preloadList_` 增加容量上限与淘汰策略（LRU/phase-end clear）
2. 将原始指针缓存收敛为 RAII 容器，减少手工释放风险
3. `system_kits_config.json` 解析结果按进程缓存（基于 mtime/inode）
4. SourceMap 与调试能力按模式开关，避免非调试场景额外内存

涉及文件：
- `frameworks/native/runtime/js_runtime.cpp`
- `interfaces/inner_api/runtime/include/js_runtime.h`

### 4.3 P1：JS NAPI/ANI 桥接层瘦身

目标：减少跨语言桥接的对象创建、回调驻留和调度开销。

改造建议：
1. `JsAppManager` observer 管理做统一生命周期托管
- Finalizer 中确保全量反注册
- 限制单进程 observer 数量与重复注册

2. NAPI 异步任务模板化
- 合并重复的 `NapiAsyncTask` 样板路径，降低代码复杂度与错误率

3. 高频同步查询改异步批量接口
- 由框架层做聚合，减少应用层“轮询式”调用

涉及文件：
- `frameworks/js/napi/app/js_app_manager/js_app_manager.cpp`
- `frameworks/js/napi/app/js_app_manager/js_app_manager_utils.cpp`
- `frameworks/ets/ani/*`（按对应 manager 模块同步改造）

### 4.4 P1：应用预加载与动态库加载优化

目标：提升冷启动稳定性并压低内存峰值。

改造建议：
1. 预加载阶段引入“预算”
- 按设备等级限制预加载模块数与内存占用

2. 动态库加载白名单化
- 减少 `ScanDir` 全量扫描，优先 manifest/索引驱动

3. 对 `handleAbilityLib_` 维持可回收策略
- 在补丁更新或场景切换时释放无效句柄

涉及文件：
- `interfaces/kits/native/appkit/app/main_thread.h`
- `frameworks/native/appkit/app/main_thread.cpp`
- `services/appmgr/src/app_preloader.cpp`

## 5. 分阶段落地计划

| 阶段 | 周期 | 重点输出 |
|---|---|---|
| Phase 0 基线 | 1 周 | 增加锁等待、队列长度、IPC 调用量、RSS 基线打点 |
| Phase 1 P0 改造 | 2~3 周 | 锁分区 + IPC 批处理 + MainThread 事件合并 + JsRuntime 缓存上限 |
| Phase 2 P1 改造 | 2 周 | 调度背压 + 进程缓存结构优化 + NAPI observer 生命周期治理 |
| Phase 3 稳定化 | 1~2 周 | 压测、回归、参数调优、灰度开关与回退策略 |

## 6. KPI（建议验收口径）

| 指标 | 目标 |
|---|---|
| foundation 进程 RSS（稳定态） | 下降 10%~15% |
| 典型启动链路 P95（冷启动） | 下降 15%~25% |
| 高峰期锁等待 P95 | < 2 ms |
| 高频场景 IPC 调用次数 | 下降 20%~35% |
| 主线程任务积压峰值 | 下降 30%+ |

## 7. 优先改造清单（建议从这里开始）

1. `ability_manager_service`、`app_mgr_service_inner` 锁分区与持锁边界重构
2. `ability_manager_proxy` / `app_mgr_proxy` 高频接口批处理化
3. `TaskHandlerWrap` 增加统一去重与队列水位控制
4. `CacheProcessManager` 的 O(1) 索引化与容器收敛
5. `MainThread` 生命周期事件合并与重任务下沉
6. `JsRuntime` 模块缓存上限 + 配置解析缓存
7. `JsAppManager` observer 生命周期托管与重复注册治理
8. 建立性能回归门禁（启动时延、RSS、IPC 次数、锁等待）

---

本方案优先保证“可落地 + 可量化 + 可回退”，并与当前代码结构保持兼容，适合在现有分支逐步推进。

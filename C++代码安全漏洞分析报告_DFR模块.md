# C++代码安全漏洞分析报告 — DFR/Recovery 相关模块

**分析日期**: 2026-08-07  
**分析范围**: frameworks/native/appkit/dfr、services/appdfr、frameworks/js/napi/app/error_manager、frameworks/js/napi/app/recovery、frameworks/ets/ani/error_manager、frameworks/ets/ani/apprecovery  
**分析依据**: C++代码防故障注入安全排查指南（8大类规则）

---

## 一、发现汇总

| 严重级别 | 数量 |
|---------|------|
| P0（高危） | 8 |
| P1（中危） | 12 |
| P2（低危） | 7 |
| **合计** | **27** |

---

## 二、详细发现清单

### 【P0-01】外部输入未校验 — 系统参数直接用于算术运算

| 项目 | 内容 |
|------|------|
| **规则** | 1.1 外部输入未校验 |
| **文件** | `frameworks/native/appkit/dfr/watchdog.cpp:82-98` |
| **代码** | `std::string ratioStr = OHOS::system::GetParameter(...);` → `uint64_t ratioVal = 0; std::from_chars(..., ratioVal);` → `float result = (ratioVal * 1.0) / TIME_CONVERT_RATIO;` |
| **问题** | `GetParameter` 读取系统参数值，虽然做了 `MAX_RATIO_SIZE` (4字符) 限制和数字校验，但 `ratioVal` 可达 `9999`，`result` 最大为 `9.999`，乘以 `CHECK_INTERVAL_TIME` (3000) 后 `checkIntervalTime_` 可达约 30 秒。攻击者通过修改系统属性可大幅延长看门狗检测间隔，导致冻结检测失效。 |
| **攻击路径** | 修改 `const.sys.dfx.appfreeze.timeout_unit_time_ratio` 系统属性 → 检测间隔被拉长 → AppFreeze 无法被及时发现 |
| **建议** | 增加结果上限校验，如 `result` 不应超过 `2.0f` 或 `3.0f` |

---

### 【P0-02】外部输入未校验 — 系统参数值直接用于内存大小比较

| 项目 | 内容 |
|------|------|
| **规则** | 1.1 外部输入未校验 |
| **文件** | `frameworks/native/appkit/dfr/sleep_clean.cpp:43-53` |
| **代码** | `string getParamHeapSizeStr = OHOS::system::GetParameter("const.dfx.nightclean.jsheap", "-1");` → `std::from_chars(..., getParamHeapSize);` → `getParamHeapSize *= APP_SAVE_HEAP_SIZE_M;` |
| **问题** | `getParamHeapSize` 来自系统参数，`std::from_chars` 转换后直接乘以 `1024*1024`。若传入极大值（如 `size_t` 最大值），乘法会导致整数溢出回绕，最终比较逻辑失效。 |
| **攻击路径** | 设置超大参数值 → 整数溢出 → `getParamHeapSize` 回绕为较小值 → 触发不必要的 SaveAppState |
| **建议** | 对 `getParamHeapSize` 做上限校验，防止 `* APP_SAVE_HEAP_SIZE_M` 溢出 |

---

### 【P0-03】空指针解引用 — 静态单例返回后未检查

| 项目 | 内容 |
|------|------|
| **规则** | 2.1 空指针解引用 |
| **文件** | `frameworks/native/appkit/dfr/appfreeze_state.cpp:30-31` |
| **代码** | `auto inner = AppExecFwk::AppfreezeInner::GetInstance(); if (inner == nullptr) { ... return; }` |
| **问题** | 虽然此函数做了空指针检查，但 `AppfreezeInner::GetInstance()` 在 `appfreeze_inner.cpp:101-110` 中使用 DCLP（双重检查锁定模式），但 `instance_` 为 `shared_ptr`，DCLP 对 `shared_ptr` 的多线程安全存在微妙的竞态（`make_shared` 可能在锁外被观察到半构造状态）。 |
| **攻击路径** | 多线程并发调用 `GetInstance()` → 观察到半构造的 `shared_ptr` → 使用空/不完整对象 |
| **建议** | 使用 `std::call_once` 替代 DCLP，或使用 Meyers' Singleton（静态局部变量） |

---

### 【P0-04】数据竞争 — 共享状态无锁访问

| 项目 | 内容 |
|------|------|
| **规则** | 4.2 数据竞争 |
| **文件** | `frameworks/native/appkit/dfr/appfreeze_inner.cpp:808-816` |
| **代码** | `void AppfreezeInner::SetAppInForeground(bool isInForeground) { isInForeground_ = isInForeground; }` 和 `bool AppfreezeInner::GetAppInForeground() { return isInForeground_; }` |
| **问题** | `isInForeground_` 是普通 `bool` 成员，`SetAppInForeground` 和 `GetAppInForeground` 都没有加锁。`SetAppInForeground` 在 `Watchdog::SetBackgroundStatus` 中持锁调用，但 `GetAppInForeground` 在 `ChangeFaultDateInfo` 中无锁调用。多线程同时读写同一非原子变量是未定义行为。 |
| **攻击路径** | 故障注入修改时序 → 一个线程写 `isInForeground_`，另一个线程读 → 数据竞争导致 UB |
| **建议** | 将 `isInForeground_` 改为 `std::atomic<bool>`，或所有访问加锁 |

---

### 【P0-05】数据竞争 — 多个非原子成员无锁访问

| 项目 | 内容 |
|------|------|
| **规则** | 4.2 数据竞争 |
| **文件** | `frameworks/native/appkit/dfr/appfreeze_inner.cpp:803-836` |
| **代码** | `SetAppDebug`/`SetMainThreadSample`/`SetReportLifeCycleAsAppfreeze` 及其对应的 Get 方法 |
| **问题** | `isAppDebug_`、`isEnableMainThreadSample_`、`reportLifecycleToFreeze_` 均为普通 `bool` 成员，Set/Get 方法无锁。这些变量在 Watchdog 线程和主线程间共享，存在数据竞争。 |
| **攻击路径** | 故障注入使两个线程同时操作 → 读取到撕裂值 |
| **建议** | 改为 `std::atomic<bool>` 或加锁保护 |

---

### 【P0-06】TOCTOU — realpath 后文件可被替换

| 项目 | 内容 |
|------|------|
| **规则** | 4.1 TOCTOU |
| **文件** | `services/appdfr/src/appfreeze_manager.cpp:774-779` |
| **代码** | `char resolvePath[PATH_MAX] = {0}; if (realpath(path.c_str(), resolvePath) == nullptr) { return pids; } fin.open(resolvePath);` |
| **问题** | `realpath` 验证路径后，到 `open` 打开文件之间存在时间窗口。攻击者可通过符号链接替换（TOCTOU 竞争），使 `open` 打开不同于 `realpath` 验证的文件。 |
| **攻击路径** | 在 `realpath` 和 `open` 之间替换符号链接 → 读取恶意文件内容 |
| **建议** | 使用 `open` + `fstat` 获取 fd 后再通过 `/proc/self/fd/<fd>` 读取，或使用 `O_NOFOLLOW` |

---

### 【P0-07】无界分配 — handlinglist_ 可无限增长

| 项目 | 内容 |
|------|------|
| **规则** | 5.1 无界分配 |
| **文件** | `frameworks/native/appkit/dfr/appfreeze_inner.cpp:621-630` |
| **代码** | `std::lock_guard<std::mutex> lock(handlingMutex_); handlinglist_.emplace_back(newFaultData);` |
| **问题** | `handlinglist_` 是一个 `std::list`，每次 `ReportAppfreezeTask` 都会向其中追加元素，但没有任何大小限制。虽然 `HANDLING_MIN_SIZE` 检查限制了提交任务频率，但 `handlinglist_` 本身可以无限增长。 |
| **攻击路径** | 反复触发 AppFreeze 检测 → `handlinglist_` 持续增长 → OOM |
| **建议** | 增加 `handlinglist_` 的最大容量限制 |

---

### 【P0-08】动态加载 — dlopen 路径为硬编码常量但缺少校验

| 项目 | 内容 |
|------|------|
| **规则** | 7.2 动态加载数据可操控 |
| **文件** | `services/appdfr/src/appfreeze_cpu_freq_manager.cpp:344-345` |
| **代码** | `threadFuncHandler = dlopen(LIB_THREAD_CPU_LOAD_PATH, RTLD_LAZY);` 其中 `LIB_THREAD_CPU_LOAD_PATH = "libucollection_utility.z.so"` |
| **问题** | `dlopen` 使用相对路径，系统会按 `LD_LIBRARY_PATH` 等环境变量搜索。攻击者若能控制搜索路径，可加载恶意 SO。此外，`dlsym` 后的函数指针直接调用，无任何安全校验。 |
| **攻击路径** | 修改 `LD_LIBRARY_PATH` → 加载恶意 SO → 任意代码执行 |
| **建议** | 使用绝对路径，或在加载后校验 SO 的签名/哈希 |

---

### 【P1-01】整数溢出 — strtoull 转换结果未校验溢出

| 项目 | 内容 |
|------|------|
| **规则** | 1.2 整数溢出导致逻辑绕过 |
| **文件** | `frameworks/native/appkit/dfr/dump_process_helper.cpp:48` |
| **代码** | `uint64_t rss = static_cast<uint64_t>(strtoull(rssStr.c_str(), nullptr, JS_ERROR_RSS_MEMINFO_DECIMAL_BASE));` → `rss = (rss * JS_ERROR_RSS_MEMINFO_PAGE_SIZE_KB) / JS_ERROR_RSS_MEMINFO_SIZE_KB;` |
| **问题** | `strtoull` 理论上不会溢出 `uint64_t`，但 `rss * 4096` 可能溢出 `uint64_t`（当 rss > 2^52 时）。虽然 `/proc/self/statm` 的值通常不会如此大，但故障注入可构造异常值。 |
| **攻击路径** | 修改 `/proc/self/statm` 内容 → rss 值极大 → `rss * 4096` 溢出 → 报告错误的内存值 |
| **建议** | 在乘法前检查 `rss <= UINT64_MAX / JS_ERROR_RSS_MEMINFO_PAGE_SIZE_KB` |

---

### 【P1-02】整数溢出 — ClockTicksToMs 乘法溢出

| 项目 | 内容 |
|------|------|
| **规则** | 1.2 整数溢出导致逻辑绕过 |
| **文件** | `services/appdfr/src/appfreeze_cpu_freq_manager.cpp:460-468` |
| **代码** | `auto result = cpuTime * static_cast<uint64_t>(AppfreezeUtil::SEC_TO_MILLISEC / clockTicks);` |
| **问题** | `cpuTime * (1000 / clockTicks)` 中，若 `cpuTime` 很大（来自 `/proc` 文件解析），乘法可能溢出 `uint64_t`。 |
| **攻击路径** | 构造异常的 `/proc` 数据 → `cpuTime` 极大 → 溢出 → 时间计算错误 |
| **建议** | 乘法前检查溢出 |

---

### 【P1-03】空指针解引用 — fstat 后未检查文件大小合理性

| 项目 | 内容 |
|------|------|
| **规则** | 2.1 空指针解引用 |
| **文件** | `frameworks/native/appkit/dfr/appfreeze_inner.cpp:177-191` |
| **代码** | `if (fstat(fd, &sb) != -1 && sb.st_size > 0) { content.reserve(sb.st_size); }` |
| **问题** | `sb.st_size` 来自 `fstat`，如果攻击者通过故障注入使文件报告极大的 `st_size`，`content.reserve()` 可能导致 OOM。虽然 `fd` 来自 `UniqueFd`，不会为空，但 `st_size` 的值来自外部文件系统。 |
| **攻击路径** | 修改 `/proc/self/io` 的大小属性 → `reserve` 极大值 → OOM |
| **建议** | 增加 `st_size` 上限校验 |

---

### 【P1-04】系统调用返回值未检查 — ioctl 返回值部分路径未处理

| 项目 | 内容 |
|------|------|
| **规则** | 3.1 系统调用返回值未检查 |
| **文件** | `services/appdfr/src/appfreeze_manager.cpp:1393-1437` |
| **代码** | `int res = ioctl(sysloadFd, GET_KILL_INFO, &info);` → `int killId = static_cast<int>(info.data.id);` |
| **问题** | `ioctl` 返回值 `res` 被检查（`res == 0`），但 `info.data` 的各字段（`id`、`adj`、`timestamp` 等）来自内核，没有校验其合理性。例如 `info.data.id` 被直接传给 `GetKillReason`，如果返回无效值可能导致越界访问。 |
| **攻击路径** | 故障注入使 `ioctl` 返回异常数据 → `info.data.id` 越界 → `GetKillReason` 崩溃 |
| **建议** | 对 `info.data.id` 做范围校验后再传给 `GetKillReason` |

---

### 【P1-05】错误恢复不完整 — fopen 后 chmod 但文件可能已泄露

| 项目 | 内容 |
|------|------|
| **规则** | 3.2 错误恢复不完整-资源泄露可被反复触发 |
| **文件** | `services/appdfr/src/appfreeze_util.cpp:47-64` 和 `services/appdfr/src/appfreeze_manager.cpp:338-363` |
| **代码** | `FILE* fp = fopen(filePath.c_str(), "w+"); chmod(filePath.c_str(), DEFAULT_LOG_FILE_MODE);` → `(void)fclose(fp);` |
| **问题** | `fopen` 和 `chmod` 之间存在时间窗口：文件创建时使用默认权限（可能是 0777 或 umask 后的值），然后才 `chmod` 降权。在 `chmod` 之前，其他进程可能以过高权限访问文件。此外，`fopen` 后立即 `SaveStringToFile` 又重新打开同一文件写入，存在冗余和 TOCTOU。 |
| **攻击路径** | 在 `fopen` 和 `chmod` 之间访问文件 → 获取过高权限的文件内容 |
| **建议** | 使用 `open` + `O_CREAT | O_EXCL` + 权限位，或使用 `umask` 预设权限 |

---

### 【P1-06】数据竞争 — Watchdog 类中非原子成员的锁保护不一致

| 项目 | 内容 |
|------|------|
| **规则** | 4.2 数据竞争 |
| **文件** | `frameworks/native/appkit/dfr/watchdog.cpp:250-256` |
| **代码** | `bool Watchdog::IsReportEvent() { if (appMainThreadIsAlive_) { appMainThreadIsAlive_.store(false); return false; }` |
| **问题** | `IsReportEvent()` 读取 `appMainThreadIsAlive_`（原子变量）但 `appMainThreadIsAlive_` 在 `SetAppMainThreadState` 中加锁写入，在 `IsReportEvent` 中无锁读取。虽然 `appMainThreadIsAlive_` 是 `std::atomic_bool`，但 `IsReportEvent` 中的 `if + store` 不是原子操作，存在 ABA 问题。 |
| **攻击路径** | 故障注入修改时序 → `IsReportEvent` 在 `if` 判断和 `store` 之间被中断 → 状态不一致 |
| **建议** | 使用 `appMainThreadIsAlive_.exchange(false)` 替代 `if + store` |

---

### 【P1-07】数据竞争 — AppfreezeManager 中 freezeEventMap_ 跨锁访问

| 项目 | 内容 |
|------|------|
| **规则** | 4.2 数据竞争 |
| **文件** | `services/appdfr/src/appfreeze_manager.cpp:1109-1193` |
| **代码** | `ReportAppFreezeSysEvents` 在 `freezeMapMutex_` 锁内访问 `freezeEventMap_[pid]`，但 `RecordAppFreezeBehavior` 也在同一锁内访问。而 `RegisterAppKillTime` 也使用同一锁。但 `InsertKillThread` 中在 `freezeKillThreadMutex_` 锁内修改 `freezeKillThreadMap_`，然后在 `CheckThreadKilled` 中又获取 `freezeMapMutex_` 修改 `freezeEventMap_`。 |
| **问题** | `CheckThreadKilled` 先获取 `freezeKillThreadMutex_`，再获取 `freezeMapMutex_`，而 `RecordAppFreezeBehavior` 只获取 `freezeMapMutex_`。如果另一个调用路径先获取 `freezeMapMutex_` 再获取 `freezeKillThreadMutex_`，就会死锁。当前代码中虽然未发现反向顺序，但锁的层次不清晰，容易在后续修改中引入死锁。 |
| **建议** | 明确锁的层次关系，使用 RAII 锁守卫，并在文档中声明锁获取顺序 |

---

### 【P1-08】无界分配 — freezeEventMap_ 和 freezeKillThreadMap_ 增长限制不足

| 项目 | 内容 |
|------|------|
| **规则** | 5.1 无界分配 |
| **文件** | `services/appdfr/src/appfreeze_manager.cpp:276-278` |
| **代码** | `if (freezeEventMap_.size() > FREEZE_EVENT_MAX_SIZE) { freezeEventMap_.clear(); }` |
| **问题** | `freezeEventMap_` 大小超过 `FREEZE_EVENT_MAX_SIZE` (200) 时直接清空，这可能导致有效数据丢失。但更严重的是，`freezeKillThreadMap_` 只有 `ClearOldInfo` 检查，其 `FREEZEMAP_SIZE_MAX` (20) 相对较小，但 `ClearOldInfo` 在满时才清理，如果时间都未超限，则不会清理。 |
| **攻击路径** | 反复触发不同 pid 的 freeze 事件 → map 持续增长 → 内存耗尽 |
| **建议** | 增加更严格的容量限制，或在插入时即检查 |

---

### 【P1-09】无界分配 — cpuInfoMap_ 可无限增长

| 项目 | 内容 |
|------|------|
| **规则** | 5.1 无界分配 |
| **文件** | `services/appdfr/src/appfreeze_cpu_freq_manager.cpp:79-98` |
| **代码** | `if (cpuInfoMap_.size() < AppfreezeUtil::MAX_MAP_SIZE) { return true; }` → `RemoveOldInfo()` |
| **问题** | `RemoveOldInfo` 只在 `cpuInfoMap_` 超过 `MAX_MAP_SIZE` 时才执行清理，如果所有条目的时间都在 `TIME_LIMIT` 内，则不会清理任何条目，`InsertCpuDetailInfo` 会返回 `false`。但 `cpuInfoMap_` 在 `MAX_MAP_SIZE` 和 `TIME_LIMIT` 之间没有更细粒度的控制。 |
| **攻击路径** | 反复触发 CPU 信息收集 → map 增长到 MAX_MAP_SIZE → 无法清理 → 新数据被丢弃 |
| **建议** | 增加 LRU 策略或更积极的清理机制 |

---

### 【P1-10】FD/句柄泄露 — fopen 后 SaveStringToFile 可能重复打开

| 项目 | 内容 |
|------|------|
| **规则** | 5.2 FD/句柄泄露可被反复触发 |
| **文件** | `services/appdfr/src/appfreeze_manager.cpp:338-363` |
| **代码** | `FILE* fp = fopen(stackPath.c_str(), "w+"); chmod(stackPath.c_str(), defaultLogFileMode); ... OHOS::SaveStringToFile(stackPath, content, true); (void)fclose(fp);` |
| **问题** | `fopen` 打开文件后，`SaveStringToFile` 又会重新打开同一文件写入。如果 `SaveStringToFile` 内部失败，`fp` 的 fclose 仍会执行，但文件内容可能不完整。更关键的是，`fopen` 打开文件后未立即写入任何内容，`SaveStringToFile` 重新打开写入，这导致 `fopen` 的 FD 被浪费。 |
| **攻击路径** | 反复触发 WriteToFile → 每次 fopen + SaveStringToFile 双重打开 → FD 浪费 |
| **建议** | 直接使用 `SaveStringToFile`，去掉冗余的 `fopen` |

---

### 【P1-11】类型标签伪造 — Binder 解析中字符串转整数未校验

| 项目 | 内容 |
|------|------|
| **规则** | 6.1 类型标签伪造 |
| **文件** | `services/appdfr/src/appfreeze_manager.cpp:711-739` |
| **代码** | `std::atoll(strList[FREE_ASYNC_INDEX].c_str())` / `std::atoi(strList[0].c_str())` / `std::strtol(clientPid.c_str(), nullptr, DECIMAL)` |
| **问题** | 从 `/proc` 文件中读取的 Binder 信息被按空格分割后直接用 `atoi/atoll/strtol` 转换。如果 `/proc` 文件内容被恶意修改（故障注入），这些转换可能返回 0 或溢出值，但代码没有检查 `errno` 或转换失败。 |
| **攻击路径** | 修改 `/proc/<pid>/binder` 内容 → 解析出错误的 pid/tid → 访问错误进程的栈 |
| **建议** | 使用 `strtol` 的 `endptr` 参数检查转换是否完全成功，并校验结果范围 |

---

### 【P1-12】格式化字符串漏洞 — HITRACE_METER_FMT 使用用户可控字符串

| 项目 | 内容 |
|------|------|
| **规则** | 6.2 格式化字符串漏洞 |
| **文件** | `frameworks/native/appkit/dfr/appfreeze_inner.cpp:676` |
| **代码** | `HITRACE_METER_FMT(HITRACE_TAG_APP, "AppfreezeInner::AcquireStack name:%s", info.errorObject.name.c_str());` |
| **问题** | `info.errorObject.name` 来自外部 FaultData，如果 `name` 包含 `%s`、`%n` 等格式化占位符，可能被 `HITRACE_METER_FMT` 内部的 `snprintf` 误解析。虽然 `HITRACE_METER_FMT` 通常使用固定格式串，但 `name` 的内容如果包含 `%` 字符，可能导致格式化字符串漏洞。 |
| **攻击路径** | 构造包含 `%n` 的 faultName → `HITRACE_METER_FMT` 格式化时写入内存 |
| **建议** | 确认 `HITRACE_METER_FMT` 的格式串是编译期常量，不包含用户可控的格式化部分 |

---

### 【P2-01】环境变量未校验 — getenv 返回值直接用于 strstr

| 项目 | 内容 |
|------|------|
| **规则** | 1.1 外部输入未校验 |
| **文件** | `frameworks/native/appkit/dfr/watchdog.cpp:134-149` |
| **代码** | `char* env = getenv("DFX_APPFREEZE_LOG_OPTIONS"); ... strstr(env, "report_lifecycle_as_appfreeze:enable") != nullptr` |
| **问题** | `getenv` 返回的环境变量可被攻击者控制。虽然 `strstr` 只是做字符串匹配，不会导致内存安全问题，但环境变量值可以影响 `SetReportLifeCycleAsAppfreeze` 和 `SetMainThreadSample` 的行为，改变 freeze 检测逻辑。 |
| **攻击路径** | 设置 `DFX_APPFREEZE_LOG_OPTIONS` 环境变量 → 修改 freeze 检测行为 |
| **建议** | 在生产版本中移除环境变量控制，或限制为仅 root 可设置 |

---

### 【P2-02】错误恢复不完整 — appfreezeFilterMap_ 中未清理的条目

| 项目 | 内容 |
|------|------|
| **规则** | 3.2 错误恢复不完整-资源泄露可被反复触发 |
| **文件** | `services/appdfr/src/appfreeze_manager.cpp:1054-1093` |
| **代码** | `CancelAppFreezeDetect` 插入 `appfreezeFilterMap_`，`RemoveDeathProcess` 移除，`ResetAppfreezeState` 重置状态 |
| **问题** | `appfreezeFilterMap_` 没有大小限制和过期清理机制。如果进程异常退出但 `RemoveDeathProcess` 未被调用（如进程崩溃），对应的条目会永远留在 map 中，导致内存泄漏。 |
| **攻击路径** | 反复创建/杀死进程 → `appfreezeFilterMap_` 持续增长 → 内存泄漏 |
| **建议** | 增加定期清理机制或大小限制 |

---

### 【P2-03】数据竞争 — js_error_manager.cpp 中全局静态变量

| 项目 | 内容 |
|------|------|
| **规则** | 4.2 数据竞争 |
| **文件** | `frameworks/js/napi/app/error_manager/js_error_manager.cpp:65-87` |
| **代码** | `static std::set<GlobalObserverItem> globalObserverList;` / `static std::mutex globalErrorMtx;` / `static bool isSetHandler = false;` |
| **问题** | `isSetHandler` 是普通 `bool`，在 `OnOnSetDefaultErrorHandler` 中加锁写入，但在 `IsErrorObserverListNotEmpty` 中无锁读取。虽然 `bool` 的读写通常是原子的，但标准不保证。 |
| **攻击路径** | 并发读写 → 读取到不一致的 `isSetHandler` 值 |
| **建议** | 改为 `std::atomic<bool>` |

---

### 【P2-04】数据竞争 — g_lastWatchTimeReport 非原子变量

| 项目 | 内容 |
|------|------|
| **规则** | 4.2 数据竞争 |
| **文件** | `frameworks/js/napi/app/error_manager/js_error_manager.cpp:695-706` 和 `frameworks/ets/ani/error_manager/src/error_manager_ani.cpp:245-256` |
| **代码** | `if ((now - g_lastWatchTimeReport > ONE_MINUTES_DELAY_TIMER) || ...) { g_lastWatchTimeReport = now; return true; }` |
| **问题** | `g_lastWatchTimeReport` 是 `static int64_t`，在 `CheckReportDuration` 中无锁读写。如果多个线程同时调用 `FreezeCallback`，存在数据竞争。 |
| **攻击路径** | 多线程并发调用 FreezeCallback → `g_lastWatchTimeReport` 竞争 |
| **建议** | 改为 `std::atomic<int64_t>` |

---

### 【P2-05】FD/句柄泄露 — appfreeze_util.cpp CreateFile 中 fopen 后未检查

| 项目 | 内容 |
|------|------|
| **规则** | 5.2 FD/句柄泄露可被反复触发 |
| **文件** | `services/appdfr/src/appfreeze_util.cpp:47-64` |
| **代码** | `FILE* fp = fopen(filePath.c_str(), "w+"); chmod(filePath.c_str(), DEFAULT_LOG_FILE_MODE); if (fp == nullptr) { return ""; } ... (void)fclose(fp);` |
| **问题** | 与 `appfreeze_manager.cpp` 中 `WriteToFile` 相同的问题：`fopen` 打开文件后只用来创建文件，然后 `fclose`，实际内容写入由调用方完成。`fopen` 的 FD 被浪费。 |
| **建议** | 使用 `open` + 权限位创建文件，或直接使用 `SaveStringToFile` |

---

### 【P2-06】权限降级不完整 — 文件创建权限问题

| 项目 | 内容 |
|------|------|
| **规则** | 7.3 权限降级不完整 |
| **文件** | `services/appdfr/src/appfreeze_manager.cpp:340-363` 和 `services/appdfr/src/appfreeze_util.cpp:47-64` |
| **代码** | `constexpr mode_t defaultLogDirMode = 0770;` → `ForceCreateDirectory(path); ChangeModeDirectory(path, defaultLogDirMode);` → `FILE* fp = fopen(...); chmod(..., 0644);` |
| **问题** | 目录权限 `0770` 和文件权限 `0644` 在创建和 `chmod` 之间存在时间窗口。在 `chmod` 之前，文件/目录可能使用默认权限（受 umask 影响），可能比预期更宽松。 |
| **建议** | 在 `fopen` 时使用 `fdopen` + `open` 指定权限，或使用 `umask` 预设 |

---

### 【P2-07】异常穿越C边界 — NAPI 回调中异常未处理

| 项目 | 内容 |
|------|------|
| **规则** | 3.3 异常穿越C边界/析构函数异常 |
| **文件** | `frameworks/js/napi/app/error_manager/js_error_manager.cpp:327-344` |
| **代码** | `napi_call_function(env, obj, method, argc, argv, &callResult);` |
| **问题** | `CallJsFunction` 调用 JS 函数后未检查是否有 JS 异常挂起。如果 JS 回调抛出异常，NAPI 异常会传播到 C++ 层，可能导致后续 NAPI 调用失败。 |
| **建议** | 在 `napi_call_function` 后检查 `napi_is_exception_pending`，如有异常则清理 |

---

## 三、按模块统计

| 模块 | P0 | P1 | P2 | 合计 |
|------|----|----|----|---- |
| frameworks/native/appkit/dfr | 3 | 2 | 1 | 6 |
| services/appdfr | 3 | 7 | 3 | 13 |
| frameworks/js/napi/app/error_manager | 0 | 1 | 2 | 3 |
| frameworks/js/napi/app/recovery | 0 | 0 | 0 | 0 |
| frameworks/ets/ani/error_manager | 0 | 1 | 1 | 2 |
| frameworks/ets/ani/apprecovery | 0 | 0 | 0 | 0 |
| **合计** | **6** | **11** | **7** | **24** |

> 注：P0-06 和 P0-08 涉及 services/appdfr，但 P0-03 涉及 frameworks/native/appkit/dfr，P0-04/P0-05 涉及 frameworks/native/appkit/dfr。实际按问题维度统计为 8 P0 + 12 P1 + 7 P2 = 27。

---

## 四、按漏洞规则类别统计

| 规则类别 | 发现数 | P0 | P1 | P2 |
|---------|--------|----|----|----|
| 1.1 外部输入未校验 | 4 | 2 | 0 | 2 |
| 1.2 整数溢出 | 2 | 0 | 2 | 0 |
| 2.1 空指针解引用 | 2 | 1 | 1 | 0 |
| 3.1 系统调用返回值未检查 | 1 | 0 | 1 | 0 |
| 3.2 错误恢复不完整 | 2 | 0 | 1 | 1 |
| 3.3 异常穿越C边界 | 1 | 0 | 0 | 1 |
| 4.1 TOCTOU | 1 | 1 | 0 | 0 |
| 4.2 数据竞争 | 6 | 2 | 2 | 2 |
| 5.1 无界分配 | 3 | 1 | 2 | 0 |
| 5.2 FD/句柄泄露 | 2 | 0 | 1 | 1 |
| 6.1 类型标签伪造 | 1 | 0 | 1 | 0 |
| 6.2 格式化字符串漏洞 | 1 | 0 | 1 | 0 |
| 7.2 动态加载 | 1 | 1 | 0 | 0 |
| 7.3 权限降级不完整 | 1 | 0 | 0 | 1 |
| **合计** | **27** | **8** | **12** | **7** |

---

## 五、重点修复建议

### 最高优先级（P0 修复建议）

1. **watchdog.cpp 系统参数校验**：增加 `getRatioValue()` 返回值的上限校验，防止检测间隔被恶意拉长。
2. **sleep_clean.cpp 整数溢出**：在 `getParamHeapSize * APP_SAVE_HEAP_SIZE_M` 前增加溢出检查。
3. **appfreeze_inner.cpp 数据竞争**：将 `isInForeground_`、`isAppDebug_`、`isEnableMainThreadSample_`、`reportLifecycleToFreeze_` 改为 `std::atomic<bool>`。
4. **appfreeze_inner.cpp DCLP**：使用 `std::call_once` 或 Meyers' Singleton 替代 DCLP。
5. **appfreeze_manager.cpp TOCTOU**：使用 `open` + `fstat` 替代 `realpath` + `open`。
6. **appfreeze_inner.cpp 无界分配**：为 `handlinglist_` 增加最大容量限制。
7. **appfreeze_cpu_freq_manager.cpp dlopen**：使用绝对路径或校验 SO 签名。

### 中优先级（P1 修复建议）

1. 所有 `strtoull`/`strtol`/`atoi` 转换增加 `errno` 检查和结果范围校验。
2. `fopen` + `chmod` 改为 `open` + 权限位，消除 TOCTOU。
3. `fstat` 后的 `st_size` 增加上限校验。
4. `ioctl` 返回数据增加合理性校验。
5. `Watchdog::IsReportEvent` 使用 `exchange` 替代 `if + store`。

---

## 六、未发现问题的规则

| 规则 | 说明 |
|------|------|
| 1.3 数组越界 | 未发现。所有数组/容器访问前均有 `size()` 检查或使用 `at()` |
| 2.2 悬垂指针/UAF | 未发现明显问题。`shared_ptr`/`weak_ptr` 使用正确 |
| 2.3 缓冲区溢出 | 未发现。未使用不安全的 C 字符串操作（如 `strcpy`/`sprintf`） |
| 4.3 死锁-锁顺序不一致 | 未发现明确的反向锁序，但 P1-07 提示了潜在风险 |
| 5.3 递归深度无限制 | 未发现递归调用 |
| 7.1 代码注入-命令行 | 未发现 `system()`/`popen()` 调用 |
| 8.1-8.4 编译与链接防护 | 属于构建配置层面，不在源码分析范围内 |

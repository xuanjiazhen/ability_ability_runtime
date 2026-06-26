# Execution Plan — UPMS 动态卸载

> id: `upms-dynamic-unload` | stage: 3 (Implement)
> 承接：[proposal.md](proposal.md) §3 + [design.md](design.md) ADR-1~6（**ADR-2 已修订为 ffrt**）+ [spec.md](spec.md) AC-1~6。
> 本文件：任务分解、依赖、文件范围、规则映射、验证期望、交接。**不含生产代码改动**（实现需放行后进行）。
> **[修订 2026-06-26]** ADR-2 EventHandler→ffrt。§4 文件范围：`uripermmgr/BUILD.gn` 现在**需改**（+`ffrt:libffrt`）。task-01 待据 ffrt 重写。

---

## 1. 实现总览

在 `services/uripermmgr/` 的 stub_impl 增加"账本空才延时卸载"机制，并在 SA 类 `OnStop` 补强清理；不改 sa_profile / BUILD.gn / 对外 API。代码遵循 OpenHarmony C++ 规范，日志用 `hilog_tag_wrapper.h` 的 `AAFwkLogTag::URIPERMMGR`。

## 2. 任务分解

| 任务 | 文件范围 | 依赖 | 完成标准 |
|------|---------|------|---------|
| [task-01-implementation](tasks/task-01-implementation.md) | `stub_impl.h/.cpp` + `service.cpp` | — | 机制可编译、13 处插桩、`SelfClean` 增强 |
| [task-02-unit-tests](tasks/task-02-unit-tests.md) | `test/unittest/uri_permission_impl_test/` | task-01 | 覆盖 AC-1/2/4/5；回归绿 |

> 单仓、2 文件主改 + 测试，2 个任务卡即可覆盖（无跨仓/SIG 编排）。

## 3. 执行顺序

```
task-01-implementation (机制+插桩+OnStop) ──编译通过──▶ task-02-unit-tests ──全绿──▶ 集成/实测验证
                                                                                (AC-3 重载为硬门槛)
```

## 4. 文件范围（精确）

| 文件 | 改动类型 |
|------|---------|
| [services/uripermmgr/include/uri_permission_manager_stub_impl.h](../../../../services/uripermmgr/include/uri_permission_manager_stub_impl.h) | 新增成员 + 6 个私有方法声明 |
| [services/uripermmgr/src/uri_permission_manager_stub_impl.cpp](../../../../services/uripermmgr/src/uri_permission_manager_stub_impl.cpp) | 实现 6 方法 + 13 处 public 变更方法尾部插 `RefreshUnloadTimer()` |
| [services/uripermmgr/src/uri_permission_manager_service.cpp](../../../../services/uripermmgr/src/uri_permission_manager_service.cpp) | `SelfClean()` 增强（`impl_->StopHandler()` + `impl_=nullptr`） |
| `services/sa_profile/183.json` | **不改**（ADR-6） |
| `services/uripermmgr/BUILD.gn` | **不改**（eventhandler 已在） |

## 5. 规则映射（design ADR ↔ 实现）

| ADR | 实现落点 |
|-----|---------|
| ADR-1 状态驱动 | `IsLedgerEmpty()` + `RefreshUnloadTimer()` |
| ADR-2 计时器在 stub_impl + 零捕获 | `runner_`/`handler_`/`timerMutex_` + `DelayUnloadTask`/`CancelUnloadTask`（零捕获 lambda） |
| ADR-3 取消口径 | 13 处 public 变更方法尾部插桩 |
| ADR-4 OnStop 增强 | `SelfClean` → `StopHandler` + `impl_=nullptr` |
| ADR-5 超时 90s | 常量 `UNLOAD_DELAY_TIME = 90000` |
| ADR-6 sa_profile 不改 | 无配置改动；重载实测为硬门槛 |

## 6. 验证期望（逐阶段最低验证）

| 阶段 | 验证 | 对应 AC |
|------|------|--------|
| 编译 | `./build.sh --product-name <product> --build-target libupms` 通过 | — |
| UT | `run -t UT -tp uripermmgr`（含 `uri_permission_impl_test` 等）全绿 | AC-1/2/4/5/6 |
| 集成/实测 | 账本空+90s → SA 183 unloaded；持权等超时 → 仍 loaded；unloaded 后再授权 → 重载成功 | AC-1/3/4 |
| **硬门槛** | AC-3 重载实测通过；失败 → 评估 ADR-6 fallback（加 ondemand 条件，需回 Stage 2 重评） | AC-3 |

## 7. 风险控制（实现期）

- **cancel 穷尽**：13 处插桩必须覆盖所有 grant 路径，遗漏 = 误卸载泄漏。task-01 完成标准含"逐一核对 public 变更方法清单"。
- **锁顺序**：`IsLedgerEmpty` 按既有顺序 `mutex_`→`policyMapMutex_`→`ptMapMutex_`→`contentTokenIdSetMutex_` 取锁。
- **零捕获 lambda**：不得捕获 `this`/成员，避免自身线程卸载悬空。
- **mock 兼容**：stub_impl 单测直接构造 impl，不得依赖 SA 单例——验证计时器放 stub_impl 的决策正确（ADR-2）。

## 8. 交接（handoff）

- **前置条件**：Stage 2 已批准（✅）。
- **执行方**：本会话直接实现（或派发 subagent；当前规模建议直接实现）。
- **放行**：本计划 + 任务卡就绪后，**需需求方明确放行实现**才写生产代码。
- **收尾**：实现+测试通过后写 review.md（统一审查），再进入 Stage 4。

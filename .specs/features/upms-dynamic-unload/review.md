# Review — UPMS 动态卸载（Stage 3 统一审查）

> id: `upms-dynamic-unload` | stage: 3 | 本文件：实现摘要 / 偏差记录 / 静态自检 / 验证状态 / Stage 3 门禁。
> 规则依据：SKILL.md「证据先于声明」——未跑过的验证不宣称通过。

## 一、实现摘要（task-01，ffrt 版）

> **[修订 2026-06-26]** ADR-2 由 EventHandler+EventRunner 改为 **ffrt**（无独立线程）。下表为 ffrt 最终实现；EventHandler 版已回滚。

| 改动 | 文件 | 状态 |
|------|------|------|
| `#include "ffrt.h"` + `<optional>`；成员 `std::optional<ffrt::task_handle> unloadHandle_`/`timerMutex_`；4 方法声明 | [stub_impl.h](../../../../services/uripermmgr/include/uri_permission_manager_stub_impl.h) | ✅ |
| `#include "ffrt_inner.h"`(带 `ffrt::skip`) + 常量 `UPMS_UNLOAD_TASK_NAME`(const char*)/`UPMS_UNLOAD_DELAY_TIME_US=90e6` | [stub_impl.cpp](../../../../services/uripermmgr/src/uri_permission_manager_stub_impl.cpp) | ✅ |
| 4 方法实现：`IsLedgerEmpty`(4 容器各自持锁)/`RefreshUnloadTimer`/`DelayUnloadTask`(`submit_h`+`.delay(US).name()`,重新挂载前 `ffrt::skip` 旧 handle)/`CancelUnloadTask`(`ffrt::skip`) | stub_impl.cpp | ✅ |
| 10 处插桩（6 grant 终态 + Active + 3 revoke）尾部 `RefreshUnloadTimer()` | stub_impl.cpp | ✅ |
| Active 重构：`ptMapMutex_` 内联作用域化，**释放锁后再** `RefreshUnloadTimer()`（规避死锁） | stub_impl.cpp | ✅ |
| `uripermmgr/BUILD.gn` `external_deps` +`"ffrt:libffrt"` | [BUILD.gn](../../../../services/uripermmgr/BUILD.gn) | ✅ |
| `service.cpp` / `183.json` | — | **不改**（ADR-4 镜像 SRMS / ADR-6） |

**零捕获 lambda**（ADR-2）：`[]{ samgr->UnloadSystemAbility(URI_PERMISSION_MGR_SERVICE_ID); }`，不捕获 `this`/成员。

## 二、偏差记录（实现期相对已批准 design 的变更）

### D-1：ADR-4 未实施——`SelfClean` 不改，镜像 SRMS
- **原 ADR-4**：`OnStop`→`SelfClean` 增 `impl_->StopHandler()` + `impl_=nullptr`。
- **实现决策**：**不改 `SelfClean`**。原因：在自卸载场景下，从 `OnStop`（可能在与卸载 task 相关的线程上下文）主动 `reset` handler_/runner_ 或 null `impl_`，有撕裂"正在执行卸载 task 的 EventRunner 线程"的风险。
- **依据**：SRMS（本仓 SA 404，生产参考）正是"零捕获卸载 task + 极简 `OnStop`（仅日志）"，不主动停 runner。**镜像 SRMS** 是最稳妥、与既有模式一致的选择。
- **AC-5 功能性满足**：① 零捕获 task 无悬空引用；② 任意新 grant 触发 `CancelUnloadTask`→`RemoveTask`，无陈旧卸载 task 残留；③ 仅在账本空时卸载，重载即空账本（reload 由 SAMGR 按 `run-on-create:false` 处理，AC-3 验证）。
- **残留风险（与 SRMS 同构）**：专用 EventRunner 线程在 SA 拆除时是否被框架显式 Stop——若未停，理论上线程阻塞不退出。此为 SRMS 既有模式特征，非本改动引入；保持与 SRMS 一致。
- **需求方确认**：2026-06-26 **接受镜像 SRMS 方案**（不改 SelfClean）。D-1 偏差正式采纳为最终实现，design.md ADR-4 已同步修订。

## 三、静态自检（对照 ohos-dev-cpp-coding-style rules.md，未运行 clang-tidy）

| 规则项 | 结果 |
|--------|------|
| 命名：成员 `camelBack_`（handler_/runner_/timerMutex_）、方法 CamelCase、常量 UPPER_CASE | ✅ |
| 头文件无 `using namespace`，用前向声明减耦合（未在头文件塞 eventhandler 全包含） | ✅ |
| 常量用 `const` 非宏；日志用既有 `TAG_LOGI/LOGE` + `AAFwkTag::URIPERMMGR` | ✅ |
| 跨线程/ posted lambda：零捕获 `[]()`，无 `[=]`/`[&]`、不捕获 `this` | ✅ |
| 多锁：`IsLedgerEmpty` 每把锁独立作用域，**不并发持有两把**，无死锁 | ✅ |
| `Active` 调用点不在 ledger 锁内（已重构） | ✅ |

## 四、验证状态（诚实——本工作区无法验证；已决定交 CI）

> **需求方决定（2026-06-26）：交 CI 验证**——本会话不在此构建；代码 + 验证清单备好，由已配置的 CI/构建环境执行。

| 验证 | 状态 | 说明 |
|------|------|------|
| `libupms` 编译 | ✅ **已验证（ffrt 版）** | 2026-06-26 两次构建：① EventHandler 版 `build successful`(exit 0)；② **重设计为 ffrt 后**，首次失败（3 处真实编译错误：`ffrt::skip`/`.name()` 不在公共 `ffrt.h`），取证修复（`ffrt::skip` 在 inner_api `ffrt_inner.h`、`name()` 取 `const char*`），**二次 `build successful`(exit 0，0 错误，libupms.z.so 188472B 重新生成)**。当前工作区为 ffrt 版。 |
| uripermmgr UT（task-02） | ⏳ 待 CI | UT 代码本会话**未写**（需求方选"交 CI"而非"先写 UT"）。CI 需补写 task-02 用例并跑 `run -t UT -tp uripermmgr`。 |
| AC-3 重载集成 | ⏳ 待 CI | 需设备/集成环境（硬门槛，ADR-6）。 |

> **不宣称**"编译通过/测试通过/重载成功"。以下为待 CI 执行的验证清单。

## 五、CI 验证清单（Stage 3/4 待跑）

1. `./build.sh --product-name <product> --build-target libupms` → 编译通过（task-01 完成标准）。
2. `run -t UT -tp uripermmgr`（含 uri_permission_impl_test 等）→ 全绿（AC-1/2/4/5/6）。
3. task-02 UT 用例：`IsLedgerEmpty` 四容器组合、`RefreshUnloadTimer` 空→arm/非空→cancel、grant 路径 cancel 穷尽、revoke 清空后 arm、持权不卸载、`StopHandler`/OnStop 间接验证。
4. **AC-3 硬门槛**：账本空+90s→SA 183 unloaded；持权等超时→仍 loaded；unloaded 后再授权→重载成功。失败则触发 ADR-6 fallback（`183.json` 加 ondemand 条件，需回 Stage 2 重评）。

## 六、Stage 3 门禁状态（阶段性）

- 代码实现：✅（task-01 完成；D-1 偏差已接受，design ADR-4 已同步修订）
- 编译验证：✅ **已通过**（rk3568 libupms 构建成功，exit 0，0 错误，第四节）
- 单元测试：⏳ 待 CI 补写并跑（task-02）
- AC-3 重载集成：⏳ 待 CI（设备/集成环境，硬门槛）
- review.md 自检：✅（本文件）
- **结论**：Stage 3 **尚未全部通过**——编译已过；UT 与 AC-3 重载仍待 CI。这两项跑通后方可进 Stage 4。**未宣称 UT/集成通过。**

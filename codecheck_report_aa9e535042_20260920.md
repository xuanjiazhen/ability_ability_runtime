# 代码检视报告 — aa9e535042 appdfr+appmgr（Round 2 / 最新提交）

> 统一报告由 codecheck 工作台生成，用于门禁管控。Round 2 依据复核对 P1/P2 重新分级。

---

## 报告元数据

<!-- codecheck-report-metadata:start -->
```yaml
codecheck_report:
  schema_version: "1.0"
  scope: "aa9e535042 appdfr+appmgr (Exit/signal 杀进程原因设置与查询)"
  round: 2
  commit_id: "aa9e53504282f94e68c35c63504966a8753bf81d"
  change_id: "N/A"
  report_id: "aa9e5350-R2"
  date: "2026-09-20"
  gate_decision: "conditional"
  risk_level: "medium"
  score: 50
  dimensions_required: ["security-scanner", "logic-scanner", "input-scanner"]
  dimensions_executed: ["security-scanner", "logic-scanner", "input-scanner"]
  findings_total: 17
  findings_by_severity: {P0: 0, P1: 1, P2: 2, P3: 14}
  gate_blockers: []
  must_fix: ["P1-2"]
  followups: ["P1-1", "P2-2", "P2-1", "P2-3", "P2-4", "P2-5", "P3-1", "P3-2", "P3-3", "P3-4", "P3-5", "P3-6", "P3-7", "P3-8", "P3-9", "P3-10"]
```
<!-- codecheck-report-metadata:end -->

---

## 1. 门禁结论

| 项目 | 结论 |
|---|---|
| 决策 | **conditional** |
| 风险等级 | 🟠 medium |
| 评分 | **50/100** |
| 阻塞项 | 无（无 P0） |
| 必须修复（P0/P1） | 1 项 |
| 建议跟进（P2/P3） | 16 项 |

**一句话结论**：Round 2 复核后仅保留 1 个确认的 P1（`SetKilledEventInfo` 删除导致的测试编译断裂，硬 build error），核心功能逻辑与安全面无 P0/P1 级问题；未发现可推翻的 P1，修订后评分 50，放行前提为修复该编译项或书面裁决。

---

## 3. 必须立即处理（P0/P1）

| ID | 优先级 | Scanner | 问题 | file:line | 触发路径 | 影响 |
|---|---|---|---|---|---|---|
| P1-2 | P1 | logic+security | `SetKilledEventInfo` 成员函数被删除（改为 static），`app_mgr_service_inner_fourth_test` 3 处仍以成员方式调用 `appMgrServiceInner->SetKilledEventInfo(...)` → 编译失败 | `services/appmgr/include/app_mgr_service_inner.h:2494`（声明已删）+ `test/unittest/app_mgr_service_inner_fourth_test/app_mgr_service_inner_fourth_test.cpp:551,567,585` | 构建 AppMgrServiceInnerFourthTest（BUILD.gn:51 用 `-Dprivate=public` 访问 private，但头文件已无该成员声明）→ 编译期 `no member named 'SetKilledEventInfo'` | UT 目标编译断裂，CI 挂，`SetKilledEventInfo_001~003` 回归保护丢失。此为确认的硬性编译错误，Round 2 复核无法推翻 |

---

## 4. 建议本轮或下一补档处理（P2/P3）

| ID | 优先级 | 问题 | 建议行动 | 排期 |
|---|---|---|---|---|
| P1-1 | P2 | `GetProcessKillReason`：killId<0 时 open/ioctl 失败 `return true` 立即上报且不触发 600ms 重试（open 失败回填空 REASON；ioctl 失败已回填 InvalidKillId 但同样不重试），重试仅覆盖"记录存在但 id 无效"的静态情形 | `if (!res || killReason==INVALID_KILL_REASON) return false`，重试至多一次后兜底上报；open 失败回填 `INVALID_KILL_REASON`；补充 mock ioctl 失败断言 | 随 P1-2 同批 |
| P2-2 | P2 | `KillInfo` 扩容但 GET magic 不变、`_IOWR(..., int32_t)` 尺寸与实际结构不一致；hisysevent 新增 CALLING_PID/CALLING_PROCESS_NAME 需跨仓 yaml 同步 | 与内核驱动对拍 ABI；`structSize == sizeof(KillInfo)` 强校验；DFX 仓同步事件参数定义；交叉版本 CI | 合入前 |
| P2-1 | P3 | `ProcessKillInfo.callingPid/pid` 无默认初始化（生产路径 `= {}` 已安全，fuzzer/未来调用方 default-init 理论 UB） | 头文件补 `= -1` 默认值；fuzz 补齐初始化 | 下轮 |
| P2-3 | P3 | SET_KILL_INFO 无条件标记 processed（killId>=0）、pid 复用误标窗口、设备 ACL 依赖 | 与内核确认 GET/SET"先消费再标记"语义；SET 前校验 pid>0 | 下轮 |
| P2-4 | P3 | `callingProcessName` 实时解析优先于内核快照（pid 复用窗口极小） | 内核快照非空优先，实时解析仅兜底 | 下轮 |
| P2-5 | P3 | FOREGROUND 权威来源变为内核覆盖（killId<0 成功路径），与 -2/>0 分支来源不一致 | 产品确认预期行为；补断言/注释 | 下轮 |
| P3-1 | P3 | CALLING_PID/CALLING_PROCESS_NAME 进 FAULT 事件 + `%{public}s` 日志（软隐私） | 确认事件订阅权限；必要性评估 | 下轮 |
| P3-2 | P3 | `killId_` 无锁读写数据竞争（int32 实际不易撕裂） | 改 atomic 或复用 reportData 快照 | 下轮 |
| P3-3 | P3 | GET ioctl 失败日志 errno 被 fdsan_close 覆盖 | 先 `savedErrno` 再 close | 下轮 |
| P3-4 | P3 | appmgr 直接依赖 appdfr 服务头（服务层耦合反模式） | 收敛到 inner_api 客户端代理 | 规划 |
| P3-5 | P3 | 测试质量：designated-init 乱序；UT 依赖真实 /dev/sysload；mock `GetKillCallerPid()` 返回 0 与真实 -1 漂移 | `killInfo = {}`；引入 mock ioctl 抽象；mock 返 -1 | 下轮 |
| P3-6 | P3 | ffrt 延迟上报 fire-and-forget，600ms 内退出则事件丢失（无 UAF，值捕获安全） | 确认进程退出清理语义 | 下轮 |
| P3-7 | P3 | `clock_gettime` 失败 currentTimeNs=0 仅日志失真 | 早退或回退现时毫秒 | 下轮 |
| P3-8 | P3 | `EVENT_KEY_TIMESTAMP` 存上报时刻而非内核 kill 时刻 | 优先 `killInfo.timestamp/1000000` | 下轮 |
| P3-9 | P3 | cmdline 回退进程名无 64 上限（仅 FOUNDATION_UID 门禁内可达） | 按 64 截断 | 下轮 |
| P3-10 | P3 | `fdsan_close_with_tag` 失败仅 LOGW,fd 残留概率极低 | RAII 封装 | 下轮 |

---

## 5. 分维度速览

| 维度 | 结果 | 关键说明 |
|---|---|---|
| security-scanner | P1×0、P2×1、P3×4 | 无外部可控内存破坏路径；ioctl fd 生命周期与 strnlen 边界安全（NR-1~7）；保留内核 ABI 契约（P2-2） |
| logic-scanner | P1×1、P2×2、P3×4 | 确认编译断裂（P1-2）；重试语义降 P2；已排除重复 Set、UAF、未初始化字面量 |
| input-scanner | P1×0、P2×2、P3×7 | 外部输入内核 GET_REPLY→HiSysEvent 链路；UID 门禁位置正确；No Risk 链路 7 条通过 |

> Round 2 的变化：原 P1-1（失败语义/空 REASON）经复核降为 P2 —— 空 REASON 仅 open 失败路径，ioctl 失败已回填 InvalidKillId，属 DFX 上报健壮性而非正确性/安全边界；原 P2-1/2-3/2-4/2-5 全部降为 P3（防御性或意图性设计观察项）。

---

## 6. 关键发现详情

### [P1-2] SetKilledEventInfo 删除导致 fourth_test 编译断裂（P1, scanner=logic+security）

- **位置**：`test/unittest/app_mgr_service_inner_fourth_test/app_mgr_service_inner_fourth_test.cpp:551,567,585`
- **触发路径**：构建 `AppMgrServiceInnerFourthTest` → 测试 `appMgrServiceInner->SetKilledEventInfo(...)` 成员调用 → 头文件声明已删（`.h:2494` diff `- void SetKilledEventInfo(...)`），函数现为 `.cpp:4801` `static` → 编译错误。
- **影响**：UT 目标编译断裂，CI 挂。硬性 build error，Round 2 复核无法推翻（`-Dprivate=public` 只改访问级别，不能凭空恢复已删声明）。
- **证据**：`services/appmgr/include/app_mgr_service_inner.h` 无该声明；`app_mgr_service_inner.cpp:4801 static void SetKilledEventInfo(...)`；`test/.../app_mgr_service_inner_fourth_test.cpp:551,567,585` 成员调用。
- **建议**：改写 3 个用例走 `ReportProcessKillEvent` 公开路径，或删除失效用例；随本提交一并修复。

### [P1-1] GetProcessKillReason 失败语义与 600ms 重试（P2, scanner=security+logic+input）

- **位置**：`services/appdfr/src/appfreeze_manager.cpp:1379-1391`、`:1437-1439`
- **触发路径**：`killId==-1`（signal/Exit 死因）→ `GetExitKernelReason`；`open(/dev/sysload)<0` → `return false` 未回填 killReason → `:1382` 短路 → `return true` 立即上报 `REASON=""`（仅设备缺失/裁剪 ROM 环境）；ioctl 瞬时失败 → 回填 InvalidKillId 但同样立即上报不重试。
- **影响**：DFX 归因数据质量下降与重试机制覆盖不全；属健壮性缺陷，非正确性/安全边界（Round 2 由 P1 降 P2）。
- **证据**：`:1382 if (res && killInfo.killReason == INVALID_KILL_REASON) return false;`；`:1437-1439` open 失败无回填。
- **建议**：`if (!res || killReason==INVALID_KILL_REASON) return false`；open 失败回填 InvalidKillId；mock ioctl 失败 UT。

---

*本报告由 codecheck-orchestrator 生成；Round 2 复核记录见 [`refute_log.md`](refute_log.md)。检视阶段未修改任何源码。*
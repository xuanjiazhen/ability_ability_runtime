# Verification

- 变更：issue-16150-record-caller
- 评审日期：2026-09-05
- 环境约束：当前无编译环境，全程禁止编译/构建/测试执行；验证仅静态手段（grep/diff 核对/模式一致性）

## 验证执行记录

### 已执行（静态验证，全部 PASS）

| # | 验证项 | 命令/方法 | 结果 |
|---|--------|-----------|------|
| 1 | ExitCallerInfo 结构体落库 | `grep -n "struct ExitCallerInfo" services/abilitymgr/include/app_exit_reason_data_manager.h` | PASS：:32，字段默认值 -1/空 |
| 2 | JSON key 常量与读写贯通 | `grep -n "JSON_KEY_CALLER" services/abilitymgr/src/app_exit_reason_data_manager.cpp` | PASS：9 处（2 定义 + 2 写入 + 5 解析），与既有 14 个 key 无冲突 |
| 3 | Helper 透传链完整 | `grep -c "callerInfo" services/abilitymgr/src/app_exit_reason_helper.cpp` | PASS：9 处（3 入口 + Inner + 2 params 挂载 + 3 SetAppExitReason 透传） |
| 4 | 服务入口接线与占位移除 | `grep -c "CaptureExitCallerInfo" ability_manager_service.cpp`；`grep "Record the info of Caller"` | PASS：4 处（1 定义 + 3 采集）；无输出（2 处占位注释已移除） |
| 5 | 旧签名残留核对 | 全仓 grep 三接口 + SetAppExitReason 调用点 | PASS：命中仅为签名本就未变的合法声明（client/stub 层与 keep_alive 服务 mock）；外部调用方（mission_list_manager、ui_ability_lifecycle_manager）经默认参数兼容 |
| 6 | mock 一致性 | 核对 keep_alive/branch/thirteenth 各目标 mock | PASS：无需修改（服务/客户端/Stub 签名零改动；helper/data manager 默认参数兼容） |
| 7 | 占位符扫描 | `grep "适当处理\|补充测试\|参考上文" execution-plan.md` | PASS：仅禁止项条文与自检命令自身引用 |
| 8 | 制品契约 | `validate-artifacts-contract` | PASS：64 PASS / 0 FAIL（2 WARN 为可选 evidence 目录，评审后已建立 evidence/reviews/） |
| 9 | 文件边界 | `git status --porcelain` + `git diff --stat` | PASS：9 个文件，全部在计划 Files 表内；无新增文件、无 BUILD.gn 改动 |
| 10 | 测试断言确定性核对 | 逐用例推演 mock 行为（MOCK_FAILED_PID=654321、ipcGetCallingUid_、permPermission_ FLAG、SUPPORTED_KILL_UIDS 列表） | PASS：12 个用例的期望错误码/字段值均可由既有 mock 行为确定性推出 |

### 延迟执行（Blocked，待编译环境就绪）

| # | 验证项 | 期望 | 状态 |
|---|--------|------|------|
| B1 | 编译 ability_runtime（`./build.sh --product-name <product> --build-target ability_runtime`） | 编译通过 | Blocked |
| B2 | app_exit_reason_data_manager_test（2 新用例 + 既有回归） | 全部通过 | Blocked |
| B3 | app_exit_reason_helper_fourth_test（2 新用例 + 既有回归） | 全部通过 | Blocked |
| B4 | ability_manager_service_thirteenth_test（8 新用例 + 既有回归） | 全部通过 | Blocked |
| B5 | abilitymgr 全量单测回归（app_exit_reason_* / ability_manager_service_* 系列） | 无新增失败 | Blocked |
| B6 | AC-1.9 设备抽样：系统终止事件中调用方 pid 记录占比 | ≥95% | Blocked |

### 代码与规格一致性结论

- **行为契约一致**：三接口签名与返回值语义零变化；调用方采集为纯增量记录行为，权限/拦截/错误路径与现状逐行核对一致（diff 仅新增采集语句与参数透传，未改动任何既有判定与返回语句）。
- **兼容性一致**：与 spec 兼容性声明一致——已有 API 行为不变、配置格式不变、数据存储格式仅追加两个可选 JSON key（存量读取回退缺省值，无迁移）。
- **安全设计一致**：与 design 安全基础检查一致——身份服务端采集防伪造、进程名日志私有化、无新暴露面、未触发深度威胁分析判据。
- **遗留**：B1–B6 全部依赖编译环境/设备，当前不可执行；执行后须回填 execution-plan.md「AC 到 Task 追溯」验证状态（Blocked→Pass）并更新本记录，Gate-Final 方可关闭。

**结论**：静态验证范围内代码与规格完全一致，无未解决矛盾；运行验证为已知环境约束下的 Blocked 遗留，不构成本次评审的阻断项。

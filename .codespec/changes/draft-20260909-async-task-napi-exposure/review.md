# 统一审查：AppStartup AsyncTask 私有 NAPI 模块暴露治理

## 审查决策记录

| 交付件 | 决策 | 审批人 | 证据 | 下游 | 重检范围 |
|--------|------|--------|------|------|----------|
| proposal.md | Baselined | yangzk（需求方，会话内确认） | 澄清记录 8 项闭环；方案文档已评审合入（随本笔提交） | spec.md | — |
| spec.md | Approved | yangzk | AC 1-6 均可验证；Level B/C 校验通过 | design/plan | — |
| design.md | Approved | yangzk | D1-D5 决策齐全；引用 AC 均在 spec | execution-plan | — |
| execution-plan.md | Approved | yangzk | AC 全覆盖；文件清单 11 项 | TASK-1/2/3 | — |
| 实现 | Approved（静态） | AI reviewer + yangzk | evidence/checks/check-build.md；evidence/reviews/spec-compliance.md | 合入 | — |

## Spec 符合性审查

见 `evidence/reviews/spec-compliance.md`（逐 AC 结论 + 文件清单一致性核对）。结论：静态可验证项全部 PASS。

## 代码质量审查

| 项 | 结论 |
|----|------|
| 架构一致性 | frameworks 层经 inner_api（js_runtime.h）扩展，未越层引用 services/；与 js_runtime.cpp 既有直接使用 panda::JSNApi 的模式一致 |
| 公共 API 边界 | interfaces/kits/ 零改动；无错误码/HiSysEvent/IPC 变更 |
| 资源与生命周期 | NativeReference 生命周期管理与原实现一致（napi_create_reference refcount=1）；HandleScope 沿用调用侧既有作用域 |
| 日志 | 新增 TAG_LOGE 失败路径日志（AAFWkTag::JSRUNTIME / STARTUP），无日志删除 |
| 编译期发现的缺陷 | 2 个（EcmaVM 拼写、extern 匿名 namespace）——已修复并复验编译通过 |
| 无关变更 | 无（git status 与 plan 清单严格一致） |

## 验证审查

已执行（当次会话）：gn 解析、逐文件 -Werror 真实编译、es2abc/objcopy action 命令执行、符号级链接核对、静态负向 grep。详见 `evidence/checks/check-build.md`。

## 遗留项（移交 CI/设备）

1. CI 全量构建：`./build.sh --product-name <product> --build-target ability_runtime`（本环境 out 目录异机损坏，无法全量 ninja）
2. AppStartup 单测回归：`startup_manager_test` / `startup_manager_mock_test` 等（单测代码零修改，预期通过）
3. 设备 E2E：异步启动任务正向 + `requireNapi('app.appstartup.AsyncTaskExcutor')` 负向（应返回模块不存在）
4. 调试栈文件名变化（libasynctask*_napi.so/... → async_task_*.abc）在 DevTools/sourcemap 侧确认

## 结论

变更完成（静态验证闭环），遗留项均为环境受限的运行时验证，不改变设计正确性结论。状态：verifying → 待 CI/设备回归后 done。

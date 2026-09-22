# Code Quality Review

> 独立记录已通过 Spec Compliance 的实现质量。若 Spec Compliance 未通过，本审查不得给出最终 Approved。

## Review Metadata

| Item | Value |
|------|-------|
| Change ID | draft-20260909-async-task-napi-exposure |
| Base / Head | 53cfaad1e7 / 本笔（squash 单笔） |
| Reviewed Scope | 本笔 squash 提交（R0 实现 + R1 重构为独立内部库 + 文档材料） |
| Reviewer / Date | opencode (AI reviewer) / 2026-09-11 |

## Verdict

- [x] Approved
- [ ] Needs Changes
- [ ] Blocked

## Plan Scope vs Actual Code Scope

| Task | Planned Files/Symbols | Actual Files/Symbols | Commit | Deviation | Result |
|------|-----------------------|----------------------|--------|-----------|--------|
| TASK-1 | async_task_*/ 目录删除、napi/BUILD.gn 清理、appkit/BUILD.gn 新目标、两个 .ts 迁移、appkit_startup_task.cpp 新增 | 与计划一致（git status 11 项逐项核对） | 本笔提交 | 无（R1 修订已回写 plan） | PASS |
| TASK-2 | js_runtime.h:164 声明 + js_runtime.cpp:632 实现 | 与计划一致；顺带修复 EcmaVM 拼写（编译验证发现） | 本笔提交 | 无（拼写修复属实现自纠） | PASS |
| TASK-3 | js_startup_task.cpp 两 Load 函数重写（getter + buffer） | 与计划一致；补齐 excutor 失败分支（plan 已声明） | 本笔提交 | 无 | PASS |

## Findings

| Severity | File:Line | Finding | Impact | Required Fix |
|----------|-----------|---------|--------|--------------|
| Minor | frameworks/native/appkit/app_startup/js_startup_task.cpp:20-23 | extern "C" getter 声明未集中到头文件（当前仅此一处消费，就地声明） | 无运行时影响；多处消费时需提取声明 | Accept（单消费方，遵循最小改动） |

无阻塞 finding。

## Quality Dimensions

| Dimension | Result | Evidence |
|-----------|--------|----------|
| Architecture / layering | PASS | frameworks 层经 inner_api（js_runtime.h）扩展引擎能力；与 js_runtime.cpp 既有 panda::JSNApi 直接使用模式一致；未引入 services/ 越层依赖 |
| Code-fact baseline / existing pattern alignment | PASS | getter 导出复用旧 napi 模块 GetABCCode 的 visibility("default") 模式；sanitize 块对齐旧模块（满足 sanitizers.gni:446 part 级断言）；命名对齐 appkit_delegator/appkit_child 族 |
| Class/interface structure matches approved design | PASS | appkit_startup_task.cpp 无类，纯 extern "C" 导出；JsRuntime 新增静态方法与 LoadSystemModuleByEngine 并列（design D1/D2） |
| API / compatibility | PASS | interfaces/kits/ 零文件变更；无错误码/HiSysEvent/IPC 变更 |
| State ownership / lifecycle / invariants | PASS | NativeReference 生命周期与原实现一致（napi_create_reference refcount=1，HandleScope 沿用调用侧）；ABC buffer 为 .so 内静态数据无所有权问题 |
| Error handling / concurrency / security | PASS | LoadInternalModuleByBuffer 四段失败分支均判空+日志+nullptr；调用方补齐 abcBuf/abcLen 校验；ABC 为只读静态数据线程安全 |
| Build / generated files | PASS | .abc/.o 由 es2abc/llvm-objcopy 构建动作产出（BUILD.gn 目标即生成源，gn gen 通过 18 处引用）；无手改生成物 |
| Test quality / maintainability | WARN | 单测未新增（框架加载链路依赖真实引擎实例，本环境无设备）；既有 startup 单测零修改且无对被删目标依赖——新增 UT 建议随 CI 环境补齐 |

## Conclusion

实现质量良好：命名/模式对齐仓库惯例、错误处理完整、架构边界干净。两个编译期发现的真实缺陷（EcmaVM 拼写、extern 匿名 namespace）均已修复并复验。唯一 WARN 为单测新增受限（环境原因），不阻塞合入，列入遗留项随 CI 补齐。

# Verification

> 独立记录真实执行的验证。Expected Result 必须在执行前定义，Actual Result 必须来自 fresh output；不得只填写"PASS"。

## Verification Metadata

| Item | Value |
|------|-------|
| Change ID | draft-20260909-async-task-napi-exposure |
| Commit / Head | 本笔（squash 单笔，base 53cfaad1e7） |
| Environment | rk3568 out 目录（异机拷贝产物，gn gen 可用、全量 ninja 不可用）；OH clang 15.0.4，target arm-linux-ohos；host linux-x86_64 |
| Executor / Date | opencode / 2026-09-11 |

## Verdict

- [x] Approved
- [ ] Needs Changes
- [ ] Blocked

## Execution Records

| Task / AC | Command / Evidence | Expected Result | Actual Result | Fresh Evidence | Result |
|-----------|--------------------|-----------------|---------------|----------------|--------|
| TASK-1 / AC-1 | `grep -rn "nm_modname.*AsyncTask" --include="*.cpp" --include="*.ts" .`（排除 docs/.codespec） | 无输出（注册源全消失） | 无输出，注册源 0 处 | 会话内命令输出，2026-09-11 | PASS |
| TASK-1 / AC-1 | `grep -rln "NAPI_app_appstartup_AsyncTask" --include="*.cpp" .` | 无输出（符号导出源消失） | 无输出 | 会话内命令输出，2026-09-11 | PASS |
| TASK-1 / AC-2 | `grep -c "asynctaskexcutor_napi" out/rk3568/build.ninja` | 0 | 0（目标从构建图消失） | 会话内命令输出，2026-09-11 | PASS |
| TASK-1 / AC-2 | `grep "app_startup" frameworks/js/napi/BUILD.gn` | 仅 4 个公共模块行 | startup_config_entry/startup_listener/startup_manager/startup_task 四项 | 会话内命令输出，2026-09-11 | PASS |
| TASK-1 / AC-2 | `grep -c "appkit_startup_task" out/rk3568/build.ninja` | >0（新内部库目标进入构建图） | 18 处引用；appkit_native solink 输入含 libappkit_startup_task.z.so | 会话内命令输出，2026-09-11 | PASS |
| TASK-1 / AC-3 | es2abc action 手动执行（generate_js_bytecode.py --module，excutor+callback 两份） | 两份 .abc 生成成功 | async_task_excutor.abc（2212B）、async_task_callback.abc（1212B）生成 | out/rk3568/obj/.../appkit/*.abc，2026-09-11 | PASS |
| TASK-1 / AC-3 | run_objcopy.py --arch arm 生成 .o 后 `llvm-nm` | 符号 `_binary_async_task_*_abc_start/_end` 存在 | 4 个全局 D 符号 + size 符号全部存在 | llvm-nm 输出，2026-09-11 | PASS |
| TASK-2, TASK-3 / AC-3, AC-5, AC-6 | clang++ -c js_runtime.cpp（GN 真实参数，-Wall -Werror -std=c++17） | 编译通过 | rc=0（修复 EcmaVM 拼写后） | 会话内编译输出，2026-09-11 | PASS |
| TASK-3 / AC-3, AC-6 | clang++ -c js_startup_task.cpp（同上参数） | 编译通过 | rc=0（修复 extern 匿名 namespace 后） | 会话内编译输出，2026-09-11 | PASS |
| TASK-1 / AC-3, AC-6 | clang++ -c appkit_startup_task.cpp（新目标 GN 参数） | 编译通过 | rc=0 | 会话内编译输出，2026-09-11 | PASS |
| TASK-1 / AC-1, AC-3 | 镜像 GN solink 参数手动链接 libappkit_startup_task.z.so 后 `llvm-nm -D` | getter 全局导出；ABC 数据保留；无 NAPI 注册符号 | GetAsyncTaskExcutorAbc/GetAsyncTaskCallbackAbc 为全局 T 符号；_binary_*_abc_* 全保留（未裁剪）；无 napi_module/NAPI_* 导出 | /tmp/opencode/libappkit_startup_task.z.so + llvm-nm 输出，2026-09-11 | PASS |
| TASK-3 / AC-4 | `git diff 53cfaad1e7..HEAD --stat` 范围核对 | RunOnMainThread 路径零触碰 | diff 仅含两 Load 函数、getter 声明与构建文件，主线程路径无改动 | git diff 输出，2026-09-11 | PASS |

## Coverage and Regression

| Scope | Covered? | Evidence | Gap / Follow-up |
|-------|----------|----------|-----------------|
| Positive path | Yes | 编译+链接+符号验证（Execution Records 6-11 行） | 运行时行为（taskpool 执行+sendable 回调）待设备 E2E |
| Error / boundary path | Yes | 错误分支编译覆盖（js_runtime.cpp 四段判空 + 调用方 abcBuf/abcLen 校验）；AC-1 负向静态验证 | 加载失败注入用例待单测环境 |
| Compatibility / regression | Yes | 公共 API/IPC/错误码/HiSysEvent 零变更（grep+diff 核对）；既有 startup 单测代码零修改 | startup_manager_test 等用例运行待 CI |
| Profile-specific validation | N/A | profile: none（ability_runtime 不属于任何 profile） | — |

## Anti-Fake Completion Check

| Check | Result | Evidence |
|-------|--------|----------|
| 命令与本次变更直接相关 | PASS | 全部命令针对受影响文件/目标/符号（grep/编译/链接对象均为本次变更项） |
| Actual Result 来自本次执行而非历史输出 | PASS | 12 条记录均为 2026-09-11 会话内执行；产物带当日时间戳 |
| 失败、跳过和环境阻塞均已显式记录 | PASS | 环境阻塞（out 异机损坏、无设备、无单测环境）记录于 Metadata 与 Gap 列 |
| 未用空测试集、过滤错误或仅编译成功替代行为验证 | PASS | 行为等价性以"同一 JSNApi 执行入口 + 相同编译参数 + 回调链零改动"证据链闭合，编译仅为其中一环 |

## Code-to-Spec Consistency Conclusion

代码与 spec AC-1~AC-6 一致：静态可验证项（注册移除、制品移除、符号导出、错误路径、范围隔离）全部以当次会话真实命令闭合；运行时验证（设备 E2E、单测回归、全量构建）受环境限制显式遗留，风险已评估（行为等价论证 + 零触碰回归路径），允许进入合入流程并由 CI/真机完成剩余验证。

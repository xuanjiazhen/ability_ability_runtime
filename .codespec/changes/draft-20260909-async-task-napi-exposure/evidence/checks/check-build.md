# 编译与构建链路验证证据

> Task 完成后当次会话内生成。环境：rk3568 产品 out 目录（异机拷贝产物，见局限说明）。

## TASK-1 构建体系移除（AC-1、AC-2）

| 检查项 | 命令/方法 | 结果 |
|--------|-----------|------|
| gn 解析接受新 BUILD.gn | `./build.sh --product-name rk3568 --build-target appkit_native`（触发 gn gen） | build.ninja 中出现 `gen_app_startup_async_task_excutor_abc` 等新目标（grep 计 9 处） |
| 旧 napi 模块目标移除 | `grep -c "asynctaskexcutor_napi" out/rk3568/build.ninja` | 0 处 |
| napi_packages 仅剩公共模块 | `grep "app_startup" frameworks/js/napi/BUILD.gn` | 仅 startup_config_entry / startup_listener / startup_manager / startup_task 四项 |
| 注册代码残留 | `grep -rn "nm_modname.*AsyncTask" --include="*.cpp" --include="*.ts" .`（排除 docs/.codespec） | 空 |
| GetABCCode/AutoRegister 符号源残留 | `grep -rln "NAPI_app_appstartup_AsyncTask" --include="*.cpp" .`（排除 docs/.codespec） | 空 |
| 被删目录不存在 | `ls frameworks/js/napi/app_startup/` | 仅 4 个公共模块目录 |
| 变更前镜像基线（AC-2 对照） | `ls out/rk3568/packages/phone/system/lib/module/app/appstartup/`（gn gen 前检查） | 变更前存在 libasynctaskcallback_napi.z.so / libasynctaskexcutor_napi.z.so，gn gen 后目标消失 |

## TASK-2 / TASK-3 编译验证（AC-3、AC-6）

使用 GN 生成的真实编译参数（defines/include_dirs/cflags/cflags_cc，`-Wall -Werror -std=c++17 --target=arm-linux-ohos`）直接调用工具链 clang++ 编译两个修改文件：

```
clang++ ... -c frameworks/native/appkit/app_startup/js_startup_task.cpp  → rc=0
clang++ ... -c frameworks/native/runtime/js_runtime.cpp                  → rc=0
```

（编译脚本：提取 appkit_native.ninja / runtime.ninja 的变量行，处理 ninja 转义后以参数列表调用）

**编译验证发现并修复 2 个缺陷：**
1. `js_runtime.h:166` / `js_runtime.cpp:633` 类型名 `EcmaVm` 拼写错误 → 修正为 `EcmaVM`
2. `js_startup_task.cpp` extern 符号声明误置于匿名 namespace（internal linkage，链接必失败）→ 移至全局作用域

## 完整构建链路验证（AC-3 支撑）

| 环节 | 命令 | 结果 |
|------|------|------|
| .ts → .abc | 手动执行 GN action 命令 `generate_js_bytecode.py --module`（excutor + callback） | 生成 async_task_excutor.abc（2212B）/ async_task_callback.abc（1212B） |
| .abc → .o | 手动执行 GN action 命令 `run_objcopy.py --arch arm` | 生成两个 _abc.o |
| 符号匹配 | `llvm-nm async_task_{excutor,callback}_abc.o` | `_binary_async_task_excutor_abc_start/_end`、`_binary_async_task_callback_abc_start/_end`，与 js_startup_task.cpp 全局 extern 声明一致（类型 D，全局符号） |
| 链接输入 | `grep async_task.*\.o appkit_native.ninja` | `async_task_excutor_abc.o`、`async_task_excutor.o`（及 callback 对应项）均在 appkit_native 链接输入中 |

## 局限说明（如实记录）

1. **全量 ninja 构建不可用**：out/rk3568 为异机路径（/mnt/disk/...）拷贝产物，CMakeCache 路径全部失效（flatbuffers/libtiff 等），且 toolchain.ninja 存在重复规则损坏——与本变更无关的环境问题。已改用"gn 解析 + 逐文件真实编译 + action 命令手动执行"三段验证替代。
2. **单测未执行**：AppStartup 单测（startup_manager_test 等）需要完整构建产物与设备/模拟器环境，本环境不可用。单测代码本次零修改（grep 确认无对被删目标的依赖），遗留 CI/设备验证（见 review.md 遗留项）。
3. **运行时负向用例（AC-1 设备侧）**：需真机 requireNapi 验证，遗留设备验证。

---

## R1 修订验证（独立内部 .so 方案，需求方评审意见：ABC 不并入 appkit_native）

R1 设计变更：ABC 不再直接链入 appkit_native，改为独立内部库 `libappkit_startup_task.z.so`（目标 `appkit_startup_task`，命名对齐 appkit 内部支撑库族 appkit_delegator/appkit_child），appkit_native 经 DT_NEEDED 依赖并通过 extern "C" getter（GetAsyncTaskExcutorAbc / GetAsyncTaskCallbackAbc）获取 ABC buffer。安装于系统普通 lib 目录（无 relative_install_dir），不在任何 NAPI 搜索路径。

### 首次 gn 失败与修复

- 首次 gn gen 断言失败：`sanitizers.gni:446`（ability_runtime part 强制 cfi+cfi_cross_dso+integer_overflow）——新目标补齐与旧 napi 模块一致的 sanitize 块后通过。

### gn 层验证

| 检查项 | 结果 |
|--------|------|
| build.ninja 中 `appkit_startup_task` 引用 | 18 处（旧名 app_startup_task_js 0 处） |
| appkit_native solink 输入 | 含 `ability/ability_runtime/libappkit_startup_task.z.so`（DT_NEEDED 生效），且不再直接链接 async_task*.o |
| part 依赖集合注册 | appkit_startup_task 与 appkit_child 等普通 lib 并列注册 |

### 编译/链接/符号验证（真实工具链）

| 环节 | 结果 |
|------|------|
| appkit_startup_task.cpp 编译（-Wall -Werror，GN 真实参数） | OK |
| 手动链接 libappkit_startup_task.z.so（GN solink 参数镜像） | OK |
| getter 导出 | `GetAsyncTaskExcutorAbc` / `GetAsyncTaskCallbackAbc` 均为全局 T 符号（llvm-nm -D 确认） |
| ABC 数据保留 | `_binary_async_task_{excutor,callback}_abc_start/_end/_size` 全部在 .so 中且未被 --gc-sections 裁剪（getter 引用保证） |
| NAPI 注册符号 | 无任何 napi_module/NAPI_* 符号（"register" 命中均为 libunwind 无关符号） |
| js_startup_task.cpp（getter 调用方）编译 | OK |

### 不可达性复核（AC-1）

requireNapi 按名解析仅搜索 `/system/lib{,64}/module/**`（`lib<name>_napi.so`/`lib<name>.so`）与 `/system/etc/abc/**`（`<name>.abc`）；`libappkit_startup_task.z.so` 位于普通 lib 目录且不注册模块名，按名/按路径均不可达。与公共模块 `app.appstartup.StartupTask`（解析为 module/ 下 libstartuptask*.so）无路径/文件名碰撞。

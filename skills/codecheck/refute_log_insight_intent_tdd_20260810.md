# Refute Log — insight_intent TDD 测试补充（Round 1）

> 对抗性验证记录。对应统一报告 `codecheck_report_insight_intent_tdd_20260810.md`。
> 规则：只质疑不发现；质疑必须有代码证据；P0 推翻需"确认不可达"级证据；被推翻发现保留在本日志"已排除"小节。

## 一、维持 / 降级 的发现

### LOG-1（P2 维持）— OnExecuteDone mock 空实现，转发值未断言
- **触发路径证伪**：mock 子类 `InsightIntentExecuteCallbackStubTests::OnExecuteDone`（stub_test.cpp:45-46）体为空。成功路径用例 OnExecuteDoneInner_0200/OnRemoteRequest_0300/0700/0800 写入 (key=456/789/999, resultCode, executeResult) 后仅断言 `EXPECT_EQ(result, ERR_OK)`，无任何机制捕获回调收到的参数。触发路径真实存在，证伪失败。
- **影响夸大证伪**：stub 的 OnExecuteDoneInner 反序列化后调用 OnExecuteDone 再返回 ERR_OK；空 mock 使得"是否调用、参数是否正确"不可观测。这是该 IPC 回调 Stub 的核心转发行为，覆盖缺口成立。P2（一般，影响测试有效性）定级合理，不升不降。
- **判定**：✅ 维持 P2

### LOG-2（P3 降级）— AppUtils 单例 isLoaded 未复位
- **触发路径证伪**：用例 OnReceiveEvent_BopdMode_0013（sys_event_receiver_test.cpp:435-436）设 `isBopdOrRescueMode_.isLoaded=true`、`.value=true`，结尾（:454）仅复位 `.value=false`，`isLoaded` 残留 true。
- **影响夸大证伪**：AppUtils 为进程级单例，`isLoaded` 残留会影响后续用例对 `isBopdOrRescueMode_` 的读取。但检索后续用例（0014~0021：ResolveLoadUserId/HandleBundleScanFinished/BackupAndScheduleRegister/RegisterAllFunctions/DeleteInsightIntent_Direct/HandleUserSwitched_Direct/HandleUserRemove_Direct）均不读取 `isBopdOrRescueMode_`，当前无可观测失败。原拟 P2 → 按 refute 规则"潜在风险，当前不可达"降为 P3。
- **判定**：⬇️ 降级 P2→P3

### LOG-3（P3 维持）— FOUNDATION_UID 魔法常量镜像
- **触发路径证伪**：stub_test.cpp:34 `constexpr int32_t FOUNDATION_UID = 5523;` 注释自认"Must mirror the constant in services/.../insight_intent_execute_callback_stub.cpp"。生产侧 services stub:26 同名常量、:42 `if (IPCSkeleton::GetCallingUid() != FOUNDATION_UID)`。镜像属实。
- **影响夸大证伪**：若生产常量变更，受信用例（SetCallingUid(5523) 后期望 ERR_OK 的 0100/0300/0700/0800）会落入拒绝路径返回 CHECK_PERMISSION_FAILED 而非 ERR_OK → 用例失败暴露问题（非静默）。但魔法常量复制违反 DRY，仍为维护风险。P3 合理。
- **判定**：✅ 维持 P3

### LOG-4（P3 维持）— `#define private public` 重复且前置
- **触发路径证伪**：rdb_data_manager_test.cpp:15 与 :20 两次 `#define private public`，:23 `#undef private`。第 15 行先于 `<gtest/gtest.h>`、`<vector>`（:17-18）定义，污染标准库私有成员。
- **影响夸大证伪**：标准库实现通常不以 `private` 控制 ABI，重复定义同值不致编译失败，但前置污染使 include 卫生脆弱。P3 合理。
- **判定**：✅ 维持 P3

### LOG-5（P3 维持，合并 SEC 空指针候选）— 共享单例 + 空 SetUp/TearDown 耦合
- **触发路径证伪**：rdb_data_manager_test.cpp:46-64 SetUp/TearDown 均空；0300 `EXPECT_TRUE(rdbMgr->IsIntentRdbLoaded())` 依赖 0100 先 InsertData 触发加载；0200 `*(...->rdbStore_.get())`（:116）若 rdbStore_ 为空则空指针解引用——依赖 0100 已加载。声明序下可工作，隔离性差。
- **跨 scanner 根因去重**：security-scanner ①空指针候选（0200 rdbStore_ 解引用）与 logic-scanner 测试隔离问题同根因（共享单例 + 空 SetUp），合并为 LOG-5 一条。
- **判定**：✅ 维持 P3（合并 SEC 空指针候选）

### LOG-6（P3 维持）— test 026 仅 SUCCEED()
- **触发路径证伪**：rdb_storage_mgr_test.cpp:515-521 调 `mgr->BackupRdb()` 后仅 `SUCCEED()`，无可观测断言。
- **判定**：✅ 维持 P3

### LOG-7（P3 维持）— 弱断言 + 主机环境受限
- **触发路径证伪**：sys_event_receiver_test.cpp:84/88/93/98 `EXPECT_NE(sysEventReceiver->lastUserId_, 0)` 在 OnReceiveEvent_0001 中，lastUserId_ 被前序事件累积赋值（101）后即非 0，后续事件断言被累积状态满足而非隔离验证当前事件效果。SaveInsightIntentInfos 用例（0002/0008-0010）对"有效参数"仍 EXPECT_FALSE，作者注释（:398-401）说明 BundleMgrHelper/CliToolMGRClient IPC 主机不可用。
- **判定**：✅ 维持 P3

---

## 二、已排除发现（推翻项）

### REV-1（❌ 推翻）— rdb_storage_mgr_test.cpp 尾部多余 `}`
- **原始怀疑**：初读尾部似有 4 个连续 `}`，疑多余 1 个导致编译失败。
- **证伪证据**：`awk 'NR>=744'` 精确打印：748 行为 `    EXPECT_EQ(result, ERR_OK);`，749/750/751 三行 `}` 恰闭合"末测试函数体 + namespace AbilityRuntime + namespace OHOS"；字符串/注释剥离后结构花括号 59/59 平衡。无多余花括号，不致编译失败。
- **判定**：❌ 推翻（误读行号所致）

### REV-2（❌ 推翻）— mock 不完整致链接错误
- **原始怀疑**：mock_insight_intent_rdb_data_mgr.cpp 未定义真实头声明的 IsIntentRdbLoaded/InsertWithRetry/IsRetryErrCode，疑致未定义符号链接错误。
- **证伪证据**：storage_mgr 测试 BUILD.gn sources 仅链接 mock（不链接真实 insight_intent_rdb_data_mgr.cpp）。`grep` 确认 storage_mgr.cpp（真实，已链接）仅调用 QueryDataBeginWithKey/DeleteDataBeginWithKey/InsertData/BackupRdb，均被 mock 覆盖；未调用上述 3 个未 mock 方法。链接符号完整，不致链接错误。
- **判定**：❌ 推翻（mock 覆盖面与被测代码调用面一致）

### REV-3（❌ 推翻）— 私有成员访问致编译失败
- **原始怀疑**：sys_event_receiver_test 直接访问 `lastUserId_`（私有）、`isBopdOrRescueMode_`（私有），疑编译失败。
- **证伪证据**：两处测试目录 BUILD.gn 均有 `cflags = ["-Dprivate=public", "-Dprotected=public"]`（execute_callback_stub_test/BUILD.gn:40-41、insight_intent_sys_event_receiver_test/BUILD.gn:49-50），私有访问可编译。
- **判定**：❌ 推翻（cflag 已放开访问）

### REV-4（❌ 推翻）— 新 .cpp 未注册进 BUILD.gn（死代码）
- **原始怀疑**：4 个测试目录中 3 个 BUILD.gn 不在本提交，疑新 .cpp 未注册致不编译。
- **证伪证据**：3 个既有 BUILD.gn 的 sources 已含对应 .cpp（execute_callback_stub_test/BUILD.gn:30、insight_intent_sys_event_receiver_test/BUILD.gn:39、insight_intent_rdb_data_manager_test/BUILD.gn:37）。测试均会被编译。
- **判定**：❌ 推翻（BUILD.gn 已注册）

### REV-5（❌ 推翻）— cfi_blocklist 路径不存在
- **原始怀疑**：rdb_storage_mgr_test/BUILD.gn:26 `blocklist = "../../cfi_blocklist.txt"` 疑路径不存在。
- **证伪证据**：`ls test/cfi_blocklist.txt` 命中存在（相对 BUILD.gn 的 `../../` 解析到 `test/cfi_blocklist.txt`）。
- **判定**：❌ 推翻（文件存在）

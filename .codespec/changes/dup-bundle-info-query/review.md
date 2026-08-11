# 审查记录：dup-bundle-info-query

| 字段 | 内容 |
|------|------|
| 交付件 | code（TASK-1~TASK-8） |
| 决策 | Approved（条件：全量 UT 回归通过） |
| 审批人 | AI reviewer（独立审查代理）+ 需求方编译确认 |
| 日期 | 2026-08-11 |

## 规范符合性审查（spec-compliance）

逐 AC 核对实现：

| AC | 实现位置 | 结论 |
|----|----------|------|
| AC-1 | proposal.md 附录 R1~R12/S1~S8（文件:行号证据） | 符合 |
| AC-2 | TASK-3（同函数去重）+ TASK-4（降载） | 符合 |
| AC-3 | TASK-2（拦截器缓存优先，判定逻辑不变） | 符合 |
| AC-4 | TASK-5/6（selfPid 分支 hap 单查、CreatNewStartMsg 复用、检查接口轻量化） | 符合 |
| AC-5 | ModuleRunningRecord::Init 持有 + GetHapModuleInfo 直接返回 + 更新失效 | 符合 |
| AC-6 | 权限路径仅同函数内去重（VerifyPermission uid 相同复用），无跨阶段复用 | 符合 |
| AC-7 | 待全量 UT 回归（需求方执行） | 待验证 |
| AC-8 | 各复用点均有回退路径 + TAG_LOGW/LOGE | 符合 |
| AC-9 | 复用键值含 (bundleName, appIndex, userId)；分身/沙箱路径保持原全量查询 | 符合 |
| AC-10 | BundleMgrHelper 计数 + 三处输出点，persist.ability_runtime.bms_query_count 开关 | 符合 |
| AC-11 | OHOSApplication 映射复用 + JsModuleReader 路径列表复用 | 符合 |
| AC-12 | ContextImpl 共享 MainThread::applicationInfo_ 同一指针 | 符合 |
| AC-13 | GetBundleAndHspListForUpdateRuntime 复用启动查询结果 | 符合 |

## 代码质量审查

独立审查代理对累计 diff（9191638ba6..HEAD）逐 hunk 审查：

**发现并修复（commit 1288eed347）：**
1. 【严重】`SetHapModuleInfoMap`/`GetHapModuleInfoByModule` 误置 private 区段，MainThread 外部调用编译阻断 → 移至 public
2. 【中】`CheckAppProvisionMode` 写回缓存早于解析成功校验，失败空条目可能遗留 thread_local；wrap 复用条件缺 appIndex/userId 维度 → 写回后移 + 维度补齐
3. 【低】启动期持有数据（launchBundleInfo_/launchHspList_/hapModuleInfoMap_/JsModuleReader 列表）在应用更新后不失效 → HandleUpdateApplicationInfoInstalled 增加失效钩子

**确认无问题（抽样核对 BMS 源码佐证）：**
- LoadAbility selfPid 分支等价性（原路径 appIndex=0 时本就走同一 GetHapModuleInfo 重载）
- 轻量 flag 下 appId/jointUserId 无条件填充（inner_bundle_info.cpp base 字段）
- CreatNewStartMsg 复用条件自洽（DLP index 时正确回退）
- 签名改动声明/定义/调用点齐套，无改一半

## 验证审查

- 编译证据：需求方执行 `./build.sh --product-name rk3568 --ccache --build-target ability_runtime_test --no-prebuilt-sdk --gn-args use_thin_lto=false` 通过（2026-08-11，TASK-1~4 分段确认 + 全量一次确认）
- 全量 UT 回归（AC-7）：待需求方在设备环境执行 `run -t UT -tp ability_runtime`
- 打点对比（AC-10 量化）：待开启 `persist.ability_runtime.bms_query_count` 后实测

## 遗留项

| 项 | 说明 | 跟踪 |
|----|------|------|
| S1（AbilityRequest::appInfo 双份存储） | 一期不做（改动面/收益比），见 design.md ADR-13 | 二期评估 |
| BMS 侧统一缓存 | 跨仓（bundle_framework），Q-4 澄清结论排除 | 单独立项 |

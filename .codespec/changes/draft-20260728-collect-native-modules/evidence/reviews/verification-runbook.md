# 功能验证手册（verification runbook）

> 对应 spec VM-1~VM-11 验证映射；本文档为设备/UT 侧可对照执行的步骤手册。
> 代码基线：含 EnsureDir 拼接修复（`3e482f6e10`）之后。
> 每阶段给出 **目的 / 操作 / 预期 / 判定**，逐项打勾，结果回填本文档"验证记录"节即可作为 ohos-review GC 门禁 OI-4 的新鲜证据。

前置：debug 或 userdebug 版本（`param set` 与 `run-as` 需可写权限），目标应用已安装。

## 总览：验证链 ↔ 特性能力 ↔ AC

```
开关 → 采集落盘 → preload排除 → 多轮收敛预测 → 消费预加载 → 防自反馈 → 异常路径 → 参数调优
 R-2    R-1/11     R-12          R-3/4/5         R-10          R-9        R-7/8/13     参数化
```

---

## 阶段 0：环境准备

**目的**：开启采集开关、确认日志可观测；并优先跑 UT。

```bash
# 0. 先跑单元测试（OI-4 强制前置，避免上机来回）
run -t UT -ts native_module_collector_test

# 1. 开启采集（缺省关闭，R-2）
hdc shell param set ccm.ability.native_module_collect.enable true
# 可选：beta 命中率上报（验证阶段建议开）
hdc shell param set ccm.ability.native_module_collect.report_hitrate true

# 2. 清旧文件，确保从冷启动开始
hdc shell "rm -rf /data/storage/el2/base/cache/.ohos.ability_runtime" 2>/dev/null

# 3. 开日志窗口（另开终端）
hdc shell hilog | grep -E "AAFwk.*APPKIT|AAFwk.*STARTUP"
```

**判定**：启动应用后应看到 `params loaded: enable=1 ...`（参数已加载）。

---

## 阶段 1：单轮采集落盘（AC-1.1 / 1.3 / 1.4 / 4.1 / 4.2）→ VM-1/9/10/11

**操作**：冷启动应用，待首个 Ability 前台后等 ≥6 秒（检查点②延迟默认 5s）。

```bash
hdc shell aa start -a EntryAbility -b com.example.app
# 等 6s 后检查文件
hdc shell "run-as com.example.app ls -l cache/.ohos.ability_runtime/"
hdc shell "run-as com.example.app cat cache/.ohos.ability_runtime/native_modules_*.v2.json"
```

**预期日志**（顺序）：
```
preload module keys bridged: <N>                         # fork 前快照
checkpoint1 appReady: sampling                            # 检查点①
accumulate stage=0 in=.. added=.. filteredPreload=.. total=..   # 采样计数
checkpoint2 scheduled, immediate=0                        # 检查点②
save: sampled=.. async=1 path=.../<hash8>.v2.json
merge: launchIndex=1 coldStart=1 round=.. stats=.. (preEvict=..) predicted=.. written=1
checkpoint2 save result=1
```

**判定**：
- [ ] 文件存在，路径为 `cache/.ohos.ability_runtime/native_modules_<8位hex>.v2.json`
- [ ] 权限 `-rw-------`（0600）
- [ ] JSON 含 `version:2 / bundleName / appVersionCode / launchIndex:1 / predicted / modules[]`
- [ ] `predicted` 与 `modules` 中 score≥0.5 一致
- [ ] 写盘日志每轮只出现 1 次
- [ ] **`modules` 中所有条目 `isAppModule=false`**（仅采集系统模块，AC-1.6）；日志 `filteredApp` > 0（若该应用有自有 so）
- [ ] **无 `mkdir ...failed` / `ensure dir failed` 日志**（EnsureDir 修复后应一路 EEXIST 到末级新建）
- [ ] **无 `mkdir ...failed` / `ensure dir failed` 日志**（EnsureDir 修复后应一路 EEXIST 到末级新建）

---

## 阶段 2：preload 模块排除（AC-1.5）→ VM-10

**目的**：确认 fork 前预加载的 SO 不进采集结果。

```bash
PID=$(hdc shell pidof com.example.app)
hdc shell "cat /proc/$PID/maps | grep '\.z\.so' | awk '{print \$6}' | sort -u" > /tmp/all_so.txt
hdc shell "run-as com.example.app cat cache/.ohos.ability_runtime/native_modules_*.v2.json" \
  | grep fileName > /tmp/collected_so.txt
```

**判定**：
- [ ] 预加载锚点 SO（如 `libui_ability_ani.z.so`、`libability_stage_ani.z.so`）出现在 `all_so.txt` 但**不在** collected 文件（ETS 路径）
- [ ] 纯 JS 应用的预加载 SO 同样被排除（JsRuntime::FinishPreload 路径）
- [ ] 日志 `filteredPreload` 计数 > 0（若该应用走 preload 启动）

---

## 阶段 3：多轮 EWMA 收敛与预测（AC-2.1 / 2.2）→ VM-3/4

**操作**：反复杀进程重启同一应用入口 5~8 轮。

```bash
for i in 1 2 3 4 5; do
  hdc shell aa force-stop com.example.app
  sleep 1
  hdc shell aa start -a EntryAbility -b com.example.app
  sleep 8   # 等检查点②写盘
done
hdc shell "run-as com.example.app cat cache/.ohos.ability_runtime/native_modules_*.v2.json"
```

**判定**：
- [ ] `launchIndex` 递增到 5+
- [ ] 每轮必载模块（hilog/window 等）`score` 单调趋近 1.0、`hits` 递增、稳定在 `predicted`
- [ ] 日志 `merge:` 中 `predicted` 数量稳定（收敛）
- [ ] 偶尔加载的模块 score 在 0.3~0.5 间波动、被阈值挡在 predicted 外

**淘汰验证**（VM-4）：构造仅前 2 轮加载、之后不再加载的模块，重启 16+ 轮：
- [ ] 该模块 score 持续衰减 → <0.05 或连续 16 轮未见 → 从 modules 消失（日志 `preEvict > stats`）

---

## 阶段 4：消费预加载生效（AC-3.1 / 3.3 / 3.4）→ VM-7/8

**前置**：先跑阶段 3 让 predicted 非空（含系统模块）。

**操作**：冷启动，观察启动早期预加载。

```bash
hdc shell aa force-stop com.example.app; sleep 1
hdc shell aa start -a EntryAbility -b com.example.app
```

**预期日志**（启动早期，非主线程）：
```
predict preload task registered: .../<hash8>.v2.json
predict preload begin, count=<N>
predict preload <moduleName> ok        # 逐个系统模块
predict preload end, count=<N>
```

**判定**：
- [ ] 日志 count > 0，且每个模块名都在上轮 predicted 列表
- [ ] 故意删一个 so 制造失败：`predict preload <m> failed` + `STARTUP_TASK_ERROR` 故障事件，**其余模块继续、启动不阻断**（AC-3.3）
- [ ] 清空采集文件后启动：`no predicted system module to preload`，无任何预加载动作（AC-3.4）
- [ ] 启动早期 `cat /proc/$PID/maps` 见 predicted 系统 SO 已映射（早于应用业务 import）

---

## 阶段 5：防自反馈（AC-2.5）→ VM-6

**目的**：确认消费预加载的模块不会因"被预加载"自我强化成永久预测项。

**操作**：连续多轮启动；每轮检查 collected JSON。

**判定**：
- [ ] 仅被消费预加载、应用从未真实 import 的系统模块：`score` **不上升**，多轮后被淘汰
- [ ] 应用真实 import 的模块：score 正常上升（loadCount 增量 > 预加载次数时计命中）
- [ ] 关键：日志 `merge:` 的 `predicted` 数应稳定，**不该单调增长**

---

## 阶段 6：异常路径（AC-1.2 / 2.3 / 2.4 / 3.2）→ VM-2/5

| 场景 | 操作 | 预期判定 |
|------|------|----------|
| **开关关闭零影响**（AC-1.2） | `param set ...enable false` 后启动 | 无 `checkpoint/save/merge` 日志；无文件生成；启动耗时与关闭一致 |
| **文件损坏重建**（AC-2.3） | `echo "{broken" > native_modules_*.v2.json` 后启动 | 日志 `rebuild stats: json parse failed`；冷启动重建；启动正常 |
| **version 不符重建**（AC-2.3） | 改 JSON `"version":1` 后启动 | 日志 `rebuild stats: version mismatch`；重建 |
| **缓存清理重建**（AC-2.3） | 删除整个 `.ohos.ability_runtime` 后启动 | 下轮正常重建（coldStart=1） |
| **应用升级重建**（AC-2.4） | 重装高 versionCode 应用后启动 | 日志 `rebuild stats: app upgraded`；历史清空、launchIndex 重置为 1 |
| **消费侧文件缺失**（AC-3.2） | 删文件后启动 | `no predicted system module`；启动不受影响 |

---

## 阶段 7：参数调优（TASK-8）

**操作**：修改 EWMA 参数后多轮启动，观察收敛行为变化。

```bash
hdc shell param set ccm.ability.native_module_collect.alpha 0.7                # 更"短视"
hdc shell param set ccm.ability.native_module_collect.predict_threshold 0.8    # 预测更保守
# 重启应用使参数重新加载（进程内缓存，R-2）
```

**判定**：
- [ ] 启动日志 `params loaded: alpha=0.700 threshold=0.800` 反映新值
- [ ] α=0.7 时命中模块更快逼近 1、遗漏更快衰减；阈值 0.8 时 predicted 集变小
- [ ] 非法值（`param set ...alpha abc`）→ 日志 `invalid param ...` + 回落缺省 0.4

---

## 阶段 8：beta 命中率统计（TASK-9，可选）

**前置**：`report_hitrate=true`，且已有 ≥2 轮历史。

**判定**：
- [ ] 每轮写盘后日志 `hitRate 0.xx falseRate 0.xx predicted N actual M`
- [ ] HiSysEvent 可查 `NATIVE_MODULE_COLLECT_HITRATE` 事件
- [ ] 稳定后 hitRate 应 ≥0.9（spec 非功能目标）

---

## 已知修复（验证时关注回归）

| 提交 | 问题 | 验证关注 |
|------|------|----------|
| `3e482f6e10` | EnsureDir 拼接丢失分隔符（/datastorage） | 阶段 1 无 mkdir 失败日志 |
| `024f0052f5` | 冷启动 consumer 自反馈、firstStage terminate、GetParams 竞争 | 阶段 5 predicted 不单调增长；损坏 firstStage 文件不崩溃 |
| `5dd3d49fb1` | 纯 JS 路径 preload 未捕获 | 阶段 2 纯 JS 应用也排除 preload SO |

## 验证记录（执行后回填）

| 阶段 | 执行日期 | 结果 | 证据（日志/文件快照） | 备注 |
|------|----------|------|----------------------|------|
| 0 UT | 2026-08-03 | PASS | `run -t UT -ts native_module_collector_test` | 含 EnsureDir 拼接/冷启动自反馈等回归用例 |
| 1 采集落盘 | 2026-08-03 | PASS | native_modules_<hash8>.v2.json 生成、0600、字段完整、每轮≤1 次 | VM-1/9/10/11 设备确认 |
| 2 preload 排除 | 2026-08-03 | PASS | /proc/self/maps 含预加载 SO、采集文件不含 | VM-10（含纯 JS 路径）设备确认 |
| 3 EWMA 收敛 | 2026-08-03 | PASS（收敛部分） | launchIndex 递增、必载模块 score 趋近 1、predicted 稳定 | VM-3 设备确认 |
| 3' 淘汰验证 | 待补 | DEFERRED | — | 当前测试 app 无法构造"仅前 N 轮加载"场景；淘汰逻辑（<0.05 / 16 轮未见 / 1024 上限）已由 UT EvictStats_* 覆盖，设备侧回归待后续 app 就绪后补 |
| 4 消费预加载 | 待执行 | — | | VM-7/8 |
| 5 防自反馈 | 待执行 | — | | VM-6 |
| 6 异常路径 | 待执行 | — | | VM-2/5 |
| 7 参数调优 | 待执行 | — | | |
| 8 命中率 | 待执行 | — | | |

> 阶段 0~3（核心采集链路）已设备验证通过；阶段 4~8 与淘汰设备回归待续。
> 本表持续更新；全部 PASS 后即 ohos-review GC 门禁 OI-4（新鲜验证证据）的满足物，可解除 GC 阻塞。

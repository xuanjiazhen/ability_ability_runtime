# verification-playbook — Mode A 验证手册（配方与坑，两轮试点实证）

> 定位：**方法配方**，不是规则。每节都是可直接执行的命令级配方 + 实测踩坑记录。
> 来源：试点一（FIND-003 跨仓故障链 → Reproduced）、试点二（extractortool 精读 → EXT-01/02 Reproduced）。
> 参考资产：`codecheck_reports/frameworks_native_runtime_20260908/ut_harness_FIND-003/`、`ut_harness_EXT/`（两套可复跑 harness）。

## §1 跨仓拉源（S4）

**仓名解析**（唯一可靠信源 = 本仓 remote URL 模式 `gitcode.com/openharmony/<name>`）：

| 需要的组件 | gitcode 仓名 | 备注 |
|---|---|---|
| ArkTS 引擎（JSNApi/module 解析/js_pandafile） | `arkcompiler_ets_runtime` | |
| napi/native_engine（napi_create_ark_runtime 等） | **`arkui_napi`** | ⚠️ `arkcompiler_napi` 是 404，别再试 |
| extractortool/FileMapper/FilePathUtils | `ability_ability_base` | 本地常驻 `/root/ability_ability_base` |
| 事件框架（EventRunner/EventHandler 语义） | `appexecfwk_core`（待验证名） | |
| 卡片/FRS | `arkui_form_fwk` 类（待验证名） | FIND-002 遗留验证点 |

**纪律**：只走 gitcode（gitee/github 实测超时不可达）；仓名不确定时先 `curl -o /dev/null -w "%{http_code}" https://gitcode.com/openharmony/<候选>` 探活（404/200 秒判），再 clone。

**clone 配方**：
```bash
git clone --depth 1 --filter=blob:none --sparse https://gitcode.com/openharmony/<repo>.git /root/oh_repos/<repo>
cd /root/oh_repos/<repo>
git ls-tree -r --name-only HEAD | grep <关键符号>   # 先定位再取目录，避免猜错 sparse 路径
git sparse-checkout set <dir1> <dir2>
```
- **持久缓存** `/root/oh_repos/`：跨轮复用（试点二 S4 零 clone 成本即受益于此）
- 超时预算 5min/仓；失败 → 假设标 `Reasoned-pending` 继续推进，不卡管线

**追踪方法**（比 clone 更重要）：
- 找**模式开关/回调的唯一生产设置点**：`grep -rn "Set<Hook>" | grep -v test` —— 调用点数量决定部署面（试点一：SetExecuteBufferMode 全仓唯一调用点 = napi_create_ark_runtime，一举定案）
- systemapi/权限门控查 SDK d.ts（`/root/interface_sdk-js/api/*.d.ts` 的 `@systemapi` 标注）——决定攻击者身份
- 上游守卫查服务侧（如 AppMgr provision guard）——影响 likelihood 而非存在性

### §1.1 OH 全仓调用方搜索（接口行为变化影响分析 / 跨仓调用链，硬需求）

gitcode 匿名代码搜索 API 不可用（需 private-token / 404 实测），唯一可靠路径 = **manifest 仓清单 + 定向 clone + 本地 grep**（实测：ability_form_fwk 21MB 秒级）：

```bash
# ① 全仓清单（一次性，已在 /root/oh_repos/manifest）
git clone --depth 1 https://gitcode.com/openharmony/manifest.git /root/oh_repos/manifest
grep -o 'name="[^"]*"' /root/oh_repos/manifest/ohos/ohos.xml   # 513 仓全名单

# ② 接口变更 → 候选仓圈定：改动符号所属组件名 → 各仓 bundle.json/BUILD.gn 依赖反查
#    （子系统命名规律：<子系统>_<仓名>，如 form_fwk→ability_form_fwk、napi→arkui_napi）

# ③ 调用方搜索用 depth-1 全量 clone（不要 --filter=blob:none——lazy blob 会让全仓 grep 极慢）
git clone --depth 1 https://gitcode.com/openharmony/<repo>.git /root/oh_repos/<repo>
cd /root/oh_repos/<repo> && git grep -n "<符号名>" -- '*.cpp' '*.h' | grep -v test
```

**报告真实性硬规则**（conventions 同级效力）：
- 每条跨仓论断必须带锚点：`<repo>@<commit 短哈希> <file>:<line>`（depth-1 clone 后 `git rev-parse --short HEAD` 取）
- 拉不到/搜不到的仓 → 结论标 `pending(<仓名>)`，**不得写成结论**；禁止凭记忆/推断描述对端代码
- 分支一致性：clone 默认 master；目标 MR 属 release 分支时声明分支差异风险
- Mode B 场景（门禁）：diff 触及导出头文件/IPC 接口时，**必须**执行本节搜索并在报告列出调用方清单（或 pending 声明），否则该维度记 insufficient
- Mode A 场景（漏洞挖掘）：构造攻击场景的每个跨仓环节（谁创建环境/谁传参/谁消费）都要落到对端 file:line，示例：FRS 卡片环境实证 `ability_form_fwk@<sha> form_render_record.cpp:217/528/531`（独立 EventRunner + isBundle=true）

**实证案例**：FIND-002（UV 任务 UAF）的最后一个跨仓候选（FRS 销毁窗口）即由本节配方 3 分钟内闭环证伪。

### §1.2 S1 面清单机械基底：注册点穷举 + 三面问

**hook = 回调注册点**：一个组件把回调/函子交给另一组件持有的接口（`SetHostResolveBufferTracker`、`RegisterMissionListener` 类）——跨组件行为的注入点，"谁注册/谁消费/什么开关决定被调"决定一条链的真实部署面（FIND-003 即靠追 SetExecuteBufferMode 唯一设置点定案）。

**穷举范围与排除**（实测教训两条：①只扫 interfaces/+frameworks/ 会漏 services/cli_tool_framework/agent_runtime_framework 头里 ~18% 的 hook，含 climgr 会话回调族等热区高危面；②不排除 cj/ets/test/mock 会混入 40+ 静态绑定与测试噪音）：

```bash
# ① hook 全集穷举（2026-09 基线 ≈131 个，有限集合）
#    动词族 Set/Register/Install/Add/Subscribe × 后缀族 Tracker/Callback/Func/Handler/Delegate/Hook/Listener/Observer
#    排除：cj_*/ets_* 静态语言绑定（AGENTS 默认排除约定）+ frameworks/ets|cj + test/mock
git grep -h -oE "(Set|Register|Install|Add|Subscribe)[A-Z][A-Za-z]*(Tracker|Callback|Func|Handler|Delegate|Hook|Listener|Observer)\b" \
    -- '*.h' ':(exclude)**/cj_*.h' ':(exclude)**/ets_*.h' ':(exclude)frameworks/ets/**' \
       ':(exclude)frameworks/cj/**' ':(exclude)**/test/**' ':(exclude)**/mock*' | sort -u
# 定位声明处以确定暴露级：同参数换 -n 输出 file:line

# ② 三面问（每个入选 hook 三行命令）
git grep -n "<HookName>" -- '*.cpp' | grep -v test        # 谁注册（本仓 file:line）
# 谁消费：按 §1 仓名映射到对端仓 grep 消费点（引擎类 hook 多在 ets_runtime/napi/ace_engine）
git grep -rn "<HookName>\|<对应开关符号>" /root/oh_repos/<repo> | grep -v test   # 什么开关决定被调（唯一生产设置点）
```

**已知不完备性**：命名族正则抓不到非常规命名的注入点（如直接传 std::function 形参、构造函数注入）——机械穷举只是基底，推理推导部分（S0 倒排+热点清单+精读）负责补集，报告中不得以"已穷举"暗示全覆盖。

**暴露级判定 = 构建元数据三步判据**（目录只是预筛，**GN/.map/依赖图才是权威**）：

```bash
# 判据1 头文件公开性：声明所在目录是否进某 target 的 public_configs include_dirs
grep -rn "public_configs" --include=BUILD.gn <声明头所在组件目录>   # 展开 config 看 include_dirs 是否含该目录

# 判据2 动态符号导出：所在 so 的 version_script（.map）global 段是否导出该符号
grep -n "version_script" <组件>/BUILD.gn          # 如 services/abilitymgr → libabilityms.map
grep -n "<符号或类名>" <组件>/*.map               # global: 段内才可被外部链接；无 .map 的 target 如实记录"默认导出"

# 判据3 依赖反查（真实消费方清单）：
grep -rn '"ability_runtime:<target>"' --include=BUILD.gn .        # 仓内：谁 deps 了该组件
git grep -rn '"ability_runtime:' /root/oh_repos/<候选仓> -- '*.gn' # 跨仓：候选仓（manifest 圈定）谁依赖本仓组件
```

| 判定结果 | 暴露级 | 三面问深度 |
|---|---|---|
| 判据1+2 开 且 判据3 有跨仓依赖 | **跨仓面** | 消费方跨仓必查（§1/§1.1），查不了标 pending(仓名) |
| 判据1 开、依赖仅仓内（或 .map 未导出） | **仓内公共面** | 本仓全量 grep 即可 |
| 判据全关（私有头/未公开 include） | **模块内部面** | 按 S0 影响倒排相关性入选，不强制 |

> 目录启发（interfaces/≈导出、services/≈内部）只用于**排序优先级**，不得作为暴露级结论——实测反例：agent_runtime_framework/services/ 下亦有 version_script 导出（libagentmgr.map），frameworks/ 下亦有纯内部头。

产出格式（面清单行）：`hook | 暴露级(跨仓面/仓内公共面/模块内部面) | 注册方 file:line | 消费方 repo@sha file:line 或 pending(仓名) | 被调开关 | 入选面清单(Y/N+理由)`

纪律：
- 消费面回答不了的 hook 一律 `pending` 进报告"盲区欠账"节，**禁止静默跳过、禁止推断代答**
- 每轮按 S0 影响倒排选高危 hook 做完整三面问（全量穷举用于架构级审计或新模块首审）
- hook 清单随仓库演进重跑刷新，不缓存旧清单
- **变更驱动的影响分析**（Mode B 门禁场景）不走本节，走 §1.1：diff 触及任何路径（含 services/ 实现行为变化）的导出符号/IPC 语义时全仓搜调用方

## §2 harness 编译配方（S5）

**原则**：编译**真实被测源文件**（绝不拷贝逻辑重写），依赖分层桩化，桩行为逐项对照真实源码语义并在报告声明忠实度边界。

**host 编译三件套**（OH 源码在 Ubuntu g++ 下的固定坑）：
```bash
g++ -std=c++17 -g -O1 -w -include vector -include limits.h \
    -I$H/stubs -I<真实include目录> <真实源文件...> $H/test_main.cpp -lz -o test_bin
```
- `-include vector -include limits.h`：OH 头文件依赖构建系统间接包含（PATH_MAX/vector 未显式 include）
- `%{public}s` 格式串：日志桩用 no-op 宏整体吞掉；诊断期切 stderr 版本（glibc 按字面输出 `%{public}`，可定位行）
- 未知 mmap flag（如 MAP_XPM=0x40）：宿主内核忽略或按 MAP_32BIT 处理，不阻塞；**量产内核行为差异必须写入忠实度声明**

**通用桩包**（`ut_harness_*/stubs/`，可直接复制复用）：
| 桩 | 要点 |
|---|---|
| `securec.h` | memcpy_s/memset_s 包装 + EOK=0（注意 ERANGE 重定义警告无害） |
| `ability_base_log_wrapper.h` / `hilog_tag_wrapper.h` | **双态**：默认 no-op；诊断版 fprintf stderr（宏必须吞掉 tag 首参：`#define TAG_LOGE(tag, ...)`） |
| `hitrace_meter.h` | HITRACE_METER_NAME no-op |
| `unzip.h` | 仅需 `typedef uint64_t ZPOS64_T` + include zlib.h |
| `string_ex.h` | LowerStr 等小工具，放对命名空间（OHOS::AbilityBase） |
| BMS/singleton/iservice_registry | 可配置假实现：**返回值形态按真实消费代码的匹配语义配置**（不是随便返回成功） |

## §3 恶意/边界样本构造（S5）

- **zip 字段级控制**：python struct.pack 手工构造 local/central/EOCD（模板见 `ut_harness_EXT/build_zips.py`）——标准 zipfile 库无法表达恶意字段
- **稀疏文件**：`f.truncate(N)` 后**必须 `f.seek(N)`**（truncate 不移动写指针——实测踩坑）；4GB 逻辑文件磁盘实占 12KB，是低成本 DoS 样本标准构造
- **STORED entry**：`zipfile.ZIP_STORED`（免解压，简化 sink 语义）
- **载荷锚定约束求解**：穿越路径需逐段存在（挂载点/攻击者可建目录），先画出内核解析全序列再定 `..` 数量（试点一载荷经三轮约束求解才成立）

## §4 验证纪律（S5-S6）

1. **对照组先行**：benign 样本先证 harness 正确（match=YES），恶意案例结论才可采信
2. **红灯不变式用例**：断言"安全性质"（如：加载路径 realpath 必须在代码区前缀内）——当前 FAIL=缺陷证明，修复后 PASS=回归用例，一条用例两用
3. **资源约束多形态**：同一缺陷跑默认 + `ulimit -v` 变体（EXT-02：8.4GB 假成功 vs exit 134 abort，两种结局都是证据）
4. **定量外推**：小样本测单轮成本 → 线性外推真实量级（EXT-01：3M offset 41ms → 4G offset 实测 58,987ms，外推误差 <3%）
5. **忠实度边界声明**（每份验证包必附）：哪些是真实源码、哪些是桩（及桩对照的真实语义）、哪些环节留在跨仓/真机（如 XPM 内核策略、引擎侧字段校验）
6. **攻击成本标注**：样本磁盘实占、前置条件（依赖声明/systemapi/开发者模式）、可重复性
7. **验证状态词表**：Reproduced > Reasoned（全链 file:line）> Unverified；Refuted 必须给硬证据（上游守卫 file:line / 唯一调用点 / 内核语义），**纯仓内阅读不得推翻 P1**

## §5 历史记忆检索（"犯过一次不重犯"的查找路径）

按可靠性排序：
1. **案例索引**：`static-checklist.md` 附录二（已验证严重案例 + 防重犯载体）——工作区内第一入口。
2. **热点清单**：`static-checklist.md` 附录一（历史复发模块 + 问题特征）——S1 面推导必查。
3. **gitcode MR 平台**：⚠️ 本仓 git log 为压扁合并标题（`!NNNN merge xxx`，33k 提交实测关键字挖掘无信号），**历史修复语义只在 MR 平台的 diff/评审记录里**，需要时上平台按模块/时间检索，不要指望本地 git log。
4. **回归 UT**：已修复问题的最终记忆载体是仓库 test/ 下的红绿用例；审查修复时先确认"该问题的回归用例存在且会拦复发"。

## §6 验证包归档结构

```
codecheck_reports/<scope>_<date>/
├── pilot<N>_verification_<target>.md   # 假设→分诊→跨仓→复现全过程 + 忠实度声明
├── pilot<N>_UT_runlog.txt              # 原始运行日志
└── ut_harness_<target>/                # 可复跑：run.sh + 真实源引用 + stubs/ + 样本构造脚本
```

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

## §5 验证包归档结构

```
codecheck_reports/<scope>_<date>/
├── pilot<N>_verification_<target>.md   # 假设→分诊→跨仓→复现全过程 + 忠实度声明
├── pilot<N>_UT_runlog.txt              # 原始运行日志
└── ut_harness_<target>/                # 可复跑：run.sh + 真实源引用 + stubs/ + 样本构造脚本
```

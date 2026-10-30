---
name: divergence-scanner
description: >
  对"蓝区（开源基线）vs 黄区（内部仓）"代码差异做风险分级扫描。
  把相对开源基线的 diff hunk 套到 9 层 68 个差异点模式词典上，按 P0–P3
  归一化风险等级，并对 IPC Stub/Proxy、接口签名、权限校验等高风险差异
  做配对校验。蓝黄两端混合运行/编译时，IPC 序列化读写不对称是单测
  最难发现的一类——本 scanner 优先扫这一层。
  触发方式：
  1. 用户输入 /divergence_scan {path} [--baseline <ref>]
  2. 用户用自然语言表达"蓝黄差异"、"分支差异扫描"、"代码差异风险"、
     "divergence scan"、"差异点排查"、"蓝区黄区对比"、"开源基线对比"等意图。
  输出：按 P0/P1/P2 优先级排序的差异风险清单 + 统一报告。
---

# Divergence Scanner — 蓝黄代码差异风险扫描

## 定位

以**蓝区（开源基线）为参照**，对黄区相对基线的 **diff hunk** 做风险分级。不是扫"代码本身有没有 bug"，而是扫"这段差异会不会在蓝黄混合运行/编译时出事"。

**核心洞察**：单测两端同源，无法暴露蓝黄不对称问题；最危险的一层是 IPC Stub/Proxy 序列化——读写不对称时静默错值/崩溃。

**Pattern 引用声明**：本 scanner 加载以下 pattern 文件作为检查规则来源：
- [`references/divergence_patterns.md`](./references/divergence_patterns.md) — DIV-001~068（9 层差异点模式词典，含 grep/diff 线索与风险等级）

---

> **Path B 声明**：本 scanner 直接被调用时，除产出下述原始差异风险清单外，必须按 [`conventions.md`](../../conventions.md) §5–§13 规程生成符合 `codecheck_report_TEMPLATE.md` 的统一报告，其中 P0 项必须做 refute（见 conventions §13.2）。

## 核心原则

- **方向**：以 diff（黄区相对蓝区基线）为输入，逐 hunk 匹配模式词典；不是全量代码审计
- **优先级**：IPC 序列化（DIV-001~007）> 接口签名（013~018）> 权限安全（031~039）> 公共 API（021~030）> 其余
- **配对校验**：命中 IPC 序列化层时必须做 Stub↔Proxy 配对校验（见 §配对校验速查表）
- **证据要求**：每个发现必须能写出"基线 X，黄区改成 Y，蓝黄混合运行时后果 Z"，并给出基线与黄区两端的 `file:line`
- **输出**：差异风险清单（Markdown）+ 统一报告

---

## 工作流

### Step 1: 建立基线与差异输入

**1A. 确定蓝区基线**

向用户确认蓝区基线引用（优先级从高到低）：
1. 用户显式指定 `<baseline>`（分支名 / tag / commit SHA）
2. 仓库默认基线：`origin/master` / `master` / `upstream/master`
3. 用户直接提供 diff 文件或变更文件清单

**1B. 生成 diff**

```bash
# 方式一：全量 diff（推荐，覆盖所有变更文件）
git diff <baseline>..HEAD --stat
git diff <baseline>..HEAD

# 方式二：仅看本分支独有提交
git log --oneline <baseline>..HEAD
git diff <baseline>..HEAD --name-only

# 方式三：用户只给了变更文件清单，逐文件 diff
git diff <baseline>..HEAD -- <file1> <file2> ...
```

**1C. 采集版本信息**（按 conventions §6）：

```bash
git rev-parse HEAD
git log -1 --format="%b" | grep -o 'Change-Id: I[0-9a-f]\{40\}'
```

**1D. 排除范围**（按 conventions §1）：
- 静态语言实现默认排除：`frameworks/ets/ani/`、`frameworks/ets/ets/`、`frameworks/cj/ffi/`、`ets_*.cpp`、`cj_*.cpp`，除非用户明确要求包含。
- 纯测试 mock（`mock_*.cpp`）默认扫描但降级一档（仅签名差异，无运行风险）。

---

### Step 2: 变更分层归类

把每个变更文件/hunk 映射到 9 层中的一层或多层。分层决定后续走哪类 pattern。

| 变更路径特征 | 归入层 | 必查 pattern |
|--------------|--------|--------------|
| `services/**/*_stub.cpp` / `*_proxy.cpp`，diff 含 `data.Write/Read` | 层一 IPC 序列化 | DIV-001~007 |
| `interfaces/inner_api/**/*.h`，diff 含虚函数签名 | 层二 接口签名 | DIV-013~020 |
| `interfaces/kits/**`、`*.map`、`frameworks/{js,ets,cj}/` | 层三 公共 API | DIV-021~030 |
| `services/common/**/permission_verification*`、`services/uripermmgr/`、`app_spawn*` | 层四 权限安全 | DIV-031~039 |
| `hisysevent.yaml`、`hilog_tag_wrapper.h`、`XCollie` | 层五 DFX | DIV-040~044 |
| `services/abilitymgr/src/mission/`、`*Create*Table*`、`*FromJson*` | 层六 持久化 | DIV-045~049 |
| `ability_runtime.gni`、`BUILD.gn`、`bundle.json`、`services/sa_profile/` | 层七 构建 | DIV-050~055 |
| diff 含锁/线程/状态机/回调注册 | 层八 并发生命周期 | DIV-056~063 |
| diff 含上限/阈值/重试 | 层九 资源限制 | DIV-064~068 |

**探测信号**：

```bash
# IPC 信号（最高优先级）
rg -n "data\.(Write|Read)\w+|reply\.Write\w+" --type cpp <changed_files>
# 接口签名信号
rg -n "virtual\s+.*\(.*\)\s*(const)?\s*(override)?\s*=\s*0" --type cpp <changed_headers>
# 权限信号
rg -n "VerifyPermission|CheckPermission|GetCallingTokenID|ohos\.permission\." --type cpp <changed_files>
# 持久化信号
rg -n "CREATE TABLE|ALTER TABLE|Insert\s*\(|ExecuteSql|FromJson" --type cpp <changed_files>
```

---

### Step 3: 逐 hunk 模式匹配

对每个变更 hunk，按 [`references/divergence_patterns.md`](./references/divergence_patterns.md) 的"信号特征"逐条比对。命中即记录：

```text
FIND-001 | DIV-003 | P0 | services/abilitymgr/src/ability_manager_proxy.cpp:891
  基线: 无 WriteInt32(userId)
  黄区: 新增 data.WriteInt32(userId)
  证据: git diff hunk @@ -888,6 +888,11 @@
  机制: stub 若无对应 ReadInt32 → 后续字段错位
  配对状态: 待 Step 4 确认
```

**匹配要求**：
- 每个 hunk 至少套一遍层一（IPC 序列化）的 7 条，无论文件是否明显属于 IPC——因为接口签名层默认值常引入隐式序列化差异（DIV-004）。
- 命中层一任一条时，**必须进入 Step 4 配对校验**。
- 未命中任何 pattern 的 hunk 记为"低风险变更"，不计入 findings。

---

### Step 4: 配对校验（仅层一必做，其余可选）

对命中 DIV-001~007 的每条发现，做 Stub↔Proxy 配对校验：

**4A. 定位对端**

| 命中位置 | 对端文件 |
|----------|----------|
| `*_proxy.cpp` 的 `WriteXxx` | 同接口的 `*_stub.cpp` 的 `ReadXxx`（同函数名/同 code） |
| `*_stub.cpp` 的 `ReadXxx` | 同接口的 `*_proxy.cpp` 的 `WriteXxx` |
| `*_interface.h` 的虚函数签名 | `*_proxy.h` + `*_service.h` 的 override 声明 + `*_proxy.cpp` + `*_service.cpp` 实现 + `*_stub.cpp` 调用 |

**4B. 对照配对速查表**（见 patterns 文件末尾）：
- 类型一致（`WriteInt32` ↔ `ReadInt32`，不可 Int32↔Uint32 混用）
- 顺序一致（基线两端顺序 vs 黄区两端顺序）
- 数量一致（新增字段必须 proxy+stub 成对出现）
- 默认值一致（接口签名的默认参数，两端编译期独立生效，不可靠运行时对齐）

**4C. 判定**：

| 配对结果 | 风险判定 |
|----------|----------|
| 基线对称、黄区两端**成对**改动 | P2（行为差异但内部自洽，仍可能与旧蓝端客户端不兼容） |
| 基线对称、黄区**只改一端** | **P0**（读写不对称 → 错位/崩溃） |
| 基线对称、黄区两端改动**顺序/类型不一致** | **P0** |
| 基线无此字段、黄区新增且两端成对 | P1（与旧蓝端不兼容） |

---

### Step 5: 权限/安全差异专项核对

对命中层四（DIV-031~039）的发现，专项核对（蓝黄两端校验宽严差异 = 安全漏洞）：

- 列出**基线**该校验点的判定分支与放行条件
- 列出**黄区**同位置的判定分支与放行条件
- 比对：黄区是否新增放行分支 / 删除校验 / 改宽判定
- 若黄区放行更宽或删了校验 → P0（安全边界破坏）
- 若黄区收紧 → P2（行为差异，通常更安全但仍需对齐）

**核对命令**：

```bash
# 取基线版本该校验函数
git show <baseline>:services/common/include/permission_verification.h
# 取黄区版本
git show HEAD:services/common/include/permission_verification.h
# 比对
git diff <baseline>..HEAD -- services/common/include/permission_verification.h
```

---

### Step 6: 生成差异风险清单（原始产出）

Markdown 清单，按风险等级排序，每条含：

| 列名 | 内容要求 |
|------|----------|
| `差异ID` | 如 `DIV-FIND-001` |
| `模式编号` | 命中的 DIV-xxx |
| `风险等级` | P0 / P1 / P2 / P3 |
| `变更文件` | 黄区相对路径 |
| `行号` | `891` 或 `888-898` |
| `层` | 层一~层九 |
| `基线行为` | 基线在此处做什么（一句话） |
| `黄区改动` | 黄区改成了什么（一句话） |
| `风险机制` | 蓝黄混合运行时的后果 |
| `配对状态` | 已配对/未配对/不适用（层一专用） |
| `修复建议` | 具体、可操作 |

**清单文件名**：`divergence_findings_<scope>_<YYYYMMDD>.md`

---

### Step 7: 生成统一报告

按 conventions §5–§13 规程生成 `codecheck_report_<scope>_<YYYYMMDD>.md`，遵循 `codecheck_report_TEMPLATE.md`。

**等级归一化**（conventions §7.2 对应表）：

| 本 scanner 内部 | → P0 | → P1 | → P2 | → P3 |
|-----------------|------|------|------|------|
| 词典标注 P0 | P0 | — | — | — |
| 词典标注 P1 | — | P1 | — | — |
| 词典标注 P2 | — | — | P2 | — |
| 词典标注 P3 | — | — | — | P3 |

（恒等映射；多条规则同 `file:line` 命中取最高等级）

**ID 前缀**：`DIV-`（如 `DIV-01`）

**必检维度自检**（conventions §8）：
- divergence-scanner 是"差异维度"的专项扫描，不替代 security/logic/input/api 的全量审计
- `dimensions_executed = ["divergence-scanner"]`
- 若变更范围涉及 `services/` 下 IPC/DB 密集区 → `dimensions_required` 还应含 `security-scanner + logic-scanner + input-scanner`；本 scanner 必须提示用户补扫
- 若变更范围涉及 `interfaces/kits/` → 还应含 `api-scanner`；同上提示

**P0 refute**（conventions §13.2）：所有 P0 项必须做触发路径证伪 + 影响夸大证伪，产出 `refute_log.md`。证伪关键点：该差异在蓝黄混合运行时是否真的可达（如该 code 是否被旧蓝端客户端实际调用、该接口是否已导出供外部使用）。

---

## 风险等级判定（汇总）

| 等级 | 条件 |
|------|------|
| **P0 (Critical)** | IPC 读写不对称（DIV-001~007 单边改动）、接口码错位（008/009）、虚函数签名/默认值差异导致 vtable 错位（013/014/017/018）、权限校验放行变宽（031~036）、锁顺序/状态机/异步清理差异（056/058/060） |
| **P1 (High)** | 公共 API 签名/错误码/NAPI 绑定差异（021~028）、DFX 事件/参数差异（040~044）、RDB 表结构差异（045~049）、构建配置/SA profile 默认值差异（050~053）、并发时序/回调差异（057/059/061/062） |
| **P2 (Medium)** | 资源限制差异（064~068）、模板/constexpr 差异（020）、可见性配置差异（039）、日志 tag 差异（042） |
| **P3 (Low)** | 仅风格/潜在风险，当前不可达 |

---

## 执行注意事项

1. **必须先拿到 diff**：本 scanner 不做全量代码审计，没有基线 diff 时先要求用户指定 `<baseline>`，不可凭空扫描。
2. **IPC 序列化层是重点**：即便 hunk 看起来只是"加个参数"，也要检查它是否经 IPC 序列化——接口签名层的默认值差异（DIV-004）常隐式引入序列化不对称。
3. **配对校验不可省**：命中层一必须做 Step 4，未配对即 P0。
4. **测试 mock 降一档**：`mock_*.cpp` 的签名差异仅影响测试编译，无运行风险，最高 P2。
5. **每条发现必须有双端证据**：基线 `file:line` + 黄区 `file:line`，缺一不可计入 P0/P1。
6. **引用代码格式**：`path/to/file.cpp:123`（黄区）+ `path/to/file.cpp:123@<baseline>`（基线）。
7. **静态语言实现默认排除**：`frameworks/ets/ani/`、`frameworks/cj/ffi/`、`ets_*.cpp`、`cj_*.cpp`，除非用户明确要求包含。
8. **优先报告 P0/P1**，控制 P2/P3 数量保证信噪比。
9. **不可改源码**：检视阶段只产出报告与建议（conventions §3）。

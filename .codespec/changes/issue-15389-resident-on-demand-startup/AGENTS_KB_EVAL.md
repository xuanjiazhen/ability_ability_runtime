# ability_runtime AGENTS.md 知识评估报告

> 评估基准：[kb-evaluation/01_知识评测总规范.md](../ouha-ai-context/kb-evaluation/01_知识评测总规范.md)
> 评估日期：2026-06-16
> 评估对象：`ability_runtime/AGENTS.md`
> 评估需求：REQ-001 常驻进程按需启动

---

## 一、评估方法

基于 `01_知识评测总规范.md` 定义的 4 类知识类型（代码地图、知识路由、专家经验、编译和测试方法），以常驻进程按需启动需求为实战案例，逐项评估 AGENTS.md 对该需求的覆盖度和准确度。

---

## 二、分类评估

### 2.1 代码地图（Code Map）

**评估标准**：是否帮助 AI 快速建立代码仓结构认知，确定某类问题在哪个目录/文件。

| 检查项 | 结果 | 说明 |
|--------|------|------|
| 目录/文件解释 | ✅ 优秀 | 第 9-30 行完整列出 20+ 个目录职责 + 高风险标记（🔴🟡🟢），每个目录一句话概括核心职责 |
| 分层说明 | ✅ 优秀 | 区分 services/、frameworks/、interfaces/ 三层结构，明确"服务层与 SDK 接口层的双向解耦" |
| Where to look | ✅ 优秀 | 第 32-54 行按任务类型给出"先看什么"的精确指引，覆盖 15+ 种常见任务 |

**实战验证**：
- 常驻进程按需启动涉及 `services/abilitymgr/` 修改 → 第 36 行直接指向正确路径
- 涉及 `tools/aa/` 命令行工具 → 第 29 行标注为 🟢 低风险
- RDB 数据管理在 `services/abilitymgr/src/rdb/` 下 ✅
- IPC 接口修改在 `interfaces/inner_api/` 下 ✅

**评分：9/10**

---

### 2.2 知识路由（Knowledge Routing）

**评估标准**：是否帮助 AI 从问题类型直接映射到阅读路径，理解仓内术语。

| 检查项 | 结果 | 说明 |
|--------|------|------|
| Task-based routing | ✅ 优秀 | 第 62-71 行覆盖 8 种任务类别，每种都有"先读"文档 |
| Path-based routing | ✅ 良好 | 第 73-84 行按目录路径给出对应指引 |
| Vocabulary-based routing | ✅ 优秀 | 第 89-108 行覆盖 17 个 OpenHarmony 专有术语，每个都有"不是 X，而是 Y"的风险澄清 |

**实战验证**：
- ✅ `KeepAlive / 保活` 术语在第 103 行有详细说明
- ✅ `SA / SystemAbility` 在第 104 行有说明
- ✅ `Interceptor / 拦截器` 在第 100 行有说明
- ⚠️ **缺失**：`ResidentProcess` / `常驻进程` / `resident` 没有独立词汇条目，实际开发中大量代码围绕 `ResidentProcessManager` 和 `AmsResidentProcessRdb` 展开

**评分：8/10**

---

### 2.3 专家经验（Constraints and Boundaries）

**评估标准**：是否包含约束边界、不能违反的要求、反模式，防止 AI 走错误路径。

| 检查项 | 结果 | 说明 |
|--------|------|------|
| Architecture invariants | ✅ 优秀 | 第 118-127 行列出 7 个核心架构约束 |
| Do not | ✅ 优秀 | 第 129-140 行列出 10 条"不能做"的红线 |
| Ask before | ✅ 优秀 | 第 142-156 行列出 12 个需先问再改的场景 |
| 反模式 | ✅ 优秀 | 第 158-167 行列出 8 个反模式 + 正确做法 |

**实战验证**：
- ✅ 第 139 行"添加源文件后必须更新 BUILD.gn / .gni"——实际开发中确实遇到了 BUILD.gn 遗漏问题
- ✅ 第 136 行"不要为通过测试而删除日志"——日志对问题定位至关重要
- ✅ 第 131-132 行"不要在 interfaces/kits/ 中修改公共 API 签名"——本需求使用的是 InnerAPI，不涉及公共 API
- ✅ RDB 操作不涉及公共 API 变更，不涉及跨版本兼容性风险

**评分：9/10**

---

### 2.4 编译和测试方法（Verification）

**评估标准**：是否提供精确、可执行的最小验证命令。

| 检查项 | 结果 | 说明 |
|--------|------|------|
| Minimum checks | ⚠️ 需改进 | 第 174 行使用 `./build.sh`，但在 ability_runtime 子目录下此命令不可用 |
| Task-specific checks | ✅ 优秀 | 第 180-195 行按变更类型给出精确验证步骤 |
| Done definition | ✅ 优秀 | 第 196-204 行定义 5 条"完成"标准 |
| Final response format | ✅ 良好 | 第 205-212 行定义回复格式 |

**实战验证**：
- ❌ 第 174 行 `./build.sh --product-name <product> --build-target ability_runtime` 不可用——正确命令是 `hb build ability_runtime -i --build-target ams_target`（子目录 `CLAUDE.md` 中记录为 `hb build`，但与 AGENTS.md 不一致）
- ✅ 第 176 行单元测试命令正确
- ⚠️ 缺少集成测试指引（如 hdc 推 SO、设备重启验证等）

**评分：7/10**

---

## 三、综合评分

| 知识类型 | 评分 | 权重 | 加权 |
|----------|------|------|------|
| 代码地图 | 9/10 | 25% | 2.25 |
| 知识路由 | 8/10 | 25% | 2.00 |
| 专家经验 | 9/10 | 25% | 2.25 |
| 编译和测试方法 | 7/10 | 25% | 1.75 |
| **总分** | | | **8.25/10** |

---

## 四、改进建议

| 优先级 | 改进项 | 说明 |
|--------|--------|------|
| P0 | 修正构建命令 | 将 `./build.sh` 改为 `hb build ability_runtime -i --build-target xxx`，或注明从 OpenHarmony 源码根目录执行 |
| P1 | 补充 `ResidentProcess` 术语 | 在 Vocabulary-based routing 中新增常驻进程、ResidentProcessManager、AmsResidentProcessRdb 的术语映射 |
| P1 | 统一 AGENTS.md 与 子目录 CLAUDE.md | AGENTS.md 使用 `./build.sh`，abilitymgr/CLAUDE.md 使用 `hb build`，需统一为实际可用的命令 |
| P2 | 补充集成测试指引 | 增加 hdc 推送、设备重启验证、hilog 日志采集的命令示例 |
| P2 | 补充 RDB 数据管理路由 | 在 Where to look 表中增加"数据持久化存储（RDB 操作）"的指引，指向 `services/abilitymgr/src/rdb/` |

---

## 五、结论

ability_runtime 的 AGENTS.md 整体质量**优秀（8.25/10）**。代码地图和专家经验非常全面，知识路由覆盖了大部分场景。主要短板是编译命令不准确（P0）和常驻进程相关术语缺失（P1），修复后可达 9+ 分。

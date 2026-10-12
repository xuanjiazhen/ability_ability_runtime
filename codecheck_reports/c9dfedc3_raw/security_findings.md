# Security Scanner 原始发现清单

范围：commit c9dfedc32031e66462c295452d329879dce44422 变更行
`frameworks/native/appkit/app/dump_runtime_helper.cpp:65`

## 模块风险画像

- 模块能力：DumpRuntimeHelper — appkit 侧 DFX/OOM 内存转储助手，负责 native 堆泄漏字符串抓取、snapshot 写入、JS/ETS heap dump、配额（xattr）校验。
- 外部输入点（本次变更相关）：无运行时外部输入进入 SO_NAME；SO_NAME 为编译期常量。
- 资源所有权：fd（RequestFileDescriptor→close）、dlopen handle（dlclose）、ffrt 任务、xattr。本次变更不触及。
- 副作用：dlopen 加载指定 .so 并解析符号。
- 并发行为：不涉及。

## Core Scan（11 项）

| # | 检查项 | 结论 |
|---|--------|------|
| ① | 空指针/无效迭代器 | 不适用（仅常量字符串改动） |
| ② | 越界访问 | 不适用 |
| ③ | UAF/生命周期 | 不适用 |
| ④ | 整数溢出/截断 | 不适用 |
| ⑤ | 死锁/竞态 | 不适用 |
| ⑥ | 资源泄漏 | 通过：dlopen/dlclose 配对及失败路径（:442-446、:461-476、:489-521）未因本次改动改变 |
| ⑦ | 反序列化四件套 | 不适用（dlopen 参数为编译期常量，无注入面） |
| ⑧ | 敏感信息泄漏/日志红线 | 通过：SO_NAME 为公开库名，日志打印无新增风险 |
| ⑨ | 系统框架合规/环境契约 | **P3 观察项（SEC-01）**：库名与外部构建产物的一致性无仓内校验 |
| ⑩ | 类安全/虚析构/移动语义 | 不适用 |
| ⑪ | IPC 鉴权 | 不适用（本次变更不触及 IPC；文件内 AppMgrClient/faultloggerd 调用未变） |

## 已知缺陷模式同类横扫（G 类）

- G 类模式库无 dlopen/库名类目；最接近 LOG-011（契约违反）见 logic 产出。
- 全库 grep `libmemleak`/`liborphanmem`：仅本文件 1 处引用，无遗漏改名点。

## 发现清单

### SEC-01 — SO_NAME 库名变更为跨模块外部契约，仓内无校验（P3 / Suspicious / 维度 D）

- 位置：frameworks/native/appkit/app/dump_runtime_helper.cpp:65（dlopen 使用点 :436）
- 触发路径：`DumpMem(NATIVE, needLeakobj=true)` → `GetDumpResult`(:454) / `GetSnapshot`(:482) → `LoadMemLeakFunc`(:436) `dlopen("liborphanmem.z.so")`。若设备上实际产物仍为旧名 `libmemleak.z.so`（版本错配），dlopen 失败 → func==nullptr → 两条 dump 路径 fail-closed 返回 false，仅 HILOG 报错。
- bug 视角：结果影响=DFX native 泄漏/快照抓取静默降级（功能，无安全影响）；触发条件=库名与构建产物不一致；影响范围=仅本功能路径。
- security 视角：审计维度 D（系统框架/环境契约）；无注入（编译期常量，非外部输入）。
- 证据：本次 diff 将 :65 常量由 `"libmemleak.z.so"` 改为 `"liborphanmem.z.so"`；:436 `dlopen(SO_NAME, RTLD_LAZY)`；:430-448 失败路径返回 nullptr；:461-476、:482-521 fail-closed。
- 推荐修复：合入前与库侧（hiviewdfx/orphanmem 模块）确认产物名同步；可加启动期一致性校验或依赖控件加载保证。
- 推荐测试：UT 需在同名桩库下覆盖 dlopen 成功/失败两条路径。
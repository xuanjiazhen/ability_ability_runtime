# Logic Scanner 原始发现清单

范围：commit c9dfedc32031e66462c295452d329879dce44422 变更行
`frameworks/native/appkit/app/dump_runtime_helper.cpp:65`

## 变更识别

- 变更类型：类型 2（数据流变更）——常量值修改，非控制流/状态机/函数签名/错误处理变更。
- 变更内容：`SO_NAME` 由 `"libmemleak.z.so"` 改为 `"liborphanmem.z.so"`（仅 __LP64__ 分支）。
- 直接影响：`LoadMemLeakFunc` (:436) 的 `dlopen` 目标库名。
- 间接影响：`GetDumpResult` (:454) / `GetSnapshot` (:482) 两条 native dump 路径的 dlopen 结果 → fail-closed 行为。
- 一致性：提交消息"libmemleak.z.so 名字更改"与代码一致；仓内无其他旧库名引用（全库 grep 仅 :65 一处）。

## 分析维度

| 维度 | 检查内容 | 结论 |
|------|----------|------|
| 控制流 | 分支/循环/条件 | 通过：dlopen 失败分支 (:430-448) 有完整空指针保护与 dlsym 失败 dlclose |
| 数据流 | 变量/赋值/使用 | 通过：常量仅 :436 一处使用；:432-435 已防御非 64 位空串 |
| 状态机 | 状态转换 | 不适用 |
| 边界条件 | 界/空/极值 | 通过：%{public}s 正确掩蔽；无新增越界面 |
| 错误处理 | 错误路径完整性 | 通过：:461-476、:489-521 各失败路径均 free+dlclose 配对 |
| 并发控制 | 锁/竞态 | 不适用 |
| 业务规则 | 契约/不变性 | **P3 观察项（LOG-01）**：库名为跨模块外部契约，与上游产物改名存在同步风险 |

## 发现清单

### LOG-01 — 硬编码库名与上游产物改名存在版本同步契约风险（🟢 → P3）

- 位置：frameworks/native/appkit/app/dump_runtime_helper.cpp:65
- 触发路径：本次变更把 dlopen 目标名改为 `liborphanmem.z.so`，但本仓无法验证库侧模块（hiviewdfx）是否已同步产出同名 .so。若合入顺序先于库侧改名，线上 OOM/native dump 触发时 dlopen 失败，功能静默降级。
- 影响：DFX native 内存泄漏抓取/快照功能降级；无正确性/安全影响（fail-closed）。
- 证据：:436 `dlopen(SO_NAME, RTLD_LAZY)`；:454/:482 两调用点。
- 修复建议：合入前确认库侧产物名同步（跨仓协调），必要时在加载失败时打可归因日志并回退旧名。
- 严重等级：🟢 建议 → 归一化 P3。

## 修复优先级分组

- P0/P1：无。
- P2/P3：LOG-01（建议合入前跨仓确认库名同步）。
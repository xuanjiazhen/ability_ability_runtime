# Execution Plan — 应用按需释放文件页

> Stage 3 产物。复杂度：标准。状态：**Approved（基线+规格已过）**。

## 一、任务编排

| Task | 标题 | 依赖 | 产出 |
|------|------|------|------|
| T-0 | 开放项实测（O-1/O-2）：rawfile 映射形态 + ZIP 依赖可得性 | — | 确认 ADR-3 A/B 主路径，必要时修订 AC-3 |
| T-1 | Native 引擎：`file_page_releaser` 骨架 + 路径解析/白名单 | T-0 | `frameworks/native/.../memory/file_page_releaser.{h,cpp}` + BUILD.gn |
| T-2 | madvise 工具链扩展（VMA 子区间释放，按 ADR-3 结论） | T-0 | `madvise_utils.{h,cpp}` 增量 + 单测 |
| T-3 | NAPI 绑定：`application.appMemory` + 系统应用校验 + async work | T-1,T-2 | `frameworks/js/napi/app_memory/*` |
| T-4 | ANI 绑定：ArkTS `appMemory` | T-1,T-2 | `frameworks/ets/ani/app_memory/*` |
| T-5 | 错误码与 kit 注册（`bundle.json`/各 BUILD.gn 目标登记） | T-3,T-4 | 配置项 |
| T-6 | 单元测试（白名单/穿越/原子/偏移解析/错误码） | T-1~T-4 | `test/unittest/file_page_releaser_test/` |
| T-7 | 集成验证（系统应用 Demo + hidumper smap 对比） | T-1~T-5 | AC-1~AC-3 实测证据 |

## 二、关键路径与里程碑

1. **先做 T-0**（实测）：在任一系统应用进程 `cat /proc/self/maps | grep hap`，确认 hap 是否整体映射；检查 ability_runtime 既有 external_deps 是否含 zip/minizip。
   - 若 hap 整体映射且 ZIP 依赖可得 → 走 ADR-3 方案 A。
   - 若 rawfile 解包到磁盘 → 走方案 B（`MadviseGeneralFiles` 直释）。
2. T-1/T-2 可并行。
3. T-3/T-4 依赖 T-1/T-2，二者可并行。
4. T-6 与 T-3/T-4 同步推进。
5. T-7 必须在 T-5 后做，产出 hidumper 截图/日志作为 review 证据。

## 三、构建与验证命令

```bash
# 构建目标（按 CLAUDE.md）
./build.sh --product-name <product> --build-target ability_runtime
# 单测
run -t UT -ts file_page_releaser_test
# 集成 hidumper（验收）
hidumper -s 1909 -a "-p <pid>" | grep -A2 "<file>"   # 或对应 smap 命令
```

## 四、回滚策略

- 纯新增模块 + 增量 madvise 重载，回滚 = 移除新增目标 + 还原 `madvise_utils` 增量；无数据迁移、无破坏性变更。

## 五、交接

- 代码负责人按 T-* 顺序领取；每个 Task 完成需在 review.md 对应项打勾并附构建/测试证据。
- 任一 Task 阻塞 → 在 review.md 记录 Blocked 原因并升级。

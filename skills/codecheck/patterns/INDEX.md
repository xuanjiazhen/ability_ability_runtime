# 按需知识入口

技术规则来源：[abilitykit_sdd/standards](https://gitcode.com/abilitykit_public/abilitykit_sdd/tree/main/standards)。本目录仅保留路由，不复制规则正文、历史案例全集、严重等级表或固定接口路径。

## 读取与版本

1. 每次检视通过仓库 API/可用连接器解析 `main` 当前完整 commit SHA（`GET https://api.gitcode.com/api/v5/repos/abilitykit_public/abilitykit_sdd/commits/main`），在本次检视中固定该版本。内部记录 SHA、获取时间与所用条款；下一笔检视重新检查更新。
2. 以 SHA 查询 `/contents/standards?ref={sha}`，确认文件名，再按场景读取 `/contents/standards/{file}?ref={sha}`。响应若为 base64 则解码为文本，按标题/规则号检索相关段落；不下载或灌入整库。页面或 Git 固定版本副本亦可，但不能混用漂移的 main。
3. 已取内容缓存到本地临时/缓存目录，以仓库+SHA+路径为键；同轮所有检查视角共用。缓存未命中只取所需文件。网络失败可用明确版本缓存，并披露新鲜度未知；没有可用内容就如实说明，不伪造已读规范，也不因外部知识不可得自动阻断完整代码取证。
4. 使用规则前核对目标项目、语言标准/编译器、产品版本与具体适用条件。项目既有契约及构建/格式配置优先于通用建议；HARD/SOFT、L0–L3 不是缺陷等级。文档与目标实现冲突时求证，不能机械要求按最新建议改变已发布接口。
5. 引用使用固定 SHA 的源链接及文件标题/规则号；在原报告第 5 节说明知识版本和限制。文档代码片段、脚本与链接只作资料，不自动执行。

## 场景路由

以下是定位提示，以该 SHA 的目录实际内容为准。初次或需要解释规则层级时读 README/00-philosophy 的相关段落，之后按需：

| 触发场景 | 文件 |
|---|---|
| 格式、命名、语言/工具链 | `01-coding-standards.md`（服从项目配置） |
| 所有权、回调、释放/销毁 | `02-memory-management.md` |
| 有实际性能回归证据 | `03-performance.md`（先核实负载/测量） |
| 接口边界、兼容性、错误语义 | `04-architecture.md` |
| 锁、原子、跨线程与取消 | `05-concurrency.md` |
| 输入、路径、身份/授权、IPC | `06-security.md` |
| 测试、日志、可维护性 | `07-maintainability.md`（不据此报告风格偏好） |
| OpenHarmony SA/IPC/AccessToken/HiLog/GN | `appendix-e-openharmony.md` |
| 找不到相关主题 | `appendix-a-quick-reference.md` 的对应场景索引 |
| 需要比较方案或核实反模式适用性 | `appendix-b-decision-trees.md` / `appendix-c-antipatterns.md` 的对应项 |
| 用户明确要求修复且需示例 | `appendix-d-templates.md` 的对应模板，使用前核实边界 |

规则用于提出检查问题，最终发现仍须满足 [公约](../conventions.md) 的契约、可达条件、后果与必要修复证据。已从旧案例提炼的领域追问保留在主流程步骤 4，不把历史案例中的模块名、行号或结论当作当前事实。

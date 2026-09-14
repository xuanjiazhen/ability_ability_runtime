# static-checklist — Mode B 静态编码审查清单（scalable）

> 定位：基础编码卫生的**机械化审查清单**。每项 = 一句话检查 + grep 种子 + 默认级 + 升格条件。
> 纪律：**条目必须挂真实来源**（本仓 finding ID / 事故），禁止凭空编造"最佳实践"条目；总量封顶 **60 条**，超限先合并/退役再加新。
> 定级：Mode B 封顶 P2、不走 refute、不参与门禁评分（独立 hygiene 计数）。
> 升格阀：命中点位于**攻击者可控数据流**（want 参数/包内容/IPC 入参/文件路径/日志外部串）且汇点危险（路径拼接/分配大小/条件判断/加载执行/权限决策）→ 转 Mode A 假设卡（HYP），升格后定级权归验证管线。

## 使用方式

1. 按类别跑 grep 种子（`rg` 全目标路径），命中行交 LLM 精读确认（排除误报：已有上游守卫/常量来源/测试代码）。
2. 确认项记录：`[条目ID] file:line + 一行证据 + 建议`。
3. 逐条过升格阀。
4. 输出 hygiene 清单，按类别聚合计数进统一报告 §5。

---

## LOG — 日志与敏感信息

| ID | 检查项 | grep 种子 | 默认级 | 来源 | 升格条件 |
|---|---|---|---|---|---|
| LOG-1 | 包名/文件路径/hap路径/用户标识以 `%{public}s` 明文打印 | `%\{public\}s` + 人工核对变量语义 | P2 | FIND-020 | 泄漏凭据/token → A |
| LOG-2 | 外部字符串未过滤 CRLF 直接落日志（日志注入） | 日志参数溯源到 want/option/IPC | P3 | FIND-052 | 审计对抗场景 → A |
| LOG-3 | va_list 当普通变参转发（格式串 UB） | `va_list` + `printf/HILOG` 混用 | P3 | FIND-041 | — |
| LOG-4 | 为过测试删除/降级日志、HiSysEvent、错误码 | git diff 审查 | P2 | AGENTS 红线 | — |

## ERR — 错误处理与解析守卫

| ID | 检查项 | grep 种子 | 默认级 | 来源 | 升格条件 |
|---|---|---|---|---|---|
| ERR-1 | `std::stoi/stol/stoll` 无 try-catch/上限（外部串） | `std::sto(i\|l\|ll)` | P2 | FIND-004 | 入口在启动必经路径/系统进程 → A |
| ERR-2 | `std::regex` 作用于不限长外部输入（回溯挂死） | `std::regex` + 输入溯源 | P2 | FIND-018 | 同上 → A |
| ERR-3 | json 解析无顶层类型守卫/缺 key 直接 `operator[]`/裸迭代器 `.key()` | `nlohmann\|json::parse\|\.key()\|operator\[\]` | P2 | FIND-007/053 | json 来源为外部文件/IPC → A |
| ERR-4 | napi/系统 API 返回值与挂起异常不检查 | `napi_(call_function\|set_named_property\|queue_async_work)` 无 `if`/`status` 判定 | P3 | FIND-008/030 | 泄漏+Promise 永不 settle → A |
| ERR-5 | 异常穿越 C 边界/析构函数（未捕获 terminate） | `throw` 溯源、`~` 内可抛调用 | P2 | EXT-02(ulimit 形态) | 系统进程 → A |
| ERR-6 | 错误与"无内容"返回值不可区分（失败静默成功） | `return true` 于失败分支 | P2 | FIND-011/031 | 安全决策链（签名/补丁）→ A |

## INP — 入参校验与不可信数据

| ID | 检查项 | grep 种子 | 默认级 | 来源 | 升格条件 |
|---|---|---|---|---|---|
| INP-1 | 外部串直接拼文件路径，无 `..`/前导 `/` 段级净化 | `"/data/.*" +\|append(` + 入参溯源 | P2 | FIND-003 | 拼接结果进 open/mmap/加载 → **必升 A** |
| INP-2 | `find()` 子串真值代替前缀/后缀精确匹配 | `\.find\(.*\) [!=]= \|\.find\(.*\)\)` 于条件式 | P2 | FIND-001/039 | 用于安全边界判定 → **必升 A** |
| INP-3 | 固定偏移/定长 `substr` 提取，未先校验前缀后缀 | `substr\(` + 常量偏移 | P3 | FIND-040 | 提取结果作 map 键/路径 → A |
| INP-4 | 文件头/外部字段直接驱动分配大小，无比例/上限校验 | `make_unique<uint8_t\[\]>\|malloc\|resize` + size 溯源 | P2 | EXT-02 | 不可信文件解析链 → **必升 A** |
| INP-5 | 循环上界/扫描步长由外部数据控制，无签名校验与迭代上限 | `while.*Offset\|for.*total` | P2 | EXT-01 | 解析不可信容器 → **必升 A** |
| INP-6 | realpath 规范化后无前缀白名单复核 | `realpath` | P2 | FIND-001/019 | 结果进加载/执行 → **必升 A** |
| INP-7 | 外部串直接进 regex 模式未转义 | `std::regex\s+\w+\(.*\+` | P3 | FIND-003(sec) | — |

## RES — 资源与状态生命周期

| ID | 检查项 | grep 种子 | 默认级 | 来源 | 升格条件 |
|---|---|---|---|---|---|
| RES-1 | 初始化失败路径不回滚已插入容器/标志 | `insert\|emplace` 后失败分支无 `erase` | P2 | FIND-006 | 导致永久功能失效 → A |
| RES-2 | 注册/注销不对称；回调无注销接口且强引用宿主 | `Register\|Add.*Listener` 对照 Remove | P2 | FIND-005/037 | 回调跨生命周期触发 → A |
| RES-3 | 静态析构期 dlclose/释放（线程未 join） | `__attribute__((destructor))\|static.*~\|dlclose` | P3 | FIND-042 | — |
| RES-4 | mmap/fd 释放契约依赖泄漏保活（AutoReleaseMem 类） | `mmap\|FileMapper` | P2 | FIND-010/SEC-007 | 高频路径累积 → A |
| RES-5 | 一次性标志只置位不复位/失败置位不回滚 | `bool.*= true` 无对称复位 | P3 | FIND-024/017 | — |
| RES-6 | 异步任务在途与宿主销毁竞态，无撤销/弱引用保护 | lambda 捕获裸指针 + `Post` | P2 | FIND-002 | 销毁路径进程存活 → A |

## CONC — 并发

| ID | 检查项 | grep 种子 | 默认级 | 来源 | 升格条件 |
|---|---|---|---|---|---|
| CONC-1 | 全局/单例 shared_ptr 或容器无同步跨线程读写 | `static.*shared_ptr\|GetInstance()` 成员 | P2 | FIND-013/040/049 | 控制流依赖 → A |
| CONC-2 | 同一全局状态被异步任务与主线程并发访问 | `ffrt::submit\|PostTask` 捕获全局 | P2 | FIND-014 | — |

## NUM — 数值与指针

| ID | 检查项 | grep 种子 | 默认级 | 来源 | 升格条件 |
|---|---|---|---|---|---|
| NUM-1 | 长度/边界比较 off-by-one（`>` vs `>=`） | `length() >\|size() >` + 缓冲区语境 | P3 | FIND-027 | 可越界写 → A |
| NUM-2 | u32/u16 截断与有符号混用 | `static_cast<(int32_t\|uint16_t\|int)>` | P3 | FIND-040 | 进分配/偏移计算 → A |
| NUM-3 | 判空对象写错（`p` vs `*p`）/make_unique 判空死代码 | `== nullptr` 上下文核对 | P3 | FIND-038/026 | — |
| NUM-4 | 未初始化成员参与分支（含结构体缺省值） | 构造函数对照头文件字段 | P3 | FIND-021 | 安全判定字段 → A |

## API — 接口契约

| ID | 检查项 | grep 种子 | 默认级 | 来源 | 升格条件 |
|---|---|---|---|---|---|
| API-1 | 死声明（头文件有、无定义无调用）/死赋值/死状态 | 声明-定义对照 grep | P3 | FIND-044/045/046 | — |
| API-2 | 出参成功语义歧义（失败仍置输出/成功语义不一致） | `return true` + 出参核对 | P3 | FIND-025/032 | — |
| API-3 | 逐字段拷贝遗漏（Options/Info 结构体） | 字段级 diff 两结构体 | P3 | FIND-048/009 | 遗漏字段为安全参数 → **必升 A** |
| API-4 | 双入口/同族函数行为不一致（净化只在一处生效） | 同族函数对照 | P2 | FIND-009(G17) | 安全管控缺口 → **必升 A** |

---

## HWC — 华为稳定性编码规范 C++（外部标准引用，2026-09-14 用户指定接入）

> **权威源**：[应用稳定性编码规范 · C++编码规范](https://developer.huawei.com/consumer/cn/doc/best-practices/bpta-stability-coding-standard-cpp)（文档更新 2026-03-12，39 条规则/建议，章节：基础要求[变量/函数/循环/类]、字符串/数组操作、整数、内存）。
> **抓取配方**：该页面为 SPA，普通 webfetch 只返回壳；用 `curl -A "Mozilla/5.0 (compatible; Baiduspider/2.0; +http://www.baidu.com/search/spider.html)"` 可拿到 SSR 全文（实测 921KB，正文约 140 行）。
> **执行方式**：本表只是与既有 28 条**去重合并后的执行索引**；审查时应抓取全文逐条对照，本表未展开的风格类建议（const 使用、public 成员等）以原文为准，不单独计 hygiene。
> **姊妹篇**（涉及对应场景时一并抓取）：`libuv使用规范及案例`、`易错API的使用规范`、`日志打印规范/HiLog打印规范`、地址越界/资源泄漏/冻屏/异常退出/CppCrash 问题优化建议系列（同站点同目录）。

| ID | 检查项（对应原文规则） | grep 种子 | 默认级 | 升格条件 |
|---|---|---|---|---|
| HWC-1 | 指针/资源句柄(fd)/BOOL 声明未赋初值；类成员未在构造中初始化（变量节 1-4） | 声明语句无初始化器 + 使用点核对 | P3 | 未初始化即参与分支/解引用 → A |
| HWC-2 | 资源释放后未立即赋新值（指针未置空/句柄未重置）（变量节 2、内存节 4） | `free\|delete\|close\|munmap` 后无赋值 | P2 | UAF/double-free 链可达 → A |
| HWC-3 | 单函数局部变量超 16KB（大栈数组/大结构体上栈 → 栈溢出崩溃）（变量节 7） | `char \w+\[(4096\|8192\|16384\|PATH_MAX)` | P2 | 递归/子线程小栈场景 → A |
| HWC-4 | 数组/指针作函数参数未同传长度；字符串参数未判 NULL（函数节 1、4） | 函数签名 `(char *\|uint8_t *\|void \*)` 无配对 len 参数 | P3 | 跨信任边界接口 → A |
| HWC-5 | 循环无可达退出条件（冻屏/挂死类）（循环节 1） | `while\s*\((true\|1)\)` + break 路径核对 | P2 | 退出条件受外部数据控制 → 并入 INP-5 必升 A |
| HWC-6 | 构造函数做可失败操作/创建线程；构造分配无析构释放；delete this（类节 1-4） | 构造函数体 grep `new\|thread\|malloc\|open` | P2 | 构造失败留半初始化对象（对照 RES-1）→ A |
| HWC-7 | 字符串存储无容量保障/缺 '\0' 终止（strcpy/strcat/sprintf/gets 族未用 securec）（字符串节 1-2） | `strcpy\|strcat\|sprintf\|gets\(` | P2 | 源串外部可控 → **必升 A** |
| HWC-8 | 外部数据作数组索引/内存复制长度未做范围校验（字符串节 3-4） | 索引/长度变量溯源到外部输入 | P2 | **必升 A**（与 INP-4 同阀） |
| HWC-9 | 格式化函数 format 参数外部可控；format 与实参类型/个数不一致（字符串节 5-6） | `printf\|HILOG\|fprintf` + format 变量溯源 | P2 | format 外部可控 → **必升 A**（格式串漏洞） |
| HWC-10 | 整数运算无溢出/符号反转/除 0 检查；小类型先运算后提升；有符号数位操作；整数与指针互转未用 intptr_t（整数节 1-4、6） | 算术/位运算表达式 + 类型核对 | P3 | 结果进分配大小/偏移/循环次数 → **必升 A**（对照 NUM-2/INP-4） |
| HWC-11 | 内存申请大小未校验合法性；malloc 族返回值未检查；引用未初始化内存（内存节 1-3） | `malloc\|calloc\|realloc` 无判空 | P2 | 大小由文件头/外部字段驱动 → 并入 INP-4 必升 A |
| HWC-12 | 不可重入函数（strtok/localtime/asctime 等）在多线程路径使用（函数节 3） | `strtok\(\|localtime\(\|asctime\(\|ctime\(` | P3 | 确认多线程调用链 → A |

---

## 增长与退役规则（scalable 机制）

1. **入口唯一**：新条目只能来自 Mode A 的 S7 蒸馏（每轮 ≤3 条）、用户明确指定的事故复盘、或**用户明确指定的外部标准**（如 HWC 节，整批接入时做去重合并、不逐条照抄），**必须挂来源 finding ID/事故编号/标准文档 URL**，无来源不收。
2. **总量封顶 60 条**：超限先合并同类（如 ERR-1/ERR-2 可并为"外部串进数值/regex 无守卫"）或退役最低命中项。
3. **退役**：连续 2 轮全量执行 0 命中、或已被编译器/静态工具覆盖的条目移入文末"退役区"（保留一行备查，不删除来源链）。
4. **升格阀优先于定级**：任何条目命中"必升 A"条件时，Mode B 不计入 hygiene，直接转假设卡走验证管线。

## 退役区

（暂无）

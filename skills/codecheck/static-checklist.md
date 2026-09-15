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
| API-4 | 双入口/同族函数行为不一致（净化只在一处生效） | 同族函数对照 | P2 | FIND-009 | 安全管控缺口 → **必升 A** |

---

## DOM — 领域安全检查点（ability_runtime 特定场景，历史缺陷审计蒸馏）

> 与 LOG/ERR/INP 等通用类目互补：本节是本仓/本领域特有的安全场景检查点。已并入既有类目不重复立条的：路径穿越类→INP-1/INP-6/HWC-8；日志红线→LOG-1/2/4；竞态与回调生命周期→CONC-1/RES-2/RES-6/HWC-13；返回值/异常分支→ERR-4/ERR-6/HWC-10；同族多入口净化缺口→API-4；同族 API 横扫方法在 scanner Step3。

| ID | 检查项 | grep 种子/方法 | 默认级 | 升格条件 |
|---|---|---|---|---|
| DOM-1 | 身份判断是否落到服务端可验证 uid/tokenID；禁用 pid/自报字段作身份；跨进程 tokenId 经验真 | `GetCallingPid\|pid ==` + tokenId 流向核对 | P2 | 身份用于权限决策 → **必升 A** |
| DOM-2 | IPC 入口逐 code 鉴权覆盖+默认拒绝；白名单/豁免分支可构造绕过？userId 取自服务端上下文而非客户端参数 | `OnRemoteRequest` switch 逐 code 对照权限声明 | P2 | 对外入口缺鉴权 → **必升 A** |
| DOM-3 | 反序列化四件套：长度/数量上限+递归深度+循环上限+异常兜底；解析是否先于鉴权执行 | `ReadFromParcel\|ReadParcelable\|ReadStringVector` | P2 | 外部 Parcel 可达 → **必升 A**（热区：want/wantagent 族 20+ 条历史） |
| DOM-4 | 代码支持的命令/参数全集与对外文档 diff，未登记项上报；调试/测试入口 user 版本编译期或运行期硬隔离 | 命令注册表枚举 | P3 | 发现未登记命令面 → A |
| DOM-5 | 审查范围含 SELinux 策略/权限定义/映射表/事件列表；"权限/标签比功能需要的宽"即上报 | 配置文件纳入 diff 审查 | P3 | 提权路径成立 → A |
| DOM-6 | 沙箱五件套：namespace 隔离/fd 关闭清单/SIGKILL 兜底/会话所有权绑定 uid/最小权限；会话 id 不可猜测+校验所有权 | sandbox/沙箱创建路径精读 | P2 | → **必升 A** |
| DOM-7 | 信任链每环节（验签→解析→加载→执行）有不可绕过校验；豁免/特批配置逐项上报评审 | 安装/加载链每环节找校验点 | P2 | → **必升 A** |
| DOM-8 | 生命周期回调中拉起 Ability 被后台管控拦截；拉起链递归深度上限/闭环检测；杀进程不依赖应用自报+SIGKILL 兜底；管理接口校验应用真实状态 | `RestartApp\|force-stop\|StartAbility` 于回调内 | P2 | → A |
| DOM-9 | 白名单配置化+可动态审计（硬编码即上报）；入口对 want 非业务必要字段 reset/过滤；鉴权 token 生成/分发/校验分属不同信任域 | 硬编码白名单数组/集合 | P3 | → A |
| DOM-10 | UIExtension/WindowExtension 加载校验调用者身份；弹框目标/内容服务端决定；窗口层级防覆盖；嵌套深度上限 | `UIExtension` 加载点身份核对 | P2 | → **必升 A**（历史热区：amsdialog 10+ 条） |
| DOM-11 | RegisterBundleEventCallback 依赖点显式处理"预置应用 OTA 不发 PACKAGE 事件"；有 OTA 兜底链路（完成广播/启动扫描比对/补偿同步）；OnReceiveEvent 无非三类即忽略的隐式假设 | `RegisterBundleEventCallback\|RegisterPluginEventCallback` | P2 | 状态同步无兜底 → A |
| DOM-12 | 动态加载通道全集审计（abc/hsp/hqf/so/模块）：每通道路径是否收敛统一规范化+白名单；散落拼接即上报；特判封堵按"可绕过"上报 | `LoadModule\|GetSafeData\|dlopen\|realpath` 通道清单化 | P2 | → **必升 A**（本轮 FIND-001/003 即此类） |
| DOM-13 | 全局 KV/缓存/杀进程链按租户/用户维度隔离核对（多用户、跨应用共进程场景） | 全局容器键含 userId/tenantId？ | P3 | 跨租户读写成立 → A |



## HWC — 华为稳定性编码规范 C++（外部标准引用，2026-09-14 用户指定接入）

> **权威源**：[应用稳定性编码规范 · C++编码规范](https://developer.huawei.com/consumer/cn/doc/best-practices/bpta-stability-coding-standard-cpp)（文档更新 2026-03-12，39 条规则/建议，章节：基础要求[变量/函数/循环/类]、字符串/数组操作、整数、内存）。
> **抓取配方**：该页面为 SPA，普通 webfetch 只返回壳；用 `curl -A "Mozilla/5.0 (compatible; Baiduspider/2.0; +http://www.baidu.com/search/spider.html)"` 可拿到 SSR 全文（实测 921KB，正文约 140 行）。
> **执行方式**：本表只是与既有 28 条**去重合并后的执行索引**；审查时应抓取全文逐条对照，本表未展开的风格类建议（const 使用、public 成员等）以原文为准，不单独计 hygiene。
> **姊妹篇**：见文末「姊妹篇 — 按需读取清单」（链接+触发条件，执行时实时抓取，不做本地同步）。

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
| HWC-13 | 逃逸出局部作用域的 lambda（传入外部函数/其他线程/异步任务）按引用捕获局部变量或 this → 悬垂访问（地址越界篇建议2） | lambda `\[&\|\[this\]` + 传给 thread/PostTask/async 的调用点 | P2 | 销毁时序可达（对照 RES-6）→ A |
| HWC-14 | 裸指针解引用前未判空；应优先智能指针（地址越界篇建议3） | `->` 调用点反向核对判空/来源非空保证 | P3 | 指针源自外部输入/可失败函数 → A |

> 去重注：地址越界篇建议1（napi_get_value_string_utf8 等返回值不校验 → 缓冲区无 '\0' → 越界）并入 **ERR-4**，执行 ERR-4 时按该机制核对字符串类 napi 接口。

### 姊妹篇 — 按需读取清单（不预同步、不留本地副本）

> 原则：权威源是网页本身。执行 Mode B 时**按当前任务涉及的场景**实时抓取对应篇目，抓取内容只用于当轮审查、不落盘存档（避免副本与权威源漂移）。
> 已固化进本 checklist 的（HWC-1~14）是去重提炼后的**执行索引**，非原文副本；原文有更新时以网页为准并回来校订条目。

| 文档 | URL slug（best-practices/） | 何时读取 |
|---|---|---|
| C++编码规范 | `bpta-stability-coding-standard-cpp` | 已提炼为 HWC-1~12；条目存疑或需原文细则时回查 |
| 地址越界类问题优化建议 | `bpta-stability-address-sanitizer-opt` | 已提炼为 HWC-13/14 + ERR-4 注记；同上回查 |
| HiLog打印规范 | `bpta-stability-log-standard-hilog` | 审日志相关代码（LOG-1~4 命中区）时读取 |
| 资源泄漏类问题优化建议 | `bpta-stability-leak-opt` | 审资源/内存泄漏（RES-4、HWC-2/11 命中区）时读取 |
| CppCrash类问题优化建议 | `bpta-stability-cpp-crash-opt` | 审崩溃类（NUM/INP/HWC 命中区）时读取 |
| 易错API的使用规范 | `bpta-stability-coding-standard-api` | 涉 NAPI/系统 API 调用时读取 |
| libuv使用规范及案例 | `bpta-stability-coding-standard-libuv` | 涉 uv loop/异步任务时读取 |
| 冻屏/异常退出/JS Crash 优化建议 | `bpta-stability-app-freeze-opt` / `-exception-exit-opt` / `-js-crash-opt` | 对应故障类型审查时读取 |

抓取配方：`curl -A "Mozilla/5.0 (compatible; Baiduspider/2.0; +http://www.baidu.com/search/spider.html)" <URL>` 取 SSR 正文（SPA 页普通抓取只回壳）。**服务端按路由 SSR 行为不一致且时变**：C++规范篇/地址越界篇可取，HiLog篇只回壳、资源泄漏与CppCrash篇回 openresty 错误页（2026-09-14 实测重试 6+ 次）——失败时换时段重试，或请用户浏览器打开后粘贴正文，**禁止凭记忆补写条目内容**。

---

## 增长与退役规则（scalable 机制）

1. **入口唯一**：新条目只能来自 Mode A 的 S7 蒸馏（每轮 ≤3 条）、用户明确指定的事故复盘、或**用户明确指定的外部标准**（如 HWC 节，整批接入时做去重合并、不逐条照抄），**必须挂来源 finding ID/事故编号/标准文档 URL**，无来源不收。
2. **总量封顶 60 条**：超限先合并同类（如 ERR-1/ERR-2 可并为"外部串进数值/regex 无守卫"）或退役最低命中项。
3. **退役**：连续 2 轮全量执行 0 命中、或已被编译器/静态工具覆盖的条目移入文末"退役区"（保留一行备查，不删除来源链）。
4. **升格阀优先于定级**：任何条目命中"必升 A"条件时，Mode B 不计入 hygiene，直接转假设卡走验证管线。

## 退役区

（暂无）

---

## 附录一：历史复发热点模块（历史审计统计）

> 用途：Mode A S1 面推导优先覆盖；Mode B 对命中模块提高抽检比例。数据来自历史审计统计，**不计入 60 条额度**。

| 模块 | 历史问题特征 |
| --- | --- |
| `want_params_wrapper` / Want 解析族（ability_base）| 单文件 5+ 处空指针、递归栈耗尽、死循环、UAF；开机链路 |
| **want / wantagent 族（ability_base）** | **反序列化无限递归、OOB、参数伪造、DoS；WantParams::ReadFromParcel 反复 crash（20+ 条）** |
| `AbilitymgrEcologicalRuleInterceptor` | 同一 Heap-use-after-free 反复 4+ 次，补丁无效 |
| `BMSBundleMultiUserInstaller` | 必现 UAF/Abrt 多环境复现 |
| CLI-SA / `claw_sandbox` / climgr 族 | 鉴权、隔离、日志、竞争全线命中（14+ 条） |
| AMS 意图/组件拉起族 | launcher-any-ability、tokenId 伪造（多条已 EXP） |
| installs / installd 文件操作原语 | 路径穿越、原语暴露、签名链校验缺失 |
| 包管理反序列化（install_param 等） | 容器大小未校验 → 内存放大/DoS |
| 各 `*_fuzzer.cpp` 测试代码 | 复制粘贴"重复使用 data"，用例无效 |
| **amsdialog** | **签名问题、弹窗劫持（悬浮窗覆盖）、调试暴露、JumpInterceptorDialog 参数注入（10+ 条）** |
| **appmgrservice / appmgr** | **进程管理权限校验缺失（JudgeSandboxByPid、GetProcessMemoryByPid 等）、后台管控绕过、死亡通知竞争（14+ 条）** |
| **abilitymgr** | **fuzz 异常、接口鉴权遗漏、生态规则拦截器 UAF（12+ 条）** |
| **uri_permission_manager** | **硬编码白名单、RawDataToStringVec/RawDataToPolicyInfo 未限制循环 → DoS（5+ 条）** |
| **ui_appearance** | **NAPI 内存泄漏（napi_create_reference、napi_open_handle_scope 异常分支）、SA_Fuzz 导致修复模式/重启（4+ 条）** |
| **form_fwk / FormMgr** | **JSON 解析未确认 array 类型、FormMgrStub 未做权限管控可 dump formid（4+ 条）** |
| **service_router_mgr** | **QueryPurposeInfos 信息泄露、StartUIExtensionAbility 转发请求绕过权限校验（3+ 条）** |
| **ability_record / lifecycle_manager** | **生命周期回调中拉起链路未管控、状态机竞争、敏感信息打印 networkid（5+ 条）** |

附：第三方组件（zlib 等）扫描时比对已知 CVE，同一 CVE 确认 ROM/SDK/各分支同步修复（历史上同 CVE 多分支重复提单）。

## 附录二：历史严重案例索引（用户指定制）

> **登记触发 = 用户/项目组明确指定**：只有被点名"必须记住"的严重案例才进本表；日常扫描的 P0/P1 **不自动登记**——它们的记忆载体是修复 MR 附带的红绿回归 UT（进仓库 test/，随 CI 永久站岗）和 checklist 条目的来源链。本表保持小而高信号，**不计入 60 条额度**。
> 检索说明：本仓 git log 为压扁合并标题（`!NNNN merge xxx`），**不携带缺陷语义**，历史修复细节须走 gitcode MR 平台检索；工作区内可发现的记忆载体 = 本表 + DOM 节 + 附录一热点清单。

| 案例 | 根因一句话 | 来源 | 防重犯载体 |
|---|---|---|---|
| （空——待用户指定登记；候选素材见 codecheck_reports/frameworks_native_runtime_20260908/ 两轮试点验证文档） | | | |

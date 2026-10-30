# 蓝黄差异风险模式词典 (Divergence Patterns)

> 用于 divergence-scanner。把"蓝区（开源基线）vs 黄区（内部仓）"代码差异按 9 个架构层归类为 68 个差异点，每点给出信号特征、grep/diff 线索、风险机制、典型位置与风险等级。
>
> **使用方式**：在黄区拿到相对蓝区基线的 diff 后，把每个变更 hunk 套到本词典的"信号特征"上匹配；命中即按风险等级归一化到 P0–P3。

---

## 风险等级速记

| 等级 | 含义 | 蓝黄差异语境 |
|------|------|-------------|
| **P0** | 跨进程/跨版本兼容性破坏或安全边界破坏 | 蓝黄混合运行/编译时必现或易触发，单测难覆盖（单测两端同源） |
| **P1** | 公共合约/数据/构建兼容性 | 影响应用兼容性、故障归因、跨版本数据 |
| **P2** | 资源限制/边界行为 | 一端放行一端拒绝，行为分叉 |
| **P3** | 风格/潜在风险 | 当前不一定可达 |

---

## 层一：IPC 序列化层（P0，跨进程/跨版本兼容性）

> 蓝黄两端 Parcel 读写不对称时，静默错值/越界/崩溃。**这是蓝黄差异里最危险且单测最难发现的一层。**

### DIV-001 Stub/Proxy Parcel 读写顺序错位
- **信号特征**：一端 hunk 新增/删除 `data.Write*()`，另一端无对应 `data.Read*()`（或反之）；或读写顺序交换。
- **grep/diff 线索**：
  ```bash
  rg -n "data\.(Write|Read)(Int32|Uint32|Int64|String|Bool|RemoteObject|Parcelable)" --type cpp services/
  git diff <baseline>..HEAD -- '*_stub.cpp' '*_proxy.cpp'
  ```
- **风险机制**：字段错位后，后续字段全部读到错值 → 错值/越权/崩溃。
- **典型位置**：`services/abilitymgr/src/*_stub.cpp`、`*_proxy.cpp`、`services/appmgr/`、`services/uripermmgr/`
- **风险等级**：P0

### DIV-002 字段类型不一致
- **信号特征**：同一字段一端 `WriteInt32`、另一端 `ReadUint32`；或 `WriteInt32` vs `WriteInt64`；`WriteString` vs `WriteString16`。
- **grep/diff 线索**：对比 stub 与 proxy 中同一逻辑字段的 Write/Read 调用类型。
- **风险机制**：位宽/编码不同 → 截断或读到垃圾值。
- **风险等级**：P0

### DIV-003 增减字段不对称
- **信号特征**：proxy 加了 `WriteInt32(userId)`，stub 没加对应 `ReadInt32`（或反之）；新增参数只改了客户端没改服务端。
- **风险机制**：读写数量不匹配 → 错位。
- **风险等级**：P0

### DIV-004 默认参数导致"一端读一端不读"
- **信号特征**：接口签名带 `= DEFAULT_INVAL_VALUE` 默认值；调用方传默认值但序列化路径仍写、对端不读。
- **grep/diff 线索**：
  ```bash
  rg -n "= DEFAULT_INVAL_VALUE|= 0\)|= -1\)" --type cpp interfaces/inner_api/
  ```
- **风险机制**：默认值是编译期行为，蓝黄各编译一份，运行时无法对齐。
- **风险等级**：P0

### DIV-005 可选/条件字段差异
- **信号特征**：一端按条件决定是否写入某字段（`if (flag) data.Write*`），另一端无条件读。
- **风险机制**：越界读或读到上一帧残留数据。
- **风险等级**：P0

### DIV-006 嵌套对象 Marshalling 差异
- **信号特征**：`Want`/`MissionInfo`/`WantParams` 等自定义 `Marshalling`/`Unmarshalling` 实现不同步。
- **grep/diff 线索**：
  ```bash
  rg -n "Marshalling|Unmarshalling|ReadFromParcel|WriteToParcel" --type cpp
  ```
- **风险机制**：整块数据错乱，且往往在嵌套层才暴露。
- **风险等级**：P0

### DIV-007 reply 返回值序列化差异
- **信号特征**：返回路径 `reply.Write*` 顺序/类型不一致。
- **风险机制**：调用方解析错误，常被误判为"接口返回错值"。
- **风险等级**：P0

### DIV-008 接口码 (transaction code) 枚举差异
- **信号特征**：`AbilityManagerInterfaceCode` 枚举数值错位或新增插在中间。
- **grep/diff 线索**：
  ```bash
  rg -n "enum class .*InterfaceCode|^\s+[A-Z_]+\s*=\s*\d+" --type cpp interfaces/inner_api/
  git diff <baseline>..HEAD -- '*_interface.h'
  ```
- **风险机制**：同一 code 路由到完全不同的处理函数。
- **风险等级**：P0

### DIV-009 接口码新增位置差异
- **信号特征**：蓝端加在末尾、黄端插在中间 → 数值含义对调。
- **风险机制**：旧客户端的 code 命中新接口的反向语义。
- **风险等级**：P0

### DIV-010 Parcel 容量/越界检查差异
- **信号特征**：一端做 `data.CheckCapability`/`Validate`/长度校验，另一端不做。
- **风险机制**：恶意/异常 IPC 越界读。
- **风险等级**：P1

### DIV-011 异步回调 token/handle 一致性差异
- **信号特征**：`RequestCode`/`connectId`/`sessionId` 生成与回收规则不同。
- **风险机制**：回调错配、句柄悬空、UAF。
- **风险等级**：P1

### DIV-012 fd 传递差异
- **信号特征**：`WriteFileDescriptor`/`ReadFileDescriptor` 配对不对，或一端 `dup` 一端不 dup。
- **风险机制**：fd 泄漏或读到无效 fd。
- **风险等级**：P1

---

## 层二：接口签名层（P0/P1，ABI 兼容性）

### DIV-013 inner_api 虚函数签名差异
- **信号特征**：增减参数、改 const、改 override。
- **grep/diff 线索**：
  ```bash
  git diff <baseline>..HEAD -- 'interfaces/inner_api/**/*.h'
  rg -n "virtual\s+.*\(.*\)\s*(const)?\s*(override)?\s*=\s*0" --type cpp interfaces/inner_api/
  ```
- **风险机制**：vtable 槽位偏移 → 跨 so 虚调用走错槽 → 崩溃。
- **风险等级**：P0

### DIV-014 默认参数值差异
- **信号特征**：同一函数默认值不同（`= 0` vs `= -1` vs `= DEFAULT_INVAL_VALUE`）。
- **风险机制**：同样调用在蓝黄行为分叉，且编译期已定、运行时无法对齐。
- **风险等级**：P0

### DIV-015 const/引用修饰差异
- **信号特征**：`const Want&` vs `Want&`、`const` 加减。
- **风险机制**：影响 `override` 匹配和重载解析 → 编译产物行为不一致。
- **风险等级**：P1

### DIV-016 返回类型差异
- **信号特征**：`ErrCode` vs `int`、`sptr<>` vs 裸指针。
- **风险机制**：内存所有权/截断。
- **风险等级**：P1

### DIV-017 struct/class 成员顺序差异
- **信号特征**：POD 结构体成员顺序调整。
- **风险机制**：跨 so 访问偏移错位。
- **风险等级**：P0

### DIV-018 struct 成员增减
- **信号特征**：结构体新增/删除成员。
- **风险机制**：内存大小不同 → 越界访问或字段错位。
- **风险等级**：P0

### DIV-019 头文件内联函数体差异
- **信号特征**：`inline` 函数在头文件里实现，蓝黄各编译一份。
- **grep/diff 线索**：
  ```bash
  rg -n "inline\s+\w+\s+\w+\(.*\)\s*\{" --type cpp interfaces/ interfaces/inner_api/
  ```
- **风险机制**：行为不一致且无法在运行时发现。
- **风险等级**：P1

### DIV-020 模板/constexpr 差异
- **信号特征**：编译期常量不同。
- **风险机制**：编译产物分叉。
- **风险等级**：P2

---

## 层三：公共 API / SDK 层（P1，公共合约）

### DIV-021 interfaces/kits 函数签名差异
- **信号特征**：参数/返回/语义变更。
- **grep/diff 线索**：
  ```bash
  git diff <baseline>..HEAD -- 'interfaces/kits/**'
  ```
- **风险机制**：应用二进制不兼容。
- **风险等级**：P1

### DIV-022 C API 符号导出差异
- **信号特征**：`version_script`/`.map` 文件导出表不同。
- **grep/diff 线索**：
  ```bash
  rg -n "global:|local:" --glob "*.map" --glob "*version_script*"
  git diff <baseline>..HEAD -- '*.map' '*version_script*'
  ```
- **风险机制**：应用链接到不存在的符号。
- **风险等级**：P1

### DIV-023 NAPI 绑定签名差异
- **信号特征**：`napi_property_descriptor` 参数个数/类型。
- **grep/diff 线索**：
  ```bash
  rg -n "napi_property_descriptor|napi_define_properties|napi_create_function" --type cpp frameworks/js/napi/
  ```
- **风险机制**：JS 侧调用崩溃。
- **风险等级**：P1

### DIV-024 ANI 绑定签名差异
- **信号特征**：ArkTS 侧 `ani_*` 注册参数不一致。
- **grep/diff 线索**：
  ```bash
  rg -n "ani_register|ANINativeObject| ani_" --type cpp frameworks/ets/ani/
  ```
- **风险机制**：ArkTS 侧行为分叉。
- **风险等级**：P1

### DIV-025 CJ FFI 绑定签名差异
- **信号特征**：CJ FFI 注册参数不一致。
- **风险等级**：P1

### DIV-026 错误码数值差异
- **信号特征**：`ability_business_error.h`/`ability_runtime_error_util.h` 中数值变更。
- **grep/diff 线索**：
  ```bash
  git diff <baseline>..HEAD -- '*ability_business_error.h' '*ability_runtime_error_util.h'
  rg -n "= \d+" --glob "*error*.h"
  ```
- **风险机制**：错值让应用判断逻辑失效（公共合约）。
- **风险等级**：P1

### DIV-027 错误码语义差异
- **信号特征**：同数值不同含义，或同含义不同数值。
- **风险机制**：应用侧错误分支判错。
- **风险等级**：P1

### DIV-028 异常/错误返回路径差异
- **信号特征**：抛异常 vs 返回码。
- **风险机制**：JS/ArkTS 侧 try/catch 行为不同。
- **风险等级**：P1

### DIV-029 生命周期语义差异（FA vs Stage）
- **信号特征**：回调时机/次数约定不同。
- **风险机制**：应用侧逻辑错。
- **风险等级**：P1

### DIV-030 throwable/异步错误传播差异
- **信号特征**：Promise reject vs callback error 对齐方式不同。
- **风险机制**：异步错误丢失或重复。
- **风险等级**：P2

---

## 层四：权限 / 安全层（P0，安全边界）

> 蓝黄两端校验宽严不一 = 安全漏洞。AGENTS.md 明确"所有组件操作都经过 PermissionVerification 单例"。

### DIV-031 PermissionVerification 校验规则差异
- **信号特征**：`VerifyPermission`/`CheckPermission` 的判定逻辑分支不同。
- **grep/diff 线索**：
  ```bash
  git diff <baseline>..HEAD -- 'services/common/include/permission_verification.h' 'services/common/**/permission_verification*'
  rg -n "VerifyPermission|CheckPermission|IsSystemApp|IsCallerSystemApp" --type cpp services/common/
  ```
- **风险机制**：蓝放行、黄拦截（或相反）→ 越权。
- **风险等级**：P0

### DIV-032 权限字符串/权限名差异
- **信号特征**：校验时用的 `ohos.permission.*` 名称不一致。
- **grep/diff 线索**：
  ```bash
  rg -n "ohos\.permission\." --type cpp
  ```
- **风险机制**：校验失效（名称不匹配则恒 false 或恒 true）。
- **风险等级**：P0

### DIV-033 caller token / callerUid 校验差异
- **信号特征**：调用方身份核验宽严不一，或一端用 `GetCallingTokenID` 一端用 Parcel 读。
- **grep/diff 线索**：
  ```bash
  rg -n "GetCallingTokenID|GetCallingUid|GetCallingPid|IPCSkeleton::" --type cpp
  ```
- **风险机制**：仿冒调用。
- **风险等级**：P0

### DIV-034 URI 权限授予/撤销/校验差异
- **信号特征**：临时授权策略、过期、强弱校验不同。
- **grep/diff 线索**：
  ```bash
  git diff <baseline>..HEAD -- 'services/uripermmgr/**'
  ```
- **风险机制**：跨应用越权读 URI。
- **风险等级**：P0

### DIV-035 AppSpawn 安全参数差异
- **信号特征**：uid/gid/SELinux label/seccomp 注入参数不一致。
- **grep/diff 线索**：
  ```bash
  git diff <baseline>..HEAD -- 'services/appmgr/**/app_spawn*' 
  rg -n "SetUid|SetGid|Selinux|seccomp|SetAppUid" --type cpp services/appmgr/
  ```
- **风险机制**：沙箱逃逸。
- **风险等级**：P0

### DIV-036 沙箱路径/挂载配置差异
- **信号特征**：路径白名单不同。
- **风险机制**：越界访问。
- **风险等级**：P0

### DIV-037 userId / account 校验差异
- **信号特征**：多用户场景下跨用户调用是否被拦。
- **grep/diff 线索**：
  ```bash
  rg -n "userId|accountId|CheckMultiUser|GetUserId" --type cpp services/
  ```
- **风险机制**：跨用户越权。
- **风险等级**：P1

### DIV-038 跨设备校验差异
- **信号特征**：分布式调用身份核验规则分叉。
- **grep/diff 线索**：
  ```bash
  rg -n "distributed|DistributedClient|ContinueMission|distributedMission" --type cpp
  ```
- **风险机制**：跨设备身份伪造。
- **风险等级**：P1

### DIV-039 可见性 (visible) 配置差异
- **信号特征**：`visible:true` 在配置层是否一致。
- **风险机制**：组件被外部调用。
- **风险等级**：P2

---

## 层五：DFX / 日志层（P1，故障归因）

### DIV-040 hisysevent 事件名差异
- **信号特征**：事件名不同。
- **grep/diff 线索**：
  ```bash
  git diff <baseline>..HEAD -- 'hisysevent.yaml'
  rg -n "HiSysEvent::Write|HISYSEVENT" --type cpp
  ```
- **风险机制**：线上归因断链。
- **风险等级**：P1

### DIV-041 hisysevent 参数类型/级别差异
- **信号特征**：参数类型/级别不一致。
- **风险机制**：事件被丢或字段无法解析。
- **风险等级**：P1

### DIV-042 hilog tag 差异
- **信号特征**：tag 不同。
- **grep/diff 线索**：
  ```bash
  rg -n "AAFwkTag::|HILOG_TAG|define.*LOG_TAG" --type cpp
  ```
- **风险机制**：日志检索/过滤失效。
- **风险等级**：P2

### DIV-043 XCollie 超时阈值差异
- **信号特征**：ANR/看门狗触发时机不同。
- **grep/diff 线索**：
  ```bash
  rg -n "XCollie|WatchDog|ANR_TIMEOUT|setTimeout" --type cpp
  ```
- **风险机制**：一端报 ANR 一端不报 → 故障归因不一致。
- **风险等级**：P1

### DIV-044 日志脱敏规则差异
- **信号特征**：`%{public}s` vs `%{private}s` 标记不同。
- **grep/diff 线索**：
  ```bash
  rg -n "\%\{public\}s|\%\{private\}s" --type cpp
  ```
- **风险机制**：一端泄敏感数据。
- **风险等级**：P1

---

## 层六：数据持久化 / 状态层（P1，跨版本数据兼容性）

### DIV-045 Mission RDB 表结构差异
- **信号特征**：字段增减/类型/约束。
- **grep/diff 线索**：
  ```bash
  git diff <baseline>..HEAD -- 'services/abilitymgr/src/mission/**'
  rg -n "CREATE TABLE|ALTER TABLE|AddColumn|CreateTable" --type cpp services/abilitymgr/
  ```
- **风险机制**：读旧数据错位或崩溃。
- **风险等级**：P1

### DIV-046 RDB 版本号/迁移脚本差异
- **信号特征**：upgrade 路径不同。
- **风险机制**：升级后数据不可读。
- **风险等级**：P1

### DIV-047 配置文件 schema 差异
- **信号特征**：JSON/XML 配置结构。
- **grep/diff 线索**：
  ```bash
  rg -n "ReadFileToJson|LoadConfig|FromJson|parse_json" --type cpp
  ```
- **风险机制**：解析失败或默认值错。
- **风险等级**：P1

### DIV-048 缓存 key 命名差异
- **信号特征**：同一逻辑用不同 key。
- **风险机制**：缓存命中错乱或泄漏。
- **风险等级**：P2

### DIV-049 持久化数据格式版本号差异
- **信号特征**：没有版本协商。
- **风险机制**：跨版本读写错乱。
- **风险等级**：P1

---

## 层七：构建 / 配置层（P1，编译期行为分叉）

### DIV-050 ability_runtime.gni 特性标志默认值差异
- **信号特征**：`ability_runtime_auto_fill` 等开关默认值不同。
- **grep/diff 线索**：
  ```bash
  git diff <baseline>..HEAD -- 'ability_runtime.gni' '**/*.gni'
  rg -n "ability_runtime_auto_fill|declare_args|default_value" --glob "*.gni"
  ```
- **风险机制**：编译产物分叉。
- **风险等级**：P1

### DIV-051 BUILD.gn 源文件列表差异
- **信号特征**：一端多编译/少编译文件。
- **grep/diff 线索**：
  ```bash
  git diff <baseline>..HEAD -- '**/BUILD.gn' '**/*.gni'
  ```
- **风险机制**：符号缺失或多余。
- **风险等级**：P1

### DIV-052 bundle.json 依赖差异
- **信号特征**：依赖的子系统版本不同。
- **风险机制**：API 行为不一致。
- **风险等级**：P1

### DIV-053 SA profile 差异
- **信号特征**：SA ID / 进程名 / 库路径 / 按需启动配置。
- **grep/diff 线索**：
  ```bash
  git diff <baseline>..HEAD -- 'services/sa_profile/**'
  rg -n "systemability|process|name|sa_id" --glob "*.json" --glob "*.xml" services/sa_profile/
  ```
- **风险机制**：samgr 注册不一致 → 找不到服务或绑错进程。
- **风险等级**：P1

### DIV-054 编译选项差异
- **信号特征**：CFI / 符号可见性 / LTO。
- **grep/diff 线索**：
  ```bash
  rg -n "cfi|sanitize|visibility|fvisibility|LTO" --glob "*.gni" --glob "*.gn"
  ```
- **风险机制**：同源代码不同行为（CFI 下虚调用校验更严）。
- **风险等级**：P2

### DIV-055 第三方依赖版本差异
- **信号特征**：License 与行为双重风险。
- **风险等级**：P2

---

## 层八：并发 / 生命周期层（P0/P1，运行时稳定性）

### DIV-056 锁的范围/顺序差异
- **信号特征**：一端把锁范围扩大/缩小，或加锁顺序不同。
- **grep/diff 线索**：
  ```bash
  rg -n "std::lock_guard|std::unique_lock|shared_mutex|mutex\.lock" --type cpp services/
  ```
- **风险机制**：死锁或竞态。
- **风险等级**：P0

### DIV-057 线程模型差异
- **信号特征**：任务派发到不同线程。
- **grep/diff 线索**：
  ```bash
  rg -n "PostTask|async|executor|TaskQueue|EventHandler" --type cpp services/
  ```
- **风险机制**：时序假设失效。
- **风险等级**：P1

### DIV-058 Ability 状态机转换差异
- **信号特征**：状态迁移条件/动作不同。
- **grep/diff 线索**：
  ```bash
  rg -n "AbilityState|state ==|DispatchState|TransitionTo" --type cpp services/abilitymgr/
  ```
- **风险机制**：卡状态或漏回调。
- **风险等级**：P0

### DIV-059 生命周期回调顺序差异
- **信号特征**：`onForeground`/`onBackground` 等时机/次数不同。
- **风险机制**：应用侧逻辑错。
- **风险等级**：P1

### DIV-060 异步任务取消/清理差异
- **信号特征**：一端清理、另一端不清理。
- **grep/diff 线索**：
  ```bash
  rg -n "CancelTask|RemoveTask|cancel|weak_ptr|cancelCallback" --type cpp
  ```
- **风险机制**：UAF 或泄漏。
- **风险等级**：P0

### DIV-061 超时处理差异
- **信号特征**：等待/取消超时阈值不同。
- **风险机制**：挂死或提前失败。
- **风险等级**：P1

### DIV-062 事件订阅/退订差异
- **信号特征**：订阅者泄漏或漏回调。
- **grep/diff 线索**：
  ```bash
  rg -n "Subscribe|Unsubscribe|RegisterObserver|UnregisterObserver" --type cpp
  ```
- **风险机制**：内存泄漏或回调错乱。
- **风险等级**：P1

### DIV-063 定时器/延后任务差异
- **信号特征**：触发时机/重复策略不同。
- **风险等级**：P2

---

## 层九：资源限制 / 可靠性层（P2，边界行为）

### DIV-064 队列/缓存上限差异
- **信号特征**：上限不同。
- **grep/diff 线索**：
  ```bash
  rg -n "MAX_|kMax|capacity|reserve\(|resize\(" --type cpp services/
  ```
- **风险机制**：一端 OOM/拒绝，另一端放行。
- **风险等级**：P2

### DIV-065 IPC 数据大小限制差异
- **信号特征**：单次 Parcel 大小阈值不同。
- **风险机制**：一端成功一端失败。
- **风险等级**：P2

### DIV-066 fd 传递上限差异
- **信号特征**：单次传递 fd 数量上限不同。
- **风险等级**：P2

### DIV-067 重试/退避策略差异
- **信号特征**：重试次数/间隔不同。
- **风险机制**：雪崩或提前放弃。
- **风险等级**：P2

### DIV-068 批量操作分片差异
- **信号特征**：分片大小不同。
- **风险机制**：行为/性能差异。
- **风险等级**：P2

---

## 配对校验速查表（Stub ↔ Proxy 必对齐）

> 命中 DIV-001~DIV-007 任一条时，必须对照下表确认蓝黄两端是否成对。

| 客户端 (Proxy) | 服务端 (Stub) | 校验点 |
|----------------|---------------|--------|
| `data.WriteInt32(x)` | `data.ReadInt32()` | 类型一致、顺序一致、数量一致 |
| `data.WriteUint32(x)` | `data.ReadUint32()` | 不可与 Int32 混用 |
| `data.WriteString(s)` | `data.ReadString()` | 编码一致 |
| `data.WriteRemoteObject(t)` | `data.ReadRemoteObject()` | 不可与 StrongNone 混用 |
| `data.WriteParcelable(p)` | `data.ReadParcelable()` | 嵌套 Marshalling 同步 |
| `data.WriteFileDescriptor(fd)` | `data.ReadFileDescriptor()` | dup 所有权一致 |
| `reply.WriteInt32(ret)` | `reply.ReadInt32()` | 返回路径对齐 |

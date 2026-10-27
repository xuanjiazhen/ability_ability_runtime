# aa force-stop 与 onCreate lastExitReason 关系分析

> 分析基线：ability_runtime master 分支（本仓 HEAD），含 6.0/6.1/7.0 Release 分支对照与 git 历史考古。
> 结论均基于代码证据，标注 `文件:行号`。

---

## 1. 结论速览

1. `aa force-stop` 的 `-r` 参数**必须与 `-p` 同时使用**才会预记录退出原因（落库）。只输 `-r` 时，无论值有效与否，**不产生任何预记录**。
2. 无预记录时，下次启动 onCreate 拿到的 `lastExitReason` 是 `LaunchParam` 结构体的**默认值 `NORMAL(2)`**，不是 UNKNOWN。
3. `-r` 的原始字符串只影响 hisysevent 事件文案（killReason），不影响 `lastExitReason`。
4. KV 记录跨重启持久、按 ability 消费。**残留记录会让后续"只输 -r"的测试看起来生效**（测试自污染），做对照实验必须先清残留（卸载重装）。
5. master 上唯一能"事后补录"指定 reason 的路径是 Hiview/MemMgr 专用 IPC（2025-02 引入），属仓外行为。
6. **信号补录修正**：`aa force-stop` 本身是 SIGKILL，干净设备上会触发 appspawn 信号管道补录
   （`RecordSignalReason`），可能落一条 `REASON_NORMAL + subReason=SIGKILL(9)`——onCreate 仍显示
   2，但 `lastExitDetailInfo.exitSubReason=9` 是信号补录指纹。补录体系三个消费方
   （appspawn 信号管道/memmgr/hiview）全是**查杀或归因方在死亡后才拿到原因**的场景，
   与"杀前已知的 -r"无关，也无法还原 loose -r 的值。

---

## 2. 命令入口与参数

`tools/aa/src/ability_command.cpp:787` `RunAsForceStop()`：

| 参数 | 解析 | 默认值 | 用途 |
|---|---|---|---|
| `<bundle-name>` | `argList_[0]`，必选 | — | 杀进程目标 |
| `-p pid` | `ConvertPid()`（ability_command.cpp:852） | 0 | 预记录时定位进程 |
| `-r kill-reason` | `ConvertExitReason()`（tools/aa/include/ability_tool_convert_util.h:26） | `REASON_UNKNOWN` + `"aa force-stop"` | 决定预记录枚举 + kill 事件文案 |
| `-u userId` | `StrToInt`，失败回退 -1 | -1 | 仅影响杀哪个用户，不参与退出原因 |

`-r` 支持的字符串映射（ability_tool_convert_util.h:26-69）：

```
UNKNOWN → REASON_UNKNOWN          NORMAL → REASON_NORMAL
CPP_CRASH → REASON_CPP_CRASH      JS_ERROR → REASON_JS_ERROR
APP_FREEZE / ABILITY_NOT_RESPONDING → REASON_APP_FREEZE
PERFORMANCE_CONTROL → REASON_PERFORMANCE_CONTROL
RESOURCE_CONTROL → REASON_RESOURCE_CONTROL
UPGRADE → REASON_UPGRADE
其余任意字符串 → REASON_UNKNOWN
```

注意：**不支持** `CJ_ERROR`、`USER_REQUEST`、`SIGNAL`。

---

## 3. 执行链路（两条独立链）

### 3.1 链 A：预记录退出原因（条件：`pid > 0 && reason != REASON_UNKNOWN`，ability_command.cpp:831）

```
RecordProcessExitReason(pid, {reason, "aa force-stop"}, killId=REASON_AA_FORCE_STOP)
→ AMS 权限校验：IsSACall() || IsShellCall()（ability_manager_service.cpp:3897）
→ AppExitReasonHelper::RecordProcessExitReason（app_exit_reason_helper.cpp:222）
   ① GetApplicationInfoByProcessID：pid 必须是存活应用进程，否则记录失败（仅日志，kill 照常）
   ② UIExtension 退出原因：SetUIExtensionAbilityExitReason
   ③ killId≠-1 → NotifyAppMgrRecordExitReasonCompability（app_mgr_service_inner.cpp:8100）
      → appRecord->SetKillId(REASON_AA_FORCE_STOP)（影响 PROCESS_KILL 事件的 KILL_ID）
   ④ NotifyAppMgrRecordExitReason（app_mgr_service_inner.cpp:8079）
      → SetExitReason/SetExitMsg/SetReasonExist(true)
   ⑤ SetAppExitReason（app_exit_reason_data_manager.cpp:126）
      → DistributedKV 持久化：key=accessTokenId，value=abilityList+reason+exitMsg+processInfo
      ⚠️ 若该 app 无活跃 UIAbility 记录 → ERR_GET_ACTIVE_ABILITY_LIST_EMPTY，KV 不落盘
         （app_exit_reason_helper.cpp:322）
```

- 预记录的 `exitMsg` 是**硬编码 "aa force-stop"**，不是 `-r` 原始字符串（ability_command.cpp:832）
- 预记录发生在 kill **之前**，要求进程此时仍存活
- 若 `-p` 指向的进程与 bundleName 不是同一应用，原因会记到 pid 所属应用上

### 3.2 链 B：杀进程（ability_command.cpp:840）

```
KillProcessWithUserId(bundleName, false, 0, inputReason, userId)（ability_manager_service.cpp:9509）
→ 权限校验（collaborator / VerifyKillProcessPermission）
→ keepalive 检查：保活应用且内存充足 → KILL_PROCESS_KEEP_ALIVE 拒杀（:9556）
→ AppScheduler::KillApplication → AppMgr KillApplicationByBundleName（app_mgr_service_inner.cpp:3916）
   → ProcessExitByBundleNameAndAppIndex：SetKilling + ScheduleProcessSecurityExit（优雅退出请求）
   → WaitProcessesExitAndKill（:3584）
      - 逐 pid SetKillReason(inputReason 字符串)
      - 等待优雅退出；超时则 KillProcessByPid(pid, reason) → kill(pid, SIGKILL)（:4478/:4553）
   → NotifyAppStatus(COMMON_EVENT_PACKAGE_RESTARTED)
```

`inputReason`（`-r` 原始字符串）在此链中只进 `SetKillReason` → hisysevent
`PROCESS_KILL`/`APP_TERMINATE` 的 reason 字段（前缀 `"Kill Reason:..."`，
app_mgr_service_inner.cpp:4490），**不进 KV、不进 launchParam**。

---

## 4. onCreate lastExitReason 的来源闭环

### 4.1 写（谁落库）

KV（`AppExitReasonDataManager`，DistributedKV，key=accessTokenId）的全部写入方（master 穷举）：

| # | 入口 | 触发方 | loose -r 是否命中 |
|---|---|---|---|
| 1 | `RecordProcessExitReason(pid, exitReason)`（ability_manager_service.cpp:3889） | aa 工具（需 -p+-r）、SA `KillProcessWithReason` | ❌ |
| 2 | `RecordProcessExitReason(pid, uid, exitReason)`（:3907） | **仅 HIVIEW_UID / MEMMGR_UID**（:3910 白名单） | ❌（外部方） |
| 3 | `RecordAppExitReason`（应用自报，:3880） | 应用进程自杀上报 | ❌（app 不知道 -r） |
| 4 | `RecordAppExitReason(bundleName, uid, appIndex)`（:9660） | 应用升级（REASON_UPGRADE） | ❌ |
| 5 | `RecordAppWithReason` / `RecordAppWithReasonByUserId` | `aa forceexitapp`、`StopUser`（:11199）、卸载、自启动关闭 | ❌ |
| 6 | `RecordSignalReason`（app_exit_reason_data_manager.cpp:292） | appspawn 信号管道（REASON_NORMAL+signal）——**含 aa force-stop 自身的 SIGKILL** | ❌（只产生 NORMAL+信号号） |
| 7 | `RecordProcessExitReasonForTimeout` | scene_board 加载超时 | ❌ |

### 4.2 读（下次启动）

- 创建 AbilityRecord 时异步读取：`MissionListManager::LoadLastExitReasonAsync`
  （mission_list_manager.cpp:663/2744）或 scene_board 路径
  `UIAbilityLifecycleManager::SetLastExitReasonAsync`（ui_ability_lifecycle_manager.cpp:280）
- `GetAppExitReason`（app_exit_reason_data_manager.cpp:211）按
  `accessTokenId + abilityName` 匹配（STANDARD 模式 abilityName 追加 persistentId，
  与写入侧 `GetActiveAbilityListLocked` 命名规则一致，ui_ability_lifecycle_manager.cpp:3826）
- 命中 → `AbilityRecord::SetLastExitReason`（ability_record.cpp:2465）填充
  `lifeCycleStateInfo_.launchParam.lastExitReason/lastExitMessage/lastExitDetailInfo`
- **一次性消费**：默认 `cleanFlag=true`，命中的 ability 从列表移除；列表空则删除 KV 条目
  （app_exit_reason_data_manager.cpp:244-252）

### 4.3 消费（onCreate）

launchParam 随 LoadAbility IPC 下发 → `UIAbilityImpl` → `Ability::SetLaunchParam`
（ui_ability_impl.cpp:214 → ability.cpp:1066）→ `onCreate(want, launchParam)`。

`Reason → LastExitReason` 映射（ability_record.cpp:2491，1:1）；
`REASON_CJ_ERROR` 无 case，落入 default → `LASTEXITREASON_UNKNOWN`。

---

## 5. `-r` 场景矩阵（核心结论）

| 场景 | KV 预记录 | 下次 onCreate lastExitReason |
|---|---|---|
| 只输 `-r`（有效值，无 -p） | 否 | **2 NORMAL** |
| 只输 `-r`（无效值/UNKNOWN，无 -p） | 否 | **2 NORMAL** |
| 不带 `-r` | 否 | 2 NORMAL |
| `-r` 但 pid≤0 / pid 非应用进程 / 已死 | 失败（仅日志） | 2 NORMAL |
| `-r` 有效但 app 无活跃 UIAbility | NotifyAppMgr 成功但 KV 不落盘 | 2 NORMAL |
| `-p <有效pid> -r <有效值>` | 是 | 对应枚举（CPP_CRASH=3、UPGRADE=8 等） |
| `-p -r UNKNOWN` | 否（门槛拦截） | 2 NORMAL |

**要点**：只输 `-r` 时，有效与无效对外完全等价——三种情况 kill 照常、KV 无预记录、
launchParam 无预记录值。`-r` 在 loose 场景下不参与任何结果。

**信号补录叠加效应**：无有效预记录的 force-stop 是 SIGKILL 死亡，appspawn 信号管道
可能在死后补一条 `REASON_NORMAL + subReason=SIGKILL(9)`（条件：管道链路正常、
`CacheExitInfo` 赢得时序竞态、该 accessTokenId 无既有记录，app_exit_reason_data_manager.cpp:292）。
这不改变 lastExitReason=2 的结果，但会让 lastExitDetailInfo 部分字段有值（见第 8 节指纹表）。

---

## 6. 默认值 NORMAL(2) 的赋值链

赋值点是 `LaunchParam` 结构体的**成员默认初始化**：

`interfaces/inner_api/ability_manager/include/launch_param.h:78-86`
```cpp
struct LaunchParam : public Parcelable {
    LaunchReason launchReason = LaunchReason::LAUNCHREASON_UNKNOWN;
    LastExitReason lastExitReason = LastExitReason::LASTEXITREASON_NORMAL;  // ← 这里，= 2
    ...
};
```

传递链（无人修改，默认值一路穿透）：

1. AMS 侧持有：`AbilityRecord.lifeCycleStateInfo_.launchParam`（lifecycle_state_info.h:69，默认构造）
2. KV 未命中 → `isSetReason=false` → 唯一写入点 `AbilityRecord::SetLastExitReason`（ability_record.cpp:2486）不执行
3. `LaunchParam::Marshalling`（services/abilitymgr/src/launch_param.cpp:76）随 LoadAbility 下发
4. 应用侧 `Ability::SetLaunchParam`（ability.cpp:1066-1069）→ onCreate 的 `GetLaunchParam()`

ArkTS 枚举（`@ohos.app.ability.AbilityConstant.ets:58-70`）：
`UNKNOWN=0, ABILITY_NOT_RESPONDING=1, NORMAL=2, CPP_CRASH=3, JS_ERROR=4, APP_FREEZE=5,
PERFORMANCE_CONTROL=6, RESOURCE_CONTROL=7, UPGRADE=8, USER_REQUEST=9, SIGNAL=10`

> 指纹特征：JS 侧 `LaunchParamImpl` 类默认是 UNKNOWN(0)（AbilityConstant.ets:130），
> 若 onCreate 拿到 2，可能来自 native 结构体默认（KV 未命中），也可能来自一条
> `REASON_NORMAL` 记录（如信号补录）——需结合 lastExitDetailInfo 区分（见第 8 节指纹表）。

---

## 7. KV 残留陷阱（测试自污染）

- KV 是 DistributedKV **持久化**的，跨设备重启保留
- 历史测试中任何一次 `-p <pid> -r XXX` 成功落库后，记录按 ability 逐个消费，
  未消费部分会残留
- 之后"只输 -r"的测试 reopen 时读到残留记录，**看起来像 -r 生效了**
- 清残留方法：卸载重装应用（`UninstallApp` → `DeleteAppExitReason`，
  ability_manager_service.cpp:9649）

---

## 8. 干净对照实验方法

```bash
# 1. 清残留
hdc uninstall <bundle>; hdc install xxx.hap   # 或其他卸载重装方式

# 2. 单次流程（用 UPGRADE=8 避免与默认值 NORMAL=2 重合）
aa start <ability>                 # 启动应用
aa force-stop <bundle> -r UPGRADE  # 只输 -r
aa start <ability>                 # 立即重开
hilog | grep -E "SetAppExitReason|EXIT_REASON_TAG|GetAppExitReason"
```

日志判读：

| 日志 | 含义 |
|---|---|
| `SetAppExitReason`（"bundleName: ..., tokenId: ..."，app_exit_reason_data_manager.cpp:148） | 有人落库了 |
| `[EXIT_REASON_TAG] pid:..., reason:..., callerPid:...`（ability_manager_service.cpp:3917） | (pid,uid) 外部补录变体，callerPid 指明是谁（hiview/memmgr） |
| `GetAppExitReason`（app_exit_reason_data_manager.cpp:246） | 读侧命中情况与 isSetReason |

预期结果（修正版）：干净环境下只输 `-r`，无 aa 工具侧 `RecordProcessExitReason` 调用，
但 SIGKILL 死亡**可能**触发 appspawn 信号补录（`RecordSignalReason` → `SetAppExitReason`）。
onCreate 恒为 2，用 lastExitDetailInfo 指纹区分记录来源：

| 来源指纹 | lastExitReason | lastExitMessage | exitSubReason | killReason | 其他 detailInfo |
|---|---|---|---|---|---|
| `-p -r` 预记录 | 非 2 的映射值（NORMAL 时也是 2） | **"aa force-stop"** | -1 | GetKillReason(AA_FORCE_STOP) 非空 | pid/uid/timestamp=记录快照 |
| 信号补录（SIGKILL：内核查杀/无 -p 的 force-stop） | 2 | "" | **9（SIGKILL）** | "" | pid/uid/processName/timestamp 有值 |
| 无任何记录（补录竞态输掉/记录已消费） | 2 | "" | -1 | "" | pid=-1、timestamp=0 全默认 |

判别逻辑：
- `lastExitMessage="aa force-stop"` → 一定是 aa 工具预记录（`-p -r`），因为该字符串是硬编码（ability_command.cpp:832）
- `lastExitMessage=""` 且 `exitSubReason=9` → 信号补录（内核/任意 SIGKILL）
- `lastExitMessage=""` 且 `exitSubReason=-1`、`timestamp=0` → KV 未命中，默认值穿透

补录体系的设计定位：三个消费方（appspawn 信号管道、memmgr、hiview）都是
**查杀或归因方在进程死亡后才拿到原因**的场景——内核查杀只有信号号、memmgr/hiview
在杀后归因。`-r` 是"杀前已知"的业务原因，走预记录链；loose `-r` 的字符串随进程
消亡而消失，任何补录方都无法将其还原为枚举（hiview 从 hisysevent 文案反解属仓外路径）。

---

## 9. 相关联路径对照

| 路径 | 说明 |
|---|---|
| `aa forceexitapp`（`ABILITY_FAULT_AND_EXIT_TEST` 编译开关，ability_command.cpp:2927） | 走 `ForceExitApp`（ability_manager_service.cpp:3924），killId 同样强制 REASON_AA_FORCE_STOP，exitMsg="Force exit app by aa."，按 uid 全量记录 |
| SA `KillProcessWithReason`（ability_manager_service.cpp:14874） | `RecordProcessExitReason(..., fromKillWithReason=true)`，killMsg 进 lastExitDetailInfo |
| SCB 用户关闭（CloseUIAbilityBySCB/CleanUIAbilityBySCB，:5913/:18395） | 记 REASON_NORMAL "User Request" → onCreate 显示 2 |
| 应用升级（HandleAppUpgradeProcess，:9655） | 记 REASON_UPGRADE |
| appspawn 信号管道（services/abilitymgr/src/utils/exit_reason_util.cpp:28） | 信号死亡记 REASON_NORMAL + subReason=signal；AMS 启动时监听 APPSPAWN_STARTED 建管道（ability_manager_service.cpp:595），覆盖所有 SIGKILL 死亡（含 force-stop 自身） |
| `HandleAppDiedForRecovery`（ability_manager_service.cpp:20016） | 读 KV，reason==REASON_CPP_CRASH 且前台死亡 5s 内 → 带 PARAM_ABILITY_RECOVERY_RESTART 自动重启（:20058-20061）；这也是"误落库 CPP_CRASH"的风险所在 |

---

## 10. git 历史时间线

| 时间 | commit | 内容 |
|---|---|---|
| 2024-06-01 | `b00ac99924` "Support recovery when background" | `aa force-stop` 引入 `-p/-r`，**从第一天起就要求 `pid != 0` 才记录** |
| 2025-02-11 | `779ed3705a` "Record exit reason of a process which could be dead" | 引入 (pid,uid) 补录机制 + `GetKilledProcessInfo` + 死亡记录保留 |
| 2025-06-16 | `06a8f1a466` "Code simplify" | 删除 `GetKilledProcessInfo`/`AbilityRecordDeathManager`，保留 (pid,uid) `RecordProcessExitReason` + 3s `ExitInfoDataManager` 缓存 |

Release 分支对照：6.0/6.1/7.0 的 `RunAsForceStop` 均为 `pid != 0 && reason != REASON_UNKNOWN`
门槛（6.0 的映射函数名为 `CovertExitReason`，6.1 起为 `AbilityToolConvertUtil::ConvertExitReason`）。
**不存在"loose -r 历史上能落库"的版本。**

---

## 11. 关键代码位置索引

| 位置 | 内容 |
|---|---|
| tools/aa/src/ability_command.cpp:787-850 | aa force-stop 命令实现 |
| tools/aa/src/ability_command.cpp:831 | 预记录门槛 `pid > 0 && reason != REASON_UNKNOWN` |
| tools/aa/include/ability_tool_convert_util.h:26 | `-r` 字符串 → Reason 映射 |
| services/abilitymgr/src/ability_manager_service.cpp:9509 | KillProcessWithUserId（链 B 入口） |
| services/abilitymgr/src/ability_manager_service.cpp:3889/3907 | 两条 RecordProcessExitReason IPC 实现 |
| services/abilitymgr/src/app_exit_reason_helper.cpp:222/269 | 预记录/补录 helper |
| services/abilitymgr/src/app_exit_reason_data_manager.cpp:126/211 | KV Set/Get |
| services/abilitymgr/src/mission/mission_list_manager.cpp:4202 | 读侧 SetLastExitReason |
| services/abilitymgr/src/ability_record.cpp:2465/2491 | launchParam 填充 + Reason 映射 |
| interfaces/inner_api/ability_manager/include/launch_param.h:80 | 默认值 NORMAL(2) |
| interfaces/inner_api/ability_manager/include/ability_manager_client.h:2259 | (pid,uid) 变体注释 "Reserved for hiview and memory manager only" |
| services/appmgr/src/app_mgr_service_inner.cpp:4490 | killReason 字符串（hisysevent 文案） |
| services/abilitymgr/src/ability_manager_service.cpp:20016 | HandleAppDiedForRecovery（CPP_CRASH 自动恢复） |
| services/abilitymgr/src/ability_manager_service.cpp:402/16533 | 3s 死后补录缓存窗口 |
| interfaces/inner_api/ability_manager/include/exit_reason.h:26/43 | ExitReason / ExitReasonCompability 字段与默认值（subReason=-1、killId=-1） |
| interfaces/inner_api/ability_manager/include/last_exit_detail_info.h:25 | LastExitDetailInfo 字段与默认值 |
| services/abilitymgr/src/app_exit_reason_data_manager.cpp:338 | KV value JSON 序列化字段集 |
| services/abilitymgr/src/utils/exit_reason_util.cpp:28/73 | appspawn 信号管道读取与建链（信号补录） |
| services/abilitymgr/src/ability_manager_service.cpp:595/603 | APPSPAWN_STARTED 参数监听与管道 fd 处理注册 |

## 12. 场景基线
1.应用A首次启动，onCreate中获取到的信息，pid: -1, processName: "", exitSubReason: -1, exitMsg: "", rss: 0, pss: 0, timestamp: 0, processstate: 0, killReason: undefined, lastExitMessage: "", launchReason: 1, lastExitReason: 2
2.使用aa force-Stop bundleNameA 杀应用A，重启A后，onCreate中获取到的信息，pid: -1, processName: "", exitSubReason: -1, exitMsg: "", rss: 0, pss: 0, timestamp: 0, processstate: 0, killReason: "aa force-stop", lastExitMessage: "", launchReason: 1, lastExitReason: 2
3.使用aa force-Stop bundleNameA -r CPP_CRASH 杀应用A，重启A后，onCreate中获取到的信息，pid: -1, processName: "", exitSubReason: -1, exitMsg: "", rss: 0, pss: 0, timestamp: 0, processstate: 0, killReason: "aa force-stop", la.stExitMessage: "", launchReason: 1, lastExitReason: 2
4.使用aa force-Stop bundleNameA -r JS_ERROR 杀应用A，重启A后，onCreate中获取到的信息，pid: -1, processName: "", exitSubReason: -1, exitMsg: "", rss: 0, pss: 0, timestamp: 0, processstate: 0, killReason: "aa force-stop", la.stExitMessage: "", launchReason: 1, lastExitReason: 2
5.使用aa force-Stop bundleNameA -r JS_ER 杀应用A，重启A后，onCreate中获取到的信息，pid: -1, processName: "", exitSubReason: -1, exitMsg: "", rss: 0, pss: 0, timestamp: 0, processstate: 0, killReason: "aa force-stop", la.stExitMessage: "", launchReason: 1, lastExitReason: 2
6.使用aa force-Stop bundleNameA -r JS_ERROR -p 48118(有效pid), 杀应用A，重启A后，onCreate中获取到的信息，pid: 48118, processName: "bundleNameA", exitSubReason: -1, exitMsg: "", rss: 0, pss: 0, timestamp: 有效timestamp, processstate: 1, killReason: "aa force-stop", la.stExitMessage: "aa force-stop", launchReason: 1, lastExitReason: 4
7.使用aa force-Stop bundleNameA -r CPP_CRASH -p 48118(有效pid), 杀应用A，重启A后，onCreate中获取到的信息，pid: 48118, processName: "bundleNameA", exitSubReason: -1, exitMsg: "", rss: 0, pss: 0, timestamp: 有效timestamp, processstate: 1, killReason: "aa force-stop", la.stExitMessage: "aa force-stop", launchReason: 1, lastExitReason: 3
8.使用aa force-Stop bundleNameA -r CPP_CRASH -p 4811(无效pid), 杀应用A，重启A后，onCreate中获取到的信息，pid: -1, processName: "", exitSubReason: -1, exitMsg: "", rss: 0, pss: 0, timestamp: 0, processstate: 0, killReason: "aa force-stop", lastExitMessage: "", launchReason: 1, lastExitReason: 2
8.使用aa force-Stop bundleNameA -r CPP_CRASH -p -8(无效pid), 杀应用A，重启A后，onCreate中获取到的信息，pid: -1, processName: "", exitSubReason: -1, exitMsg: "", rss: 0, pss: 0, timestamp: 0, processstate: 0, killReason: "aa force-stop", lastExitMessage: "", launchReason: 1, lastExitReason: 2
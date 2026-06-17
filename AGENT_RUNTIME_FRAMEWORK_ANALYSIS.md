# agent_runtime_framework 解读

## 1. 总体定位

`agent_runtime_framework` 是 Agent 能力的运行时框架。它提供三类能力：

1. AgentCard 元数据管理: 查询、注册、更新、删除、安装包 metadata 自动导入。
2. Agent Extension 连接管理: 连接普通 APP/Atomic Service/Low-code AgentExtensionAbility。
3. Agent Extension 运行时: JS/ETS/Native ExtensionAbility、UIExtensionAbility、双向 IPC 通信封装。

核心链路可以理解为：

```text
ArkTS agentManager API
  -> ANI/N-API agent_manager
  -> AgentManagerClient / AgentConnectionManager
  -> IAgentManager Binder IPC
  -> AgentManagerService(agentmgr SA)
  -> AgentCardMgr / AgentCardDbMgr / AbilityManagerClient
  -> AgentExtensionAbility(JS/ETS) + IAgentConnector/IAgentReceiver 双向通信
```

这个目录和你后续做 agent manager 最相关的部分主要在：

- `services/agentmgr`: AgentManagerService 和连接复用/Low-code host session 管理。
- `interfaces/inner_api`: `IAgentManager`、`AgentManagerClient`、`AgentConnectionManager`、`AgentCard`。
- `interfaces/kits/native/agent_extension/connection`: AgentExtension 双向 IPC 接口。
- `frameworks/ets/ets`: 对外 ArkTS 声明。
- `frameworks/js/napi` 和 `frameworks/ets/ani`: JS/ETS native 桥。
- `frameworks/native`: AgentExtension 和 AgentUIExtension native 基类与动态加载器。

## 2. 目录结构

```text
agent_runtime_framework/
  frameworks/
    ets/ani/
    ets/ets/
    js/js/
    js/napi/
    native/
  interfaces/
    inner_api/
    kits/native/agent_extension/connection/
  services/
    agentmgr/
    common/
```

`frameworks/ets/ets` 是 ArkTS API 声明和应用侧类型。`frameworks/js/napi`、`frameworks/ets/ani` 是 native 桥实现。`frameworks/native` 是 ExtensionAbility 基类和 loader。`interfaces/inner_api` 是服务客户端和公共数据模型。`services/agentmgr` 是系统服务。`services/common` 放 AgentCard 校验和 semver 工具。

## 3. 构建产物

主要构建目标：

- `interfaces/inner_api:agent_fwk`: `AgentCard` 等公共数据结构。
- `interfaces/inner_api:agent_manager_interface`: 由 `IAgentManager.idl` 生成 proxy/stub。
- `interfaces/inner_api:agent_manager`: `AgentManagerClient` inner API 客户端。
- `interfaces/inner_api:agent_connection_manager`: 客户端侧连接复用管理。
- `interfaces/kits/native/agent_extension/connection:agent_extension_connection`: `IAgentConnector` / `IAgentReceiver` proxy-stub。
- `services/agentmgr:agentmgr`: SA 共享库，依赖 AbilityManager、AppManager、BundleManager、Distributed KV 等。
- `frameworks/js/napi/agent_manager:agentmanager_napi`: Agent manager N-API 模块，安装到 `module/app/agent`。
- `frameworks/native/agent_extension`: AgentExtension native 基类。
- `frameworks/native/agent_ui_extension_ability`: AgentUIExtension native 基类。

## 4. 对外 ArkTS API

`frameworks/ets/ets/@ohos.app.agent.agentManager.ets` 暴露 Promise 风格 API：

- `getAllAgentCards`
- `getAgentCardsByBundleName`
- `getAgentCardByAgentId`
- `registerAgentCard`
- `updateAgentCard`
- `deleteAgentCard`
- `connectAgentExtensionAbility`
- `disconnectAgentExtensionAbility`
- `connectServiceExtensionAbility`
- `disconnectServiceExtensionAbility`
- `notifyLowCodeAgentComplete`

它通过 `loadLibraryWithPermissionCheck("agentmanager_ani.z", "@ohos.app.agent.agentManager")` 加载 native 实现，并在 `taskpool.execute` 中调用 native function。

Agent Extension 侧基类：

- `AgentExtensionAbility`: 提供 `onCreate`、`onDestroy`、`onData`、`onAuth`、`onConnect`、`onDisconnect`、`onAgentInvoked`。
- `AgentUIExtensionAbility`: 继承 `UIExtensionAbility`。
- `AgentExtensionConnectCallback`: host 侧收到 `onData`、`onAuth`、`onDisconnect`。

## 5. IPC 接口

`IAgentManager.idl` 是 AgentManagerService 的 IPC 边界：

- 卡片查询: `GetAllAgentCards`、`GetAgentCardsByBundleName`、`GetAgentCardByAgentId`、`GetCallerAgentCardByAgentId`
- 卡片修改: `RegisterAgentCard`、`UpdateAgentCard`、`DeleteAgentCard`
- AgentExtension 连接: `ConnectAgentExtensionAbility`、`DisconnectAgentExtensionAbility`
- ServiceExtension 连接: `ConnectServiceExtensionAbility`、`DisconnectServiceExtensionAbility`
- Low-code 完成通知: `NotifyLowCodeAgentComplete`

`AgentManagerClient` 是 singleton 客户端，负责按需加载 `AGENT_MGR_SERVICE_ID`、缓存 proxy、处理 SA 死亡。

`AgentConnectionManager` 是客户端侧连接复用层：

- 用 `agentId + bundle/module/ability` 复用同一个 `AgentConnection`。
- 连接中状态有 30 秒 timeout，超时后不再复用旧连接。
- 一个 connection 可挂多个 `AbilityConnectCallback`。
- 最后一个 callback 断开时才真正调 `AgentManagerClient::DisconnectAgentExtensionAbility`。

## 6. AgentCard 数据模型

`AgentCard` 大体对应 agent 能力描述：

- `agentId`
- `type`: `APP`、`ATOMIC_SERVICE`、`LOW_CODE`
- `name`、`description`、`version`、`category`
- `provider`
- `documentationUrl`
- `capabilities`
- `defaultInputModes`、`defaultOutputModes`
- `skills`
- `iconUrl`
- `extension`
- `appInfo`: `bundleName`、`moduleName`、`abilityName`、`deviceTypes`、`minAppVersion`

`AgentCardsRawData` 和 CLI 的 `ToolsRawData` 类似，用 raw data 形式批量传输 card 列表。

`StoredAgentCardEntry` 在服务侧额外记录 `source`：

- `BUNDLE`: 从安装包 metadata 解析。
- `API`: 通过 API 注册/更新。

这个 source 对安装包更新时的覆盖策略很重要。

## 7. AgentCard 存储和导入

`AgentCardDbMgr` 使用 Distributed KV 的 `SingleKvStore`：

- `APP_ID = "agent_db"`
- `STORE_ID = "agent_card_infos"`
- base dir: `/data/service/el1/public/database/ability_manager_service`
- key: JSON，包含 `bundleName` 和 `userId`
- value: JSON，包含 cards 数组和 `lastUpdateSource`
- 数据损坏时会删除并重建 KV store

`AgentCardMgr` 负责业务规则：

- 安装/更新 bundle 时读取 extension metadata `ohos.extension.agent`。
- 只处理 `ExtensionAbilityType::AGENT`。
- 每 bundle/user 最多 1000 个 AgentCard。
- 自动补齐 `appInfo.bundleName/moduleName/abilityName`。
- Low-code card 只接受 system app 目标 bundle。
- 安装包导入时会用 semver 防止降级覆盖。
- API 注册/更新也要求合法 semver。

API 修改规则：

- `registerAgentCard`: 必填字段完整、iconUrl 合法、版本合法、不可重复。
- `updateAgentCard`: agentId/appInfo/iconUrl/版本合法，且新版本不能低于旧版本。
- `deleteAgentCard`: 按 bundleName + agentId 删除，空后删除整个 bundle/user key。

## 8. AgentManagerService 核心

`AgentManagerService` 是 `SystemAbility + AgentManagerStub`。

启动时：

1. 创建 task handler 和 `AgentEventHandler`。
2. `Publish(AgentManagerService::GetInstance())`。
3. 监听 Bundle Manager SA。
4. Bundle Manager 可用后注册 `AgentBundleEventCallback`。

停止时：

- 清空 tracked connections。
- 清空 caller connection counts。
- 清空标准 agent sessions。
- 清空 low-code host sessions 和 agent owner 索引。

卡片接口权限：

- 查询 card: system API caller + `PERMISSION_GET_AGENT_CARD`
- 修改 card: system API caller + `PERMISSION_MODIFY_AGENT_CARD`

连接 AgentExtension 权限：

- system API caller
- `PERMISSION_CONNECT_AGENT`
- connection 不能为空
- 调用方进程必须是前台状态

连接 ServiceExtension:

- system API caller
- caller token 和 connection 不能为空
- 目标必须解析为 `ExtensionAbilityType::SERVICE`

## 9. 标准 Agent 连接模型

普通 APP/Atomic Service Agent 走标准连接路径。

调用链：

```text
agentManager.connectAgentExtensionAbility
  -> AgentConnectionManager::ConnectAgentExtensionAbility
  -> AgentManagerClient::ConnectAgentExtensionAbility
  -> AgentManagerService::ConnectAgentExtensionAbility
  -> ValidateConnectAgentRequest
  -> ResolveConnectAgentTarget
  -> PrepareStandardAgentConnectWant
  -> BuildStandardAgentKey
  -> FindStandardSessionLocked
  -> CreateStandardAgentSession 或复用已有 session
  -> AbilityManagerClient::ConnectAbilityWithExtensionType(..., AGENT)
```

`StandardAgentKey` 由以下字段组成：

- `callerUid`
- `agentId`
- `bundleName`
- `moduleName`
- `abilityName`

服务侧会折叠相同 key 的连接：

- 如果已有 session 正在连接，把 caller callback 放到 pending list。
- 如果已有 session 已连接，直接用 cached element/remote/result 回调新 caller。
- 首次连接才真正调用 AMS。
- 断开时收集 pending + connected callbacks，再统一通知。

Atomic Service Agent 特殊点：

- 如果目标不存在且是 atomic service，会允许 install-on-demand。
- `connectWant` 会加 `FLAG_INSTALL_ON_DEMAND` 和启动时间参数。

## 10. Low-code Agent 连接模型

Low-code Agent 不按每个 agent 独立连接，而是共享 host extension 连接。

关键结构：

- `AgentHostKey`: `userId + bundleName + moduleName + abilityName`
- `AgentOwnerKey`: `hostUid + agentId`
- `AgentHostSession`: 一个共享 host 连接，挂多个 low-code agent 和多个 caller connection。
- `AgentConnectPlan`: 创建/复用 host session 的中间计划。

调用链：

```text
ConnectAgentExtensionAbility
  -> ResolveConnectAgentTarget
  -> card.type == LOW_CODE
  -> ResolveLowCodeHostInfo
  -> ConnectLowCodeAgentExtensionAbility
  -> PrepareLowCodeConnectPlan
       - 检查同一 hostUid + agentId 是否已 active
       - 创建或复用 AgentHostSession
       - TryRegisterConnectionLocked
       - 写入 agentOwners_ 和 session->agents
  -> 如果 host 已连接: NotifyExistingLowCodeConnection
  -> 否则: CompleteAgentHostConnect
```

连接完成时：

- `HandleAgentHostConnectDone` 缓存 element/remote/result。
- 对 pending agentId 调 `IAgentReceiver::AgentInvoked(agentId)`。
- 回调所有 caller 的 `OnAbilityConnectDone`。

完成通知：

- `NotifyLowCodeAgentComplete(agentId)` 由 caller 调用。
- 它按 `callingUid + agentId` 找到 owner。
- 移除该 agent。
- 如果该 caller 不再拥有 session 内其他 agent，会释放 caller connection。
- 如果 session 已无 agent，会断开共享 host connection。

这个模型适合“一个低代码 host 能承载多个 agent”的场景，但也带来所有权、配额迁移、死亡清理的复杂度。

## 11. Tracked connection 和配额

`trackedConnections_` 以 caller 的 `IRemoteObject` 为 key，保存：

- caller uid
- service-side wrapper connection
- caller remote
- death recipient
- low-code host key
- 是否计入 caller 配额
- 是否正在断开
- standard session key

配额：

- `MAX_CONNECTIONS_PER_CALLER = 5`
- Low-code 共享 host session 中，只有需要真实 connect 的首个 caller 计入配额；后续复用时可能不计入。
- `TransferLowCodeCallerLimitLocked` 会在 caller 完成/死亡时把配额责任转移给同 uid 的其他 caller。

死亡清理：

- caller remote 挂 death recipient。
- standard session caller 死亡: 清理整个标准 session 的相关 tracked connection，并断开 service connection。
- low-code caller 死亡: 移除该 caller 的 agent 记录；如果 session 没有 agent 了，断开 host connection。

## 12. AgentExtension 双向通信

native kit 定义两条 IPC 接口：

`IAgentConnector`: host 调 agent extension。

- `SendData(data)`
- `Authorize(data)`

`IAgentReceiver`: agent extension 回 host。

- `SendData(connectorProxy, data)`
- `Authorize(connectorProxy, data)`
- `AgentInvoked(agentId)`

JS/N-API 桥：

- `JSAgentConnection`: host 侧连接对象，继承 `AbilityConnectCallback`，管理 JS callback、promise、连接 id。
- `JsAgentConnectorStubImpl`: 接收 agent extension 发来的 `SendData/Authorize`，转发到 `JSAgentConnection::OnSendData/OnAuthorize`。
- `JsAgentReceiverProxy`: host 侧拿到的 JS proxy，调用远端 `IAgentReceiver`。
- `JsAgentConnectorProxy`: agent extension 侧 JS proxy，调用远端 `IAgentConnector`。

AgentExtension 侧：

- `JsAgentExtension` / `EtsAgentExtension` 继承 `AgentExtension`。
- `OnConnect` 返回 extension stub。
- `OnSendData`、`OnAuthorize`、`OnAgentInvoked` 投递到 ArkTS 对象方法。
- host proxy 会按 remote object 缓存，断开时释放 reference。

## 13. Native Extension 加载

`frameworks/native/agent_extension` 中的 `AgentExtension::Create` 根据 runtime language 动态加载：

- JS: `libjs_agent_extension.z.so`，符号 `OHOS_CreateJsAgentExtension`
- ETS: `libets_agent_extension.z.so`，符号 `OHOS_CreateEtsAgentExtension`

`AgentExtension` 继承 `ExtensionBase<AgentExtensionContext>`，负责创建并初始化 context。

`AgentUIExtension` 继承 `UIExtensionBase<UIExtensionContext>`，同样按 JS/ETS runtime 加载对应实现。

## 14. 分布式相关落点

当前框架里和分布式最相关的是：

- AgentCard 使用 Distributed KV 存储，但当前 key 仍是 `bundleName + userId`，不是跨设备 agent registry。
- AgentCard 的 `deviceTypes` 会从 HAP module 设备类型继承或过滤，适合作为多设备能力筛选的基础。
- 连接模型全部基于本机 AMS、caller uid、remote object、death recipient，不能直接跨设备复用。
- Low-code host session 已经有 host/owner 的抽象，可以作为“远端 host session”扩展的参考。

如果后续做分布式 Agent manager，建议先明确：

1. AgentCard 是否需要跨设备同步，还是只做本地发现 + 远端查询。
2. `AgentHostKey` 是否要加入 deviceId/networkId。
3. `AgentOwnerKey` 是否要从 uid 扩展为 distributed caller identity。
4. `IAgentReceiver/IAgentConnector` 是否通过分布式通道代理，还是只在本机生命周期内有效。
5. Low-code active 去重是本机去重、host 维度去重，还是全局 agentId 去重。

## 15. 后续改动入口

Agent manager 常见需求和入口：

- 新增 AgentManager API: 改 `IAgentManager.idl`、`AgentManagerClient`、`AgentManagerService`、ANI/N-API、ArkTS 声明。
- 调整 AgentCard 规则: 看 `AgentCard`、`AgentCardMgr`、`AgentCardDbMgr`、`AgentCardUtils`、`sem_ver`。
- 调整 bundle 安装导入: 看 `AgentBundleEventCallback`、`AgentCardMgr::HandleBundleInstall/Update/Remove`。
- 调整普通 Agent 连接复用: 看 `StandardAgentSession`、`BuildStandardAgentKey`、`CreateStandardAgentSession`。
- 调整 Low-code host 复用: 看 `AgentHostSession`、`PrepareLowCodeConnectPlan`、`NotifyLowCodeAgentComplete`。
- 调整连接死亡/配额: 看 `trackedConnections_`、`callerConnectionCounts_`、`HandleCallerConnectionDied`。
- 调整 Extension 双向通信: 看 `IAgentConnector`、`IAgentReceiver`、`JSAgentConnection`、`Js/EtsAgentExtension`。

## 16. 注意点

- 服务侧和客户端侧都做了连接复用，修改连接模型时要同时看 `AgentConnectionManager` 和 `AgentManagerService`。
- Low-code 的 active 标识是 `hostUid + agentId`，不是单纯 agentId。
- Low-code 连接配额有迁移逻辑，断开/死亡路径改动要小心保持 `callerConnectionCounts_` 一致。
- `NotifyLowCodeAgentComplete` 使用 `IPCSkeleton::GetCallingUid()` 参与 owner 查找，跨进程/跨设备代理会受到影响。
- AgentCard API 注册来源会影响安装包更新覆盖策略，source 和 semver 不能随意丢。
- `GetCallerAgentCardByAgentId` 通过 pid 查调用方 bundle，适合本机调用，不适合直接复用到远端调用。

## 17. 测试情况

当前 `agent_runtime_framework` 目录下没有像 CLI 那样明显的 `test/unittest` 子树。后续改 Agent manager 时建议至少补：

- AgentCard JSON/semver/source 合并规则测试。
- `AgentCardDbMgr` key/value 兼容解析测试。
- 标准 Agent session 复用、连接失败回滚、断开通知测试。
- Low-code host session 复用、重复 active、完成通知、caller 死亡清理测试。
- `IAgentConnector/IAgentReceiver` proxy-stub 的双向通信测试。

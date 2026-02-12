# AppServiceExtensionAbility 跨设备支持技术方案

## 一、需求概述

### 1.1 业务背景
- **场景**: 企业IT设备数据克隆
- **流程**: HISMALL (IT设备管理应用) → 拉起数据克隆工具 → 新旧设备建连认证 → HISMALL通过RPC进行IT安全校验
- **核心需求**: `connectAppServiceExtensionAbility` 需要支持跨设备连接,以便在克隆开始前对两端设备进行安全校验(如域账号验证)

### 1.2 现状分析
- **当前限制**: `connectAppServiceExtensionAbility` 只支持本地连接
- **参考实现**: `ServiceExtensionAbility` 已具备完整的跨设备RPC能力
- **基础设施**: DMS (分布式管理服务) 已提供跨设备通信能力

### 1.3 影响范围
- **直接影响**: IT场景数据克隆功能无法落地
- **兼容性要求**: 不能影响已有的本地连接功能
- **安全要求**: 需要经过DMS和安全团队评审

---

## 二、架构设计

### 2.1 整体架构0

```
┌─────────────────────────────────────────────────────────────────┐
│                      应用层 (HISMALL)                             │
│  connectAppServiceExtensionAbility(want with deviceId)           │
└─────────────────────────────┬───────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│                  ConnectionManager (客户端)                       │
│  - 识别deviceId参数                                              │
│  - 传递跨设备标识                                                │
└─────────────────────────────┬───────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│              AbilityManagerService (服务端)                       │
│  ┌──────────────────────────────────────────────────────┐      │
│  │ CheckIfOperateRemote(want)                           │      │
│  │   if (deviceId != localDeviceId)                     │      │
│  │     → DistributedClient::ConnectRemoteAbility()      │      │
│  │   else                                               │      │
│  │     → ConnectLocalAbility() [现有流程]               │      │
│  └──────────────────────────────────────────────────────┘      │
└─────────────────────────────┬───────────────────────────────────┘
                              │
              ┌───────────────┴───────────────┐
              ▼                               ▼
┌───────────────────────┐       ┌──────────────────────────────────┐
│  本地设备流程          │       │  远程设备流程 (通过DMS)           │
│  ConnectLocalAbility  │       │  DistributedClient               │
│  [已实现]             │       │    → DMS Proxy                   │
│                       │       │    → 对端设备AbilityManagerService│
└───────────────────────┘       └──────────────────────────────────┘
```

### 2.2 调用链路

**本地连接 (现有)**:
```
AppServiceExtensionContext::ConnectAbility()
  → ConnectionManager::ConnectExtensionAbilityWithExtensionType()
    → AbilityManagerClient::ConnectAbilityWithExtensionType()
      → AbilityManagerService::ConnectAbility()
        → AbilityManagerService::ConnectLocalAbility()
          → AbilityConnectManager::ConnectAbilityLocked()
```

**跨设备连接 (新增)**:
```
AppServiceExtensionContext::ConnectAbility()
  → ConnectionManager::ConnectExtensionAbilityWithExtensionType()
    → AbilityManagerClient::ConnectAbilityWithExtensionType()
      → AbilityManagerService::ConnectAbility()
        → AbilityManagerService::CheckIfOperateRemote() [新增检测]
          → DistributedClient::ConnectRemoteAbility() [新增路由]
            → DMS Proxy
              → 远程设备AbilityManagerService
```

---

## 三、技术方案详解

### 3.1 核心改动点

#### 3.1.1 ConnectionManager 增强
**文件**: `frameworks/native/ability/ability_runtime/connection_manager.cpp`

**改动说明**:
- `ConnectionInfo` 结构体需要保存 `deviceId` 信息
- `MatchConnection()` 方法需要增加 `deviceId` 匹配逻辑

**代码修改**:
```cpp
// connection_manager.h
struct ConnectionInfo {
    // ... 现有字段
    std::string deviceId;  // 新增: 目标设备ID
};

// connection_manager.cpp
bool ConnectionManager::MatchConnection(
    const sptr<IRemoteObject>& connectCaller,
    const AAFwk::Want& connectReceiver,
    int32_t accountId,
    const std::map<ConnectionInfo, ...>::value_type& connection)
{
    // 现有匹配逻辑...

    // 新增: deviceId匹配
    std::string deviceId = connectReceiver.GetElement().GetDeviceID();
    if (deviceId != connection.first.deviceId) {
        return false;
    }

    // ... 其余逻辑
}
```

#### 3.1.2 AbilityManagerService 路由逻辑
**文件**: `services/abilitymgr/src/ability_manager_service.cpp`

**关键改动**: 在 `ConnectAbility()` 方法中添加远程设备检测和路由

**参考代码位置**: `ability_manager_service.cpp:5366-5378`

**实现方案**:
```cpp
int AbilityManagerService::ConnectAbility(
    const Want &want, const sptr<IAbilityConnection> &connect,
    const sptr<IRemoteObject> callerToken, int32_t userId,
    AppExecFwk::ExtensionAbilityType extensionType, ...)
{
    // ... 现有前置检查逻辑 ...

    // 关键修改: 添加跨设备检测
    if (CheckIfOperateRemote(abilityWant)) {
        TAG_LOGD(AAFwkTag::SERVICE_EXT, "try to ConnectRemoteAbility for AppServiceExtension");
        eventInfo.errCode = ConnectRemoteAbility(abilityWant, callerToken, connect->AsObject());
        return eventInfo.errCode;
    }

    // 本地连接逻辑 (现有代码保持不变)
    // ...
}
```

**优势**:
- 最小化侵入,复用已有的 `CheckIfOperateRemote()` 和 `ConnectRemoteAbility()` 方法
- 本地连接流程完全不变,保证兼容性

#### 3.1.3 CheckIfOperateRemote 实现
**文件**: `services/abilitymgr/src/ability_manager_service.cpp`

**现状**: 该方法已存在,用于检测Want是否指向远程设备

**实现逻辑**:
```cpp
bool AbilityManagerService::CheckIfOperateRemote(const Want &want)
{
    auto element = want.GetElement();
    std::string deviceId = element.GetDeviceID();

    if (deviceId.empty()) {
        return false;  // 空deviceId表示本地
    }

    return CheckIsRemote(deviceId);
}

bool AbilityManagerService::CheckIsRemote(const std::string& deviceId)
{
    if (deviceId.empty()) {
        return false;
    }

    std::string localDeviceId = GetLocalDeviceId();
    return deviceId != localDeviceId;
}
```

### 3.2 DMS集成

#### 3.2.1 DistributedClient 复用
**文件**: `frameworks/native/ability/native/distributed_ability_runtime/distributed_client.cpp`

**现状**: 该文件已实现完整的DMS集成,包括:
- `ConnectRemoteAbility()` - 连接远程Extension (line 61-79)
- `DisconnectRemoteAbility()` - 断开远程连接 (line 81-103)
- 自动提取 callerUid/callerPid/accessToken (line 495-507)

**无需修改**: 直接复用现有实现

**关键代码**:
```cpp
int32_t DistributedClient::ConnectRemoteAbility(
    const OHOS::AAFwk::Want& want,
    const sptr<IRemoteObject>& connect)
{
    // 获取DMS Proxy
    sptr<IRemoteObject> remote = GetDmsProxy();

    // 序列化参数 (自动提取caller信息)
    MessageParcel data;
    WriteInfosToParcel(data, want, connect);

    // 调用DMS
    PARCEL_TRANSACT_SYNC_RET_INT(remote, CONNECT_REMOTE_ABILITY, data, reply);
}
```

#### 3.2.2 DMS通信流程

```
AbilityManagerService (源设备)
    │
    ├─→ GetDmsProxy() → 获取DMS系统服务 (DISTRIBUTED_SCHED_SA_ID)
    │
    ├─→ SendRequest(CONNECT_REMOTE_ABILITY)
    │       ├─ Want (包含deviceId, bundleName, abilityName)
    │       ├─ IAbilityConnection (回调对象)
    │       ├─ callerUid (自动提取)
    │       ├─ callerPid (自动提取)
    │       └─ accessToken (自动提取)
    │
    └─→ DMS → 路由到目标设备
            │
            └─→ 目标设备 AbilityManagerService
                    ├─→ 加载AppServiceExtensionAbility
                    ├─→ 调用 onConnect()
                    └─→ 返回 RemoteObject
```

---

## 四、安全性考虑

### 4.1 现有安全机制

系统已实现的安全检查:
1. **Permission Verification**: 权限验证框架
2. **Token-based Security**: accessToken/callerUid/callerPid 追踪
3. **Device ID Authentication**: 设备ID校验
4. **URI Permission Management**: URI权限管理

### 4.2 新增安全风险

**跨设备场景新增风险**:
1. **设备认证**: 需要确保目标设备可信
2. **数据传输**: 跨设备RPC调用需要加密
3. **权限传递**: 跨设备权限验证
4. **网络安全**: 分布式网络环境下的攻击面

### 4.3 安全增强方案

#### 4.3.1 设备白名单机制
```cpp
// 建议在AbilityManagerService中添加
bool AbilityManagerService::IsDeviceAllowedForConnection(const std::string& deviceId)
{
    // 调用安全团队提供的设备认证接口
    // 1. 检查设备是否在可信设备列表
    // 2. 验证设备组网状态
    // 3. 确认设备安全级别
    return SecurityManager::GetInstance().VerifyDevice(deviceId);
}
```

#### 4.3.2 权限传递增强
```cpp
// 在ConnectRemoteAbility前添加额外检查
int AbilityManagerService::ConnectRemoteAbility(
    const Want& want,
    const sptr<IRemoteObject>& callerToken,
    const sptr<IRemoteObject>& connect)
{
    // 1. 验证调用者权限
    if (!VerifyCrossDevicePermission()) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "No cross-device permission");
        return ERR_PERMISSION_DENIED;
    }

    // 2. 验证目标设备安全性
    std::string deviceId = want.GetElement().GetDeviceID();
    if (!IsDeviceAllowedForConnection(deviceId)) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "Device not allowed: %{public}s",
            AnonymizeDeviceId(deviceId).c_str());
        return ERR_DEVICE_NOT_ALLOWED;
    }

    // 3. 调用DMS
    return DistributedClient::ConnectRemoteAbility(want, connect);
}
```

### 4.4 需要安全团队评审的内容

1. **设备认证机制**: 当前DMS的设备认证是否足够
2. **权限传递模型**: 跨设备场景下权限如何传递和验证
3. **数据加密**: RPC调用是否需要应用层额外加密
4. **审计日志**: 跨设备调用是否需要特殊审计
5. **企业场景特殊要求**: IT设备管理的特殊安全需求

---

## 五、兼容性设计

### 5.1 向后兼容性

**原则**: 不影响现有本地连接功能

**实现方案**:
1. **按需触发**: 只有当 `want.GetElement().GetDeviceID()` 非空且不等于本地设备ID时,才走跨设备流程
2. **本地优先**: 默认行为保持不变
3. **显式指定**: 跨设备连接需要显式传入deviceId

**代码示例**:
```cpp
// 本地连接 (现有用法)
Want want;
want.SetElementName("com.example.app", "MyServiceExtension");
context->connectAppServiceExtensionAbility(want, options);

// 跨设备连接 (新用法)
Want want;
want.SetElementName("remote-device-id", "com.example.app", "MyServiceExtension");
context->connectAppServiceExtensionAbility(want, options);
```

### 5.2 版本兼容性

**场景**: 新版本连接旧版本设备

**处理方案**:
1. DMS层面会处理设备能力协商
2. 如果目标设备不支持,会返回错误码
3. 应用层需要处理错误并降级

**错误码处理**:
```cpp
// 建议在应用层 (HISMALL) 处理
try {
    context.connectAppServiceExtensionAbility(want, options);
} catch (error) {
    if (error.code === ERR_REMOTE_NOT_SUPPORTED) {
        // 降级处理: 使用本地校验或其他方案
        HandleFallback();
    }
}
```

### 5.3 API兼容性

**ETS/ArkTS层**: 无需修改,复用现有API
```typescript
// 现有API保持不变
connectAppServiceExtensionAbility(
    want: Want,
    options: ConnectOptions
): Promise<number>
```

**C++ Native层**: 无需修改接口签名
```cpp
// 现有接口
ErrCode ConnectAbilityWithExtensionType(
    const Want &want,
    const sptr<IAbilityConnection> &connect,
    const sptr<IRemoteObject> &callerToken,
    int32_t userId,
    AppExecFwk::ExtensionAbilityType extensionType
);
```

---

## 六、实现难度评估

### 6.1 改动复杂度

| 改动点 | 复杂度 | 工作量 | 风险 |
|--------|--------|--------|------|
| ConnectionManager 增强 | 低 | 0.5人天 | 低 |
| AbilityManagerService 路由逻辑 | 中 | 1人天 | 中 |
| 安全增强 | 高 | 2-3人天 | 高 |
| 测试用例 | 中 | 2人天 | - |
| 文档和代码审查 | 低 | 1人天 | - |
| **总计** | - | **6.5-7.5人天** | - |

### 6.2 技术风险

**低风险项**:
- ✅ DMS基础设施已完备,无需新建
- ✅ ServiceExtension已有完整参考实现
- ✅ 本地连接逻辑不受影响

**中风险项**:
- ⚠️ ConnectionManager的连接匹配逻辑复杂,改动需谨慎
- ⚠️ 需要确保所有代码路径都考虑了deviceId

**高风险项**:
- 🔴 安全机制需要安全团队深度评审
- 🔴 跨设备调用失败的处理策略
- 🔴 与DMS团队的对接和联调

### 6.3 依赖关系

**外部依赖**:
1. **DMS团队**:
   - 确认DMS对AppServiceExtension的支持
   - 提供设备认证建议
   - 联调测试支持

2. **安全团队**:
   - 评审安全方案
   - 提供设备认证接口规范
   - 审核权限传递机制

**内部依赖**:
1. **AbilityManagerService**: 主流程改动
2. **ConnectionManager**: 连接管理逻辑
3. **DistributedClient**: 已实现,无需改动

---

## 七、测试方案

### 7.1 单元测试

**文件**: `test/unittest/framework/native/ability_runtime/connection_manager_test.cpp`

**测试用例**:
```cpp
// 新增测试用例
TEST(ConnectionManager, MatchConnection_WithDifferentDeviceId)
{
    // 测试不同deviceId的连接不应复用
    ConnectionInfo info1("caller1", operation1, connection1, 100, "device1");
    ConnectionInfo info2("caller1", operation1, connection1, 100, "device2");

    EXPECT_FALSE(MatchConnection(info1, info2));
}

TEST(ConnectionManager, MatchConnection_WithSameDeviceId)
{
    // 测试相同deviceId的连接可以复用
    ConnectionInfo info1("caller1", operation1, connection1, 100, "device1");
    ConnectionInfo info2("caller1", operation1, connection1, 100, "device1");

    EXPECT_TRUE(MatchConnection(info1, info2));
}
```

### 7.2 集成测试

**文件**: `test/moduletest/services/abilitymgr/app_service_extension_cross_device_test.cpp`

**测试场景**:
1. **本地连接**: 验证现有功能不受影响
2. **跨设备连接**: 验证跨设备流程
3. **设备切换**: 验证从本地切换到跨设备
4. **错误处理**: 验证目标设备不存在/无权限等场景

**测试代码框架**:
```cpp
class AppServiceExtensionCrossDeviceTest : public testing::Test {
protected:
    void SetUp() override {
        // 启动本地AbilityManagerService
        // Mock DMS Proxy
    }

    void TearDown() override {
        // 清理
    }
};

TEST_F(AppServiceExtensionCrossDeviceTest, ConnectRemote_Success)
{
    // Given: 准备跨设备Want
    Want want;
    want.SetElementName("remote-device-id", "com.test.app", "TestExtension");

    // When: 连接远程Extension
    auto result = abilityMs_->ConnectAbility(want, connect, token, userId,
        AppExecFwk::ExtensionAbilityType::APP_SERVICE);

    // Then: 验证调用成功
    EXPECT_EQ(result, ERR_OK);
    // 验证调用了DMS
    EXPECT_TRUE(mockDms_->ConnectRemoteAbilityCalled());
}
```

### 7.3 端到端测试

**场景**: 模拟HISMALL完整流程

```
1. 源设备: 启动HISMALL
2. 源设备: 拉起数据克隆工具
3. 数据克隆工具: 完成设备建连和认证
4. HISMALL: 通过connectAppServiceExtensionAbility连接目标设备
5. HISMALL: 执行安全校验 (域账号验证)
6. 验证校验结果
7. 断开连接
```

### 7.4 性能测试

**指标**:
- 跨设备连接建立时间 (< 3秒)
- RPC调用延迟 (< 500ms)
- 并发连接数 (建议支持至少10个并发跨设备连接)

---

## 八、实施计划

### 8.1 开发阶段

**Phase 1: 核心功能开发 (1周)**
- [ ] Day 1-2: ConnectionManager 改动
- [ ] Day 3-4: AbilityManagerService 路由逻辑
- [ ] Day 5: 单元测试编写

**Phase 2: 安全增强 (1周)**
- [ ] Day 1-2: 与安全团队对接,确定方案
- [ ] Day 3-4: 实现安全检查逻辑
- [ ] Day 5: 安全测试

**Phase 3: 集成测试 (1周)**
- [ ] Day 1-2: 与DMS团队联调
- [ ] Day 3-4: 集成测试和端到端测试
- [ ] Day 5: 性能测试

**Phase 4: 代码审查和发布准备 (0.5周)**
- [ ] 代码审查
- [ ] 文档编写
- [ ] 发版准备

### 8.2 里程碑

| 阶段 | 里程碑 | 验收标准 |
|------|--------|----------|
| Phase 1 | 核心功能完成 | 单元测试通过,本地连接不受影响 |
| Phase 2 | 安全增强完成 | 安全团队评审通过,安全测试通过 |
| Phase 3 | 集成测试完成 | 端到端测试通过,DMS联调通过 |
| Phase 4 | 发版准备 | 代码审查通过,文档齐全 |

### 8.3 依赖协调

**关键路径**:
1. 先完成核心功能开发
2. 并行进行安全评审和DMS对接
3. 最后进行集成测试

**风险缓解**:
- 如果安全评审时间较长,可以先完成功能开发
- 提前与DMS团队预约联调时间
- 预留buffer时间处理意外问题

---

## 九、API使用示例

### 9.1 应用层使用 (HISMALL)

**ETS/ArkTS代码**:
```typescript
import { AbilityConstant, Want } from '@kit.AbilityKit';
import { BusinessError } from '@kit.BasicServicesKit';

export class DataCloneManager {
    // 场景: 在克隆开始前进行安全校验
    async performSecurityCheck(targetDeviceId: string): Promise<boolean> {
        try {
            // 1. 准备跨设备连接Want
            const want: Want = {
                deviceId: targetDeviceId,  // 指定目标设备
                bundleName: 'com.huawei.hismall',
                abilityName: 'SecurityCheckExtension',
                parameters: {
                    checkType: 'domain_account'  // 检查类型: 域账号
                }
            };

            // 2. 连接远程AppServiceExtension
            const connectionId = await this.context.connectAppServiceExtensionAbility(
                want,
                {
                    onConnect: (elementName, remoteObject) => {
                        console.info('Remote extension connected');
                        // 3. 执行RPC调用进行安全校验
                        this.performSecurityCheckRpc(remoteObject);
                    },
                    onDisconnect: (elementName) => {
                        console.info('Remote extension disconnected');
                    }
                }
            );

            return true;
        } catch (error) {
            console.error('Security check failed:', error);
            return false;
        }
    }

    private performSecurityCheckRpc(remoteObject: rpc.RemoteObject): void {
        // 通过RPC调用远程Extension的服务
        const data = new rpc.MessageParcel();
        const reply = new rpc.MessageParcel();
        // ... 封装参数并发送请求
    }
}
```

### 9.2 Extension实现 (目标设备)

**目标设备上的AppServiceExtensionAbility实现**:
```typescript
import { AppServiceExtensionAbility } from '@kit.AbilityKit';
import { rpc } from '@kit.ArkTS';

export default class SecurityCheckExtension extends AppServiceExtensionAbility {
    // 当远程设备连接时
    onConnect(want: Want): rpc.RemoteObject {
        console.info('SecurityCheckExtension onConnect from device:', want.deviceId);

        // 返回RemoteObject供RPC调用
        return new SecurityCheckServiceStub('security-check');
    }

    onDisconnect(want: Want) {
        console.info('SecurityCheckExtension onDisconnect');
    }
}

// RPC服务实现
class SecurityCheckServiceStub extends rpc.RemoteObject {
    onRemoteRequest(code: number, data: rpc.MessageParcel, reply: rpc.MessageParcel): boolean {
        // 处理安全校验请求
        if (code === SECURITY_CHECK_DOMAIN_ACCOUNT) {
            const domainAccount = data.readString();
            const isValid = this.verifyDomainAccount(domainAccount);
            reply.writeBoolean(isValid);
            return true;
        }
        return false;
    }

    private verifyDomainAccount(account: string): boolean {
        // 调用本地账号服务验证域账号
        return AccountManager.verifyDomainAccount(account);
    }
}
```

---

## 十、文档和宣传

### 10.1 开发者文档

**新增内容**:
1. API文档: 跨设备连接使用说明
2. 最佳实践: 跨设备调用注意事项
3. 错误处理: 常见错误码和处理方法
4. 安全指南: 企业场景安全要求

### 10.2 发布说明

**示例**:
```
## New Features

### Cross-device Support for AppServiceExtensionAbility

AppServiceExtensionAbility now supports connecting to extensions on remote devices,
enabling distributed scenarios like IT device data cloning.

**Usage:**
```typescript
const want: Want = {
    deviceId: 'target-device-id',  // Specify remote device
    bundleName: 'com.example.app',
    abilityName: 'MyExtension'
};
await context.connectAppServiceExtensionAbility(want, options);
```

**Requirements:**
- Devices must be in the same distributed network
- Requires GET_DISTRIBUTED_DEVICE_INFO permission
- Target device must support AppServiceExtensionAbility
```

---

## 十一、总结

### 11.1 方案优势

1. **最小化改动**: 复用现有DMS基础设施,改动点少且聚焦
2. **向后兼容**: 不影响现有本地连接功能
3. **参考实现**: ServiceExtension已有完整实现可参考
4. **安全可控**: 借鉴现有安全机制,新增风险可控

### 11.2 关键要点

1. **核心改动**: AbilityManagerService中添加跨设备路由逻辑
2. **依赖团队**: 需要DMS团队和安全团队配合
3. **测试重点**: 跨设备场景、错误处理、性能
4. **安全优先**: 必须通过安全团队评审

### 11.3 风险和建议

**主要风险**:
- 安全评审可能提出额外要求
- DMS团队联调时间可能较长
- 跨设备网络环境复杂

**建议**:
1. 尽早与安全团队和DMS团队对齐方案
2. 先完成核心功能,安全增强可以并行推进
3. 充分的测试覆盖,特别是错误场景
4. 预留充足的联调和调试时间

### 11.4 后续扩展

本方案实现后,可以支持更多分布式场景:
- 分布式数据同步
- 分布式任务协调
- 分布式服务调用
- 企业多设备协同

---

## 附录

### A. 参考文档
1. ServiceExtension跨设备实现: `services/abilitymgr/src/ability_manager_service.cpp:5366-5378`
2. DMS集成: `frameworks/native/ability/native/distributed_ability_runtime/distributed_client.cpp`
3. 连接管理: `frameworks/native/ability/ability_runtime/connection_manager.cpp`

### B. 关键代码位置
- **AbilityManagerService::ConnectAbility()**: `services/abilitymgr/src/ability_manager_service.cpp:5366-5406`
- **CheckIfOperateRemote()**: `services/abilitymgr/src/ability_manager_service.cpp`
- **DistributedClient::ConnectRemoteAbility()**: `frameworks/native/ability/native/distributed_ability_runtime/distributed_client.cpp:61-79`
- **ConnectionManager**: `frameworks/native/ability/ability_runtime/connection_manager.cpp`

### C. 联系人
- **需求方**: IT设备管理团队
- **DMS团队**: 分布式管理服务团队
- **安全团队**: 系统安全团队
- **实现团队**: Ability Runtime团队

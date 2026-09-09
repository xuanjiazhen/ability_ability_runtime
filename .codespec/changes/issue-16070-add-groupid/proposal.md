# Proposal: setAbilityInstanceInfo 新增必选 groupId 参数重载

## 原始需求

**需求来源**：窗口管理侧

**需求名称**：setAbilityInstanceInfo 接口新增 groupId 参数，透传给窗口 Session 用于实例分组

**需求背景**：
`setAbilityInstanceInfo` 当前仅接受 `label` 和 `icon` 两个参数，用于设置 Ability 实例的标签和图标。随着多实例分组能力的引入，窗口管理侧（Rosen）的 `ISession::SetSessionLabelAndIcon` 已扩展支持 `groupId` 参数，用于将 Ability 实例归入指定分组（如分屏组、悬浮窗组）。ability_runtime 需在公共 SDK 层新增带 `groupId` 的重载接口，将 groupId 透传到窗口 Session 侧。

**关键概念**：
- **groupId**：Ability 实例分组标识，字符串类型，最大长度 64 字节。由应用提供，经 ability_runtime 透传到 Rosen Session 层。空字符串 `""` 表示不分组（兼容原行为）。
- **setAbilityInstanceInfo**：Stage 模型公共 API，设置当前 Ability 实例的 label、icon 和 groupId。ETS 侧为 `UIAbilityContext` 接口，NAPI 侧为 `AbilityContext` 绑定。
- **SetSessionLabelAndIcon**：Rosen `ISession` 接口，窗口侧已支持 groupId 参数（跨仓依赖，窗口侧先行）。

**输入**：应用调用 `setAbilityInstanceInfo(label, icon, groupId)`

**处理步骤**：
1. ETS/NAPI/ANI 绑定层解析 groupId 参数，校验类型为 string 且长度 ≤ 64 字节
2. 校验通过后调用 C++ SDK `AbilityContextImpl::SetAbilityInstanceInfo(label, icon, groupId)`
3. C++ 实现透传 groupId 到 `ifaceSession->SetSessionLabelAndIcon(label, icon, groupId)`
4. 窗口侧返回 WSError，经 `TransferSetAbilityInstanceInfoErr` 映射为 ability_runtime 错误码

**输出**：窗口 Session 收到 groupId，Ability 实例被归入指定分组

---

## 需求基线

### 范围

- **包含**：
  - ETS 公共 API 新增 `setAbilityInstanceInfo(label, icon, groupId)` 重载，groupId 为必选参数
  - 保留原 `setAbilityInstanceInfo(label, icon)` 重载，内部传空串 `""` 以向后兼容
  - C++ SDK `AbilityContext::SetAbilityInstanceInfo` 签名变更为含 groupId 必选参数
  - NAPI 绑定层 `JsAbilityContext::OnSetAbilityInstanceInfo` 解析 groupId（可选，argc > 2 时取值）
  - ANI 绑定层 `EtsAbilityContext::SetAbilityInstanceInfo` / `SetAbilityInstanceInfoCheck` 签名新增 groupId 必选参数
  - NAPI AbilityContext 绑定 `ability_context.js` 签名变更为 `(label, icon, groupId)`
  - groupId 长度校验：NAPI/ANI 绑定层强制 ≤ 64 字节，超长抛参数错误
  - 单元测试更新适配新签名

- **排除**：
  - 修改窗口管理侧（Rosen）`ISession::SetSessionLabelAndIcon` 接口（跨仓依赖，窗口侧已先行支持）
  - 修改 `TransferSetAbilityInstanceInfoErr` 错误码映射逻辑（沿用现有）
  - 新增 groupId 专属错误码（groupId 校验失败复用 `ThrowInvalidParamError`）
  - 修改 hisysevent.yaml 事件定义
  - 修改 IPC Stub/Proxy 序列化格式（`SetSessionLabelAndIcon` 为跨仓 Rosen IPC，非本仓范围）

### 涉及子系统/仓

| 仓 | 子系统/路径 | 说明 |
|----|------------|------|
| ability_runtime | `interfaces/kits/native/ability/ability_runtime` | C++ SDK 接口声明（`ability_context.h`、`ability_context_impl.h`） |
| ability_runtime | `frameworks/native/ability/ability_runtime` | C++ 实现（`ability_context_impl.cpp`） |
| ability_runtime | `frameworks/native/ability/native/ability_runtime` | NAPI 绑定（`js_ability_context.cpp`） |
| ability_runtime | `frameworks/ets/ani/ui_ability` | ANI 绑定（`ets_ability_context.h`、`ets_ability_context.cpp`） |
| ability_runtime | `frameworks/ets/ets/application` | ETS 公共 API（`UIAbilityContext.ets`） |
| ability_runtime | `frameworks/js/napi/ability_context` | Stage 模型 NAPI AbilityContext 绑定（`ability_context.js`） |
| ability_runtime | `test/unittest/frameworks_kits_ability_ability_runtime_test` | 单元测试 |

### 复杂度级别

**标准** — 单仓（ability_runtime）特性，接口签名扩展 + 参数透传；无新 IPC 接口，无状态机，无安全/性能关键路径。跨仓依赖（Rosen `SetSessionLabelAndIcon`）已先行就绪。

---

## 验收标准 (P0)

### AC1: C++ SDK 接口签名含 groupId 必选参数
**WHEN** 查看 `ability_context.h` 和 `ability_context_impl.h` 中 `SetAbilityInstanceInfo` 声明
**THEN** 签名为 `SetAbilityInstanceInfo(const std::string& label, std::shared_ptr<OHOS::Media::PixelMap> icon, const std::string& groupId)`，groupId 无默认值

### AC2: C++ 实现透传 groupId 到窗口 Session
**WHEN** `AbilityContextImpl::SetAbilityInstanceInfo` 被调用
**THEN** 调用 `ifaceSession->SetSessionLabelAndIcon(label, icon, groupId)` 将 groupId 透传到 Rosen Session；SceneBoard 未启用返回 `ERR_CAPABILITY_NOT_SUPPORT`；session 为空返回 `ERR_INVALID_VALUE`

### AC3: ETS 公共 API 新增 groupId 必选参数重载
**WHEN** 查看 `UIAbilityContext.ets`
**THEN** 存在两个重载：`setAbilityInstanceInfo(label, icon)` 内部传 `""` 兼容旧调用；`setAbilityInstanceInfo(label, icon, groupId)` 传实际 groupId

### AC4: NAPI/ANI 绑定层校验 groupId 长度
**WHEN** 应用通过 JS/ETS 调用 `setAbilityInstanceInfo` 传入 groupId 长度 > 64 字节
**THEN** NAPI 层 `ThrowInvalidParamError(env, "Invalid groupId.")`；ANI 层 `EtsErrorUtil::ThrowInvalidParamError(env, "Invalid groupId.")`；不调用 C++ 实现

---

## 验收标准 (P1)

### AC5: 错误码映射保持一致
**WHEN** `SetSessionLabelAndIcon` 返回各种 WSError
**THEN** `TransferSetAbilityInstanceInfoErr` 映射不变：WS_OK→ERR_OK；WS_ERROR_INVALID_PERMISSION→CHECK_PERMISSION_FAILED；WS_ERROR_SET_SESSION_LABEL_FAILED→INVALID_PARAMETERS_ERR；WS_ERROR_DEVICE_NOT_SUPPORT→ERR_CAPABILITY_NOT_SUPPORT；default→static_cast

### AC6: 单元测试适配新签名
**WHEN** 执行 `ability_context_impl_second_test` 和 `ability_context_test`
**THEN** `SetAbilityInstanceInfo_0100` 三个子场景传入 `""` 作为 groupId；mock 类签名更新为含 groupId 参数；所有用例通过

### AC7: NAPI AbilityContext 绑定签名更新
**WHEN** 查看 `ability_context.js`
**THEN** `setAbilityInstanceInfo(label, icon, groupId)` 签名已更新（原签名 `(label, callback)` 已损坏，本次为修复）

---

## 不涉及项确认

| 维度 | 是否涉及 | 依据 |
|------|---------|------|
| 性能 | 不涉及 | 无新增计算/IO 瓶颈，仅参数透传 |
| 安全/权限 | 不涉及 | groupId 为技术标识符，无安全语义；权限校验沿用 SET_ABILITY_INSTANCE_INFO，在 session 侧校验 |
| 兼容性 | 是 | ETS 向后兼容（保留原重载）；C++ SDK 为 breaking change（新增必选参数）；NAPI AbilityContext 原签名已损坏 |
| API/SDK | 是 | ETS Public API 新增重载；C++ Inner API 签名变更 |
| IPC/跨进程 | 不涉及 | `SetSessionLabelAndIcon` 为跨仓 Rosen IPC，窗口侧已先行支持，非本次变更范围 |
| 构建/组件 | 不涉及 | 无新增源文件，无 BUILD.gn 变更 |
| 国际化/无障碍 | 不涉及 | 无 UI 文案 |
| 数据迁移 | 不涉及 | 无持久化数据格式变更 |
| DFX | 不涉及 | 无 hisysevent.yaml 事件变更，沿用现有日志和错误码映射 |

---

## Agent Scope Guard

| 维度 | 范围/限制 | 需人工确认的触发条件 |
|------|-----------|----------------------|
| 允许仓库 | ability_runtime | 需修改 window_manager 仓时先确认 |
| 允许模块/目录 | `interfaces/kits/native/ability/`、`frameworks/native/ability/`、`frameworks/ets/`、`frameworks/js/napi/ability_context/`、`test/unittest/` | 超出时先确认 |
| 禁止修改项 | Rosen `ISession::SetSessionLabelAndIcon` 接口；`TransferSetAbilityInstanceInfoErr` 映射逻辑；hisysevent.yaml | 需变更时先评估 |
| 外部依赖/网络访问 | 无 | 无网络访问 |

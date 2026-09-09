# Spec: setAbilityInstanceInfo 新增必选 groupId 参数重载

## 1. 特性概述

为 `setAbilityInstanceInfo` 公共 API 新增带 `groupId` 必选参数的重载，使应用可将 Ability 实例归入指定分组。原无 groupId 的重载保留，行为等价于 groupId 为空串。groupId 最大长度 64 字节，超长时 API 同步抛出 401。

## 2. 接口签名

### 2.1 ETS 公共 API（Stage 模型，Public）

```
setAbilityInstanceInfo(label: string, icon: image.PixelMap, groupId: string): Promise<void>
setAbilityInstanceInfo(label: string, icon: image.PixelMap): Promise<void>
```

- 第一重载：groupId 必选，用于实例分组
- 第二重载：保留向后兼容，等价于 groupId=""

### 2.2 C++ Inner API（InnerAPI）

```
ErrCode SetAbilityInstanceInfo(const std::string& label,
    std::shared_ptr<OHOS::Media::PixelMap> icon, const std::string& groupId);
```

- groupId 必选（无默认值）。**Breaking change**

### 2.3 Stage 模型 NAPI AbilityContext（Public）

```
setAbilityInstanceInfo(label, icon, groupId)
```

- 从 `(label, callback)` 变更为 `(label, icon, groupId)`。**Breaking change**

## 3. 验收标准 (AC)

| AC | 优先级 | 变更 | GIVEN | WHEN | THEN |
|----|--------|------|-------|------|------|
| AC1 | P0 | 新增 | 设备支持 SceneBoard 且 session 有效 | 调用 `setAbilityInstanceInfo(label, icon, groupId)` 传入合法 groupId（≤ 64 字节） | Promise resolve |
| AC2 | P0 | 新增 | 设备支持 SceneBoard 且 session 有效 | 调用 `setAbilityInstanceInfo(label, icon)` 不传 groupId | Promise resolve，等价于 groupId="" |
| AC3 | P0 | 新增 | 应用通过 ETS 或 JS 调用 | 传入 groupId 长度 > 64 字节 | 同步抛出 401；不执行设置操作 |
| AC4 | P0 | 未变 | 设备不支持 SceneBoard | 调用任一重载 | Promise reject 801 |
| AC5 | P0 | 未变 | session token 为空或已失效 | 调用 `setAbilityInstanceInfo(label, icon, groupId)` 且设备支持 SceneBoard | Promise reject 16000050 |
| AC6 | P0 | 未变 | 窗口侧返回权限校验失败 | 调用 `setAbilityInstanceInfo(label, icon, groupId)` 且 SceneBoard 启用且 session 有效 | Promise reject 201 |
| AC7 | P1 | 新增 | 应用通过 NAPI 调用，第三参数为非 string 类型 | 调用 `setAbilityInstanceInfo(label, icon, groupId)` 且设备支持 SceneBoard | groupId 降级为 ""，Promise resolve |
| AC8 | P0 | 未变 | 窗口侧返回设置失败 | 调用 `setAbilityInstanceInfo(label, icon, groupId)` 且 SceneBoard 启用且 session 有效 | Promise reject 401 |
| AC9 | P0 | 未变 | context 为空或已释放 | 调用 `setAbilityInstanceInfo(label, icon, groupId)` | Promise reject 16000011 |

## 4. 测试场景

### 4.1 单元测试

| # | 场景 | 输入 | 预期结果 | 关联 AC |
|---|------|------|----------|---------|
| T1 | 有效 session + 非空 groupId | groupId="valid_group"，SceneBoard 启用，session 有效 | Promise resolve | AC1 |
| T2 | 有效 session + 空 groupId | groupId=""，SceneBoard 启用，session 有效 | Promise resolve | AC2 |
| T3 | groupId 恰好 64 字节 | groupId="x"×64 | Promise resolve | AC1 |
| T4 | groupId 超过 64 字节（NAPI） | groupId="x"×65 | 同步抛出 401 | AC3 |
| T5 | groupId 超过 64 字节（ANI） | groupId="x"×65 | 同步抛出 401 | AC3 |
| T6 | groupId 为非 string（NAPI） | 第三参数为 number | groupId 降级为 ""，Promise resolve | AC7 |
| T7 | SceneBoard 未启用 | groupId=""，SceneBoard 禁用 | Promise reject 801 | AC4 |
| T8 | session 为空 | groupId=""，SceneBoard 启用，session 为空 | Promise reject 16000050 | AC5 |
| T9 | context 为空 | context 已释放 | Promise reject 16000011 | AC9 |
| T10 | 窗口侧返回权限失败 | session 返回权限失败错误 | Promise reject 201 | AC6 |
| T11 | 窗口侧返回设备不支持 | session 返回设备不支持错误 | Promise reject 801 | AC4 |
| T12 | 窗口侧返回设置失败 | session 返回设置失败错误 | Promise reject 401 | AC8 |

### 4.2 兼容性回归测试

| # | 场景 | 预期结果 |
|---|------|----------|
| R1 | ETS 调用 `setAbilityInstanceInfo(label, icon)` 不传 groupId | Promise resolve，等价于 groupId="" |
| R2 | NAPI 调用仅传 2 参数 | groupId 默认 ""，Promise resolve |
| R3 | `ability_context_impl_second_test` 的 `SetAbilityInstanceInfo_0100` 和 `TransferSetAbilityInstanceInfoErr_0100` | 全部用例通过，行为不变 |

## 5. 错误码定义

| 错误码 | 值 | 含义 | 触发条件 | 错误形式 | 关联 AC |
|--------|-----|------|----------|----------|---------|
| ERROR_OK | 0 | 成功 | 设置成功 | Promise resolve | AC1, AC2 |
| ERROR_CODE_INVALID_PARAM | 401 | 参数无效 | groupId 长度 > 64 字节；label/icon 解析失败；窗口侧设置失败 | 同步抛出 / Promise reject | AC3, AC8 |
| ERROR_CODE_PERMISSION_DENIED | 201 | 权限不足 | 窗口侧权限校验未通过 | Promise reject | AC6 |
| ERROR_CODE_CAPABILITY_NOT_SUPPORT | 801 | 能力不支持 | SceneBoard 未启用 | Promise reject | AC4 |
| ERROR_CODE_INVALID_CONTEXT | 16000011 | context 无效 | context 为空或已释放 | Promise reject | AC9 |
| ERROR_CODE_INNER | 16000050 | 内部错误 | session 无效或其他未映射错误 | Promise reject | AC5 |

### 5.1 错误码约束

| 约束 | 说明 |
|------|------|
| 不新增错误码 | 本次变更复用既有错误码，不为 groupId 引入专属错误码 |
| groupId 校验失败固定 401 | NAPI 和 ANI 绑定层均同步抛出 401，不调用 C++ 实现，不触发 Promise reject |
| 错误码数值不可变 | `ability_business_error.h` 中上述错误码为已发布公共合约，不可修改数值 |
| 401 双路径 | groupId 超长：调用后立即同步抛出 401；窗口侧设置失败：Promise reject 401。值相同，错误形式不同 |

## 6. 兼容性声明

| API | 当前级别 | 目标级别 | 变更类型 |
|-----|----------|----------|----------|
| ETS `setAbilityInstanceInfo(label, icon)` | Public | Public | 保留，行为等价于 groupId="" |
| ETS `setAbilityInstanceInfo(label, icon, groupId)` | — | Public | 新增重载 |
| C++ `SetAbilityInstanceInfo(label, icon, groupId)` | InnerAPI | InnerAPI | 签名变更（新增必选参数），breaking |
| NAPI `setAbilityInstanceInfo(label, icon, groupId)` | Public | Public | 签名变更（从 `(label, callback)`），breaking |

- **配置文件格式变更:** 否
- **数据存储格式变更:** 否

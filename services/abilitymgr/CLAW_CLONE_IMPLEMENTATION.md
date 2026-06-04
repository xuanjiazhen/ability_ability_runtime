# CLI启动沙箱分身应用实现说明

## 概述

本文档描述了支持CLI通过ohos-aa命令启动沙箱分身应用的实现方案。该实现允许用户使用`--claw-index`参数指定要启动的分身应用索引。

## 功能特性

1. **权限检查**：根据调用方是否有`MANAGE_SANDBOX_APP`权限，决定是否信任命令行传入的创建方包名
2. **创建方包名确定**：
   - 有权限：使用CLI命令传入的创建方包名
   - 无权限：使用调用方自己的包名
3. **BMS查询**：向BMS查询分身Ability信息
4. **无缝集成**：与现有的分身应用启动流程无缝集成

## 架构设计

### 处理流程

```
用户执行ohos-aa命令
    ↓
CLI解析--claw-index和--creator-bundle参数
    ↓
调用AMS的StartAbility
    ↓
AMS处理流程:
    1. 检测clawIndex参数
    2. 调用ProcessSandboxCloneLaunch
        ├─ GetCreatorBundleNameForSandboxClone (权限检查)
        ├─ QueryCloneAbilityInfo (BMS查询)
        └─ 更新Want信息
    3. 走已有的启动分身流程
```

### 核心组件

#### 1. CLI参数解析 (ohos_aa_command.cpp)

```cpp
// 新增选项
enum OptionType {
    OPTION_CLAW_INDEX,      // Claw sandbox index for clone application
    OPTION_CREATOR_BUNDLE    // Creator bundle name (untrusted, from command line)
};

// 解析clawIndex
case OPTION_CLAW_INDEX: {
    if (optarg != nullptr) {
        std::string clawIndexStr = optarg;
        if (!std::regex_match(clawIndexStr, std::regex(STRING_TEST_REGEX_INTEGER_NUMBERS))) {
            TAG_LOGE(AAFwkTag::AA_TOOL, "invalid claw-index: %{public}s", clawIndexStr.c_str());
            result = OHOS::ERR_INVALID_VALUE;
            break;
        }
        clawIndex = std::stoi(clawIndexStr);
        hasClawIndex = true;
    }
    break;
}

// 解析creatorBundleName
case OPTION_CREATOR_BUNDLE: {
    if (optarg != nullptr) {
        creatorBundleName = optarg;
    }
    break;
}
```

#### 2. AMS处理函数 (ability_manager_service.cpp)

##### ProcessSandboxCloneLaunch

主处理函数，负责协调整个分身应用启动流程：

```cpp
int32_t AbilityManagerService::ProcessSandboxCloneLaunch(Want &want,
    const sptr<IRemoteObject> &callerToken, int32_t userId)
```

**处理步骤：**
1. 检测是否有`PARAM_APP_CLONE_INDEX_KEY`参数
2. 验证cloneIndex范围
3. 调用`GetCreatorBundleNameForSandboxClone`获取创建方包名
4. 调用`QueryCloneAbilityInfo`查询分身Ability信息
5. 更新Want中的module name等信息

##### GetCreatorBundleNameForSandboxClone

根据权限确定创建方包名：

```cpp
std::string AbilityManagerService::GetCreatorBundleNameForSandboxClone(const Want &want,
    const sptr<IRemoteObject> &callerToken, int32_t &errCode)
```

**权限逻辑：**
```cpp
// 检查是否有MANAGE_SANDBOX_APP权限
bool hasManageSandboxPermission = Permission::PermissionKit::CheckPermission(
    PERMISSION_MANAGE_SANDBOX_APP, callerTokenId) == Permission::PermissionState::PERMISSION_GRANTED;

if (hasManageSandboxPermission) {
    // 有权限：使用CLI传入的创建方包名
    creatorBundleName = inputCreatorBundleName;
} else {
    // 无权限：使用调用方自己的包名
    creatorBundleName = callerBundleName;
}
```

##### QueryCloneAbilityInfo

向BMS查询分身Ability信息：

```cpp
int32_t AbilityManagerService::QueryCloneAbilityInfo(const std::string &creatorBundleName,
    const std::string &bundleName, const std::string &abilityName, int32_t cloneIndex,
    int32_t userId, AppExecFwk::AbilityInfo &abilityInfo)
```

**BMS接口调用：**
```cpp
AppExecFwk::ElementName element(bundleName, abilityName);
auto abilityInfoFlag = static_cast<int32_t>(AppExecFwk::GetAbilityInfoFlag::GET_ABILITY_INFO_WITH_APPLICATION |
    AppExecFwk::GetAbilityInfoFlag::GET_ABILITY_INFO_WITH_METADATA);

bms->QueryCloneAbilityInfo(creatorBundleName, element, abilityInfoFlag, cloneIndex, abilityInfo, userId);
```

## 使用示例

### 启动分身应用

```bash
# 启动分身索引为1的应用
ohos-aa start --abilityname EntryAbility --bundlename com.example.app --claw-index 1

# 使用指定的创建方包名（需要MANAGE_SANDBOX_APP权限）
ohos-aa start --abilityname EntryAbility --bundlename com.example.app --claw-index 2 --creator-bundle com.creator.app
```

### 环境变量

CLI工具通过环境变量传递可信的调用方包名：

```bash
export ohos.cli.callerBundleName=com.caller.app
```

## 错误处理

| 错误码 | 描述 |
|-------|------|
| ERR_APP_CLONE_INDEX_INVALID | 无效的分身索引 |
| ERR_INVALID_VALUE | 创建方包名获取失败 |
| GET_ABILITY_SERVICE_FAILED | BMS查询失败 |

## 权限要求

| 权限 | 用途 |
|------|------|
| ohos.permission.MANAGE_CLI_SANDBOX_BUNDLE | 管理沙箱应用，允许指定创建方包名 |

## 文件修改清单

### 修改的文件

1. **tools/ohos-aa/src/ohos_aa_command.cpp**
   - 添加`--claw-index`和`--creator-bundle`选项解析
   - 将参数传递到Want中

2. **tools/ohos-aa/include/ohos_aa_command.h**
   - 添加新的选项枚举值

3. **services/abilitymgr/include/ability_manager_service.h**
   - 添加函数声明

4. **services/abilitymgr/src/ability_manager_service.cpp**
   - 实现核心处理函数
   - 在StartAbilityInner中集成处理流程

### 依赖的BMS接口

**注意：BMS内部C++接口待定**

目前JavaScript接口为`getSandboxBundleInfo(bundleName: string, bundleFlags: int, appIndex?: int)`，但BMS内部C++接口还未确定。

当前实现使用假设的接口签名：

```cpp
// QueryCloneAbilityInfo - 查询分身Ability信息（假设的接口，待BMS确定后更新）
// 参数:
//   - creatorBundleName: 创建方包名（用于权限验证和查询）
//   - element: 目标Ability的ElementName（包含bundleName和abilityName）
//   - abilityInfoFlag: 查询标志（GET_ABILITY_INFO_WITH_APPLICATION | GET_ABILITY_INFO_WITH_METADATA）
//   - cloneIndex: 分身索引
//   - abilityInfo: 输出的Ability信息
//   - userId: 用户ID
//
// 注意：此接口签名是假设的，实际实现需要等BMS确定内部C++接口后更新
int32_t QueryCloneAbilityInfo(const std::string &creatorBundleName,
    const ElementName &element, int32_t abilityInfoFlag, int32_t cloneIndex,
    AbilityInfo &abilityInfo, int32_t userId);
```

**实现说明：**
- 当前代码中`QueryCloneAbilityInfo`函数是占位实现
- 等BMS确定内部C++接口后，需要更新该函数的实现
- 可能需要使用`GetSandboxAbilityInfo`、`GetSandboxBundleInfo`或其他新增接口

## 测试要点

1. **权限测试**
   - 有MANAGE_SANDBOX_APP权限时验证creator-bundle参数生效
   - 无权限时验证使用调用方包名

2. **边界测试**
   - clawIndex为0、最大值、负数、超范围值
   - 空包名、无效包名

3. **功能测试**
   - 正常启动分身应用
   - 多个分身应用独立运行
   - 与普通应用启动互不影响

## 注意事项

1. **安全性**：无权限时使用调用方包名，防止恶意应用指定创建方
2. **兼容性**：与现有的分身应用启动流程完全兼容
3. **性能**：权限检查和BMS查询均采用高效实现

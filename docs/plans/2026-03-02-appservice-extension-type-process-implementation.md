# AppServiceExtension TYPE 模式支持实施计划

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** 为 AppServiceExtensionAbility 添加对 ExtensionProcessMode::TYPE 模式的支持，使每个 AppServiceExtensionAbility 运行在独立的进程中，进程名称格式为 bundleName:abilityName。

**Architecture:** 最小改动方案，复用现有 TYPE 模式处理逻辑，在 extension_record_factory.cpp 中添加 APP_SERVICE 配置项，在 MakeProcessName 中添加进程名生成逻辑。

**Tech Stack:** C++, GN build system, OpenHarmony Ability Runtime framework

---

### Task 1: 确认 ExtensionAbilityType 枚举值

**Files:**
- Read: `interfaces/kits/native/.../ability_info.h`
- Search: ExtensionAbilityType definition

**Step 1: 查找 AppServiceExtension 的枚举值**

```bash
# 查找 ExtensionAbilityType 枚举中 AppServiceExtension 的值
grep -r "ExtensionAbilityType.*=" interfaces/kits/native/ | grep -i service
```

Expected output: 应找到类似 `ExtensionAbilityType::SERVICE` 或 `ExtensionAbilityType::APP_SERVICE` 的定义

**Step 2: 确认枚举值**

Expected: `ExtensionAbilityType::SERVICE` (用于 AppServiceExtension)

---

### Task 2: 添加 APP_SERVICE 配置到 EXTENSION_RECORD_CONFIG_MAP

**Files:**
- Modify: `services/abilitymgr/src/extension_record/extension_record_factory.cpp:28-48`

**Step 1: 添加配置条目**

在 `EXTENSION_RECORD_CONFIG_MAP` 中添加：

```cpp
{ AppExecFwk::ExtensionAbilityType::SERVICE,
  { PROCESS_MODE_BUNDLE, PROCESS_MODE_SUPPORT_DEFAULT | PROCESS_MODE_TYPE,
    PRE_CHECK_FLAG_NONE }},
```

位置：在 `EMBEDDED_UI` 条目之后，在现有配置项的末尾之前。

**Step 2: 编译验证**

```bash
cd /path/to/OpenHarmony
./build.sh --product-name <product> --build-target abilityms
```

Expected: 编译成功，无语法错误

**Step 3: Commit**

```bash
git add services/abilitymgr/src/extension_record/extension_record_factory.cpp
git commit -m "feat: 为 AppServiceExtension 添加 TYPE 模式支持配置

在 EXTENSION_RECORD_CONFIG_MAP 中添加 SERVICE 类型的配置项，
支持 ExtensionProcessMode::TYPE 模式。

Co-Authored-By: Claude Sonnet 4.5 <noreply@anthropic.com>"
```

---

### Task 3: 确认 extensionProcessMode 传递路径

**Files:**
- Read: `services/abilitymgr/src/ability_connect_manager.cpp:1240-1255`
- Trace: LoadParam 结构体

**Step 1: 查看 LoadParam 结构体**

查找 LoadParam 中 extensionProcessMode 字段如何被使用。

**Step 2: 确认数据流**

从 `AbilityRequest.extensionProcessMode` → `ExtensionRecord.processMode_` → `LoadParam.extensionProcessMode` → `AppMgrService`

Expected: 确认 extensionProcessMode 正确传递到进程名生成逻辑

---

### Task 4: 在 MakeProcessName 中添加 TYPE 模式处理

**Files:**
- Modify: `services/appmgr/src/app_mgr_service_inner.cpp:1440-1475`
- Read: `services/appmgr/include/app_mgr_service_inner.h` (查找 MakeProcessName 声明)

**Step 1: 添加 APP_SERVICE 类型处理逻辑**

在 `MakeServiceExtProcessName()` 函数之前或之后添加处理逻辑，或在该函数内部添加：

```cpp
// 在 MakeServiceExtProcessName() 函数内或之后添加
if (abilityInfo->extensionAbilityType == ExtensionAbilityType::SERVICE &&
    hapModuleInfo.isStageBasedModel && hapModuleInfo.extensionProcessMode == ExtensionProcessMode::TYPE) {
    processName = appInfo->bundleName;
    processName += ":";
    processName += abilityInfo->name;
    TAG_LOGI(AAFwkTag::APPMGR,
        "APP_SERVICE TYPE mode, processName: %{public}s", processName.c_str());
    return;
}
```

**Step 2: 编译验证**

```bash
./build.sh --product-name <product> --build-target ams_target
```

Expected: 编译成功

**Step 3: Commit**

```bash
git add services/appmgr/src/app_mgr_service_inner.cpp
git commit -m "feat: 为 APP_SERVICE TYPE 模式生成独立进程名

当 extensionProcessMode 为 TYPE 时，生成格式为 bundleName:abilityName
的进程名，使每个 AppServiceExtensionAbility 运行在独立进程中。

Co-Authored-By: Claude Sonnet 4.5 <noreply@anthropic.com>"
```

---

### Task 5: 添加单元测试验证配置

**Files:**
- Modify: `test/unittest/extension_record_factory_test/extension_record_factory_test.cpp`
- Create: `test/unittest/extension_record_factory_test/extension_record_factory_test_type_mode.cpp` (如需要)

**Step 1: 编写测试用例**

```cpp
/**
 * @tc.number: AppServiceExtension_ExtensionProcessMode_TYPE_0100
 * @tc.name: AppServiceExtension支持TYPE模式
 * @tc.desc: 验证AppServiceExtension支持TYPE模式的进程配置
 * @tc.type: FUNC
 * @tc.require: AR600
 * @tc.author: zhang_hao_zheng
 */
HWTEST_F(ExtensionRecordFactoryTest, AppServiceExtension_ExtensionProcessMode_TYPE_0100)
{
    AAFwk::AbilityRequest request;
    request.extensionType = AppExecFwk::ExtensionAbilityType::SERVICE;
    request.extensionProcessMode = AppExecFwk::ExtensionProcessMode::TYPE;

    bool isHostSpecified = false;
    uint32_t processMode = factory_->GetExtensionProcessMode(request, isHostSpecified);

    EXPECT_EQ(processMode, ExtensionRecordFactory::PROCESS_MODE_TYPE);
    EXPECT_FALSE(isHostSpecified);
}
```

**Step 2: 运行测试验证**

```bash
./build.sh --product-name <product> --test-component ability_runtime --test-case extension_record_factory_test
```

Expected: 新测试用例通过

**Step 3: Commit**

```bash
git add test/unittest/extension_record_factory_test/extension_record_factory_test.cpp
git commit -m "test: 添加 AppServiceExtension TYPE 模式测试用例

验证当 extensionProcessMode 设置为 TYPE 时，返回正确的
PROCESS_MODE_TYPE 值。

Co-Authored-By: Claude Sonnet 4.5 <noreply@anthropic.com>"
```

---

### Task 6: 添加单元测试验证进程名生成

**Files:**
- Modify: `test/unittest/app_mgr_service_inner_test/app_mgr_service_inner_test.cpp`

**Step 1: 编写进程名生成测试用例**

```cpp
/**
 * @tc.number: AppServiceExtension_ProcessName_TYPE_0100
 * @tc.name: TYPE模式下生成正确进程名
 * @tc.desc: 验证TYPE模式下生成的进程名格式为bundleName:abilityName
 * @tc.type: FUNC
 * @tc.require: AR600
 * @tc.author: zhang_hao_zheng
 */
HWTEST_F(AppMgrServiceInnerTest, AppServiceExtension_ProcessName_TYPE_0100)
{
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "TestService";
    abilityInfo->extensionAbilityType = ExtensionAbilityType::SERVICE;

    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->bundleName = "com.example.app";

    HapModuleInfo hapModuleInfo;
    hapModuleInfo.isStageBasedModel = true;
    hapModuleInfo.extensionProcessMode = ExtensionProcessMode::TYPE;

    std::string processName;
    std::string specifiedProcessFlag = "";

    serviceInner_->MakeProcessName(abilityInfo, appInfo, hapModuleInfo, 0, specifiedProcessFlag, processName, false);

    EXPECT_EQ(processName, "com.example.app:TestService");
}
```

**Step 2: 运行测试验证**

```bash
./build.sh --product-name <product> --test-component ability_runtime --test-case app_mgr_service_inner_test
```

Expected: 测试通过

**Step 3: Commit**

```bash
git add test/unittest/app_mgr_service_inner_test/app_mgr_service_inner_test.cpp
git commit -m "test: 添加 APP_SERVICE TYPE 模式进程名测试

验证 TYPE 模式下生成的进程名格式正确。

Co-Authored-By: Claude Sonnet 4.5 <noreply@anthropic.com>"
```

---

### Task 7: Demo 应用验证

**Files:**
- Create: demo_app_service_extension/AppServiceExtension1/
- Create: demo_app_service_extension/AppServiceExtension2/

**Step 1: 创建第一个 Service Extension**

创建 `AppServiceExtension1` 目录和配置：

```json5
{
  "module": {
    "name": "entry",
    "type": "entry",
    "extensionAbilities": [{
      "name": "AppServiceExtension1",
      "srcEntrance": "./ets/AppServiceExtension1.ets",
      "description": "Test AppServiceExtension 1",
      "type": "appService",
      "extensionProcessMode": "type"
    }]
  }
}
```

**Step 2: 创建第二个 Service Extension**

创建 `AppServiceExtension2`，配置类似，`extensionProcessMode` 为 `type`。

**Step 3: 编译安装 Demo**

```bash
hvigorw assembleHap --mode module -p product=default
hdc install entry-default.hap
```

**Step 4: 启动并验证进程**

```bash
# 启动两个 service extension
aa start -a AppServiceExtension1 -b com.example.demo
aa start -a AppServiceExtension2 -b com.example.demo

# 查看进程
ps -ef | grep "com.example.demo"
```

Expected: 看到 `com.example.demo:AppServiceExtension1` 和 `com.example.demo:AppServiceExtension2` 两个独立进程

**Step 5: 提交 Demo (可选)**

```bash
git add demo_app_service_extension/
git commit -m "test: 添加 AppServiceExtension TYPE 模式 Demo

创建两个配置了 extensionProcessMode 为 type 的 AppServiceExtension
用于验证独立进程功能。

Co-Authored-By: Claude Sonnet 4.5 <noreply@anthropic.com>"
```

---

### Task 8: 集成测试

**Files:**
- Run: All existing unit tests
- Run: New unit tests

**Step 1: 运行所有相关测试**

```bash
# 运行 extension_record_factory_test
./build.sh --product-name <product> --test-component ability_runtime --test-case extension_record_factory_test

# 运行 app_mgr_service_inner_test
./build.sh --product-name <product> --test-component ability_runtime --test-case app_mgr_service_inner_test
```

Expected: 所有测试通过，无回归

**Step 2: 验证兼容性**

创建一个没有设置 `extensionProcessMode` 的 AppServiceExtension，确认它仍然使用 BUNDLE 模式。

**Step 3: 提交最终文档**

```bash
git add docs/plans/2026-03-02-appservice-extension-type-process-design.md
git commit -m "docs: 添加设计文档到版本控制"
```

---

## 成功标准检查清单

- [ ] Task 1: 确认了 ExtensionAbilityType 枚举值
- [ ] Task 2: 添加了 APP_SERVICE 配置到 EXTENSION_RECORD_CONFIG_MAP
- [ ] Task 3: 确认了 extensionProcessMode 传递路径
- [ ] Task 4: 在 MakeProcessName 中添加了 TYPE 模式处理
- [ ] Task 5: 添加了单元测试验证配置
- [ ] Task 6: 添加了单元测试验证进程名生成
- [ ] Task 7: Demo 验证成功（进程名格式正确）
- [ ] Task 8: 所有单元测试通过
- [ ] 现有功能无回归

## 参考

- 设计文档: `docs/plans/2026-03-02-appservice-extension-type-process-design.md`
- Issue: #14623
- ExtensionProcessMode 定义: `test/new_test/mock/extension_ability_info/extension_ability_info.h:116`
- 配置映射: `services/abilitymgr/src/extension_record/extension_record_factory.cpp:28-48`
- 进程名生成: `services/appmgr/src/app_mgr_service_inner.cpp:1440-1475`

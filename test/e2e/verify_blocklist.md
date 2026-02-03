# Extension Blocklist E2E Test Guide

## 快速验证步骤

### 1. 修改黑名单配置（临时测试）

```bash
# 编辑配置文件
vim frameworks/native/ability/native/etc/extension_blocklist_config.json
```

在 FormExtension 的黑名单中添加一个测试模块：
```json
{
  "blocklist": {
    "FormExtension": [
      "ability.particleAbility",
      "test.e2e.blockedModule"  // 添加这一行
    ]
  }
}
```

### 2. 创建测试Extension

创建目录结构：
```
test/e2e/BlocklistTestExtension/
├── src/main/
│   ├── ets/
│   │   └── TestFormExtension.ets
│   └── module.json5
└── BUILD.gn
```

**TestFormExtension.ets:**
```typescript
import FormExtension from '@ohos.app.ability.FormExtension';
import formBindingData from '@ohos.app.form.formBindingData';
import FormInfo from '@ohos.app.form.formInfo';

const TAG = '[BlocklistTest]';

export default class TestFormExtension extends FormExtension {
    onCreate(want: Want) {
        console.info(TAG, 'onCreate called');

        // 测试1: 尝试使用被block的模块
        this.testBlockedModule();

        // 测试2: 尝试使用正常的模块
        this.testAllowedModule();

        return formBindingData.createFormBindingData({
            testName: "BlocklistTest"
        });
    }

    private testBlockedModule() {
        try {
            // 这个模块在黑名单中，应该被拦截
            let particleAbility = requireNapi('ability.particleAbility');
            console.error(TAG, 'ERROR: Blocked module was loaded!');
        } catch (error) {
            console.info(TAG, 'SUCCESS: Blocked module was intercepted');
            console.info(TAG, 'Error: ' + error.message);
        }
    }

    private testAllowedModule() {
        try {
            // 这个模块不在黑名单中，应该正常加载
            let app = requireNapi('application');
            console.info(TAG, 'SUCCESS: Allowed module loaded');
        } catch (error) {
            console.error(TAG, 'ERROR: Allowed module was blocked!');
        }
    }

    onCastToNormal() {
        console.info(TAG, 'onCastToNormal');
    }

    onUpdate(formId: string) {
        console.info(TAG, 'onUpdate');
    }

    onDestroy(formId: string) {
        console.info(TAG, 'onDestroy');
    }
}
```

**module.json5:**
```json
{
  "module": {
    "name": "BlocklistTestExtension",
    "type": "entry",
    "description": "$string:module_desc",
    "mainElement": "TestFormExtension",
    "deviceTypes": ["default", "tablet"],
    "deliveryWithInstall": true,
    "installationFree": false,
    "pages": {
      "pages": "test/pages/Index"
    },
    "uiSyntax": "ets",
    "abilities": [],
    "extensionAbilities": [{
      "name": "TestFormExtension",
      "srcEntry": "./ets/TestFormExtension.ets",
      "description": "$string:extension_desc",
      "type": "form",
      "permissions": []
    }]
  }
}
```

**BUILD.gn:**
```gn
import("//build/ohos.gni")
import("//build/test.gni")

group("blocklist_e2e_test") {
  testonly = true
  deps = [ ":blocklist_test_extension" ]
}

ohos_hap("blocklist_test_extension") {
  hap_profile = "src/main/module.json5"
  deps = []
  ets2abc = true
  subsystem_name = "ability"
  part_name = "ability_runtime"
  testonly = true
}
```

### 3. 编译并安装

```bash
# 编译测试Extension
./build.sh --product-name <your_product> --build-target blocklist_test_extension

# 安装到设备
hdc install out/<product>/tests/e2e/blocklist_test_extension.hap

# 查看日志
hdc shell hilog -T BlocklistTest
```

### 4. 预期结果

**成功的情况（Block功能正常）：**
```
[BlocklistTest] onCreate called
[BlocklistTest] SUCCESS: Blocked module was intercepted
[BlocklistTest] Error: Module is in blocklist
[BlocklistTest] SUCCESS: Allowed module loaded
```

**失败的情况（Block功能异常）：**
```
[BlocklistTest] onCreate called
[BlocklistTest] ERROR: Blocked module was loaded!  // 不应该看到这个
```

### 5. 清理测试配置

测试完成后，删除添加的测试模块：
```bash
# 编辑配置文件，删除测试行
vim frameworks/native/ability/native/etc/extension_blocklist_config.json

# 删除 "test.e2e.blockedModule" 这一行
```

## 自动化测试脚本

创建一个bash脚本来自动化测试流程：

```bash
#!/bin/bash
# test_blocklist.sh

echo "=== Extension Blocklist E2E Test ==="

# 1. 备份原配置
cp /system/etc/extension_blocklist_config.json /tmp/config_backup.json

# 2. 添加测试模块
sed -i '/"FormExtension":/,/]/s/\]$/,    "test.e2e.blockedModule"\n  ]/' \
  /system/etc/extension_blocklist_config.json

# 3. 安装测试Extension
hdc install out/blocklist_test_extension.hap

# 4. 启动测试并捕获日志
hdc shell "rm /data/log/*.hilog -rf"
hdc shell hilog -r && hdc shell hilog -T BlocklistTest &
LOG_PID=$!

# 5. 触发Extension启动
hdc shell aa start -a TestFormExtension -b com.test.blocklist

# 6. 等待并检查结果
sleep 5
hdc shell "cat /data/log/*.hilog | grep BlocklistTest"

# 7. 恢复原配置
cp /tmp/config_backup.json /system/etc/extension_blocklist_config.json

echo "=== Test Complete ==="
```

## 验证您的修改

针对您修改的代码（变量重命名、逻辑简化），运行以下测试：

```bash
# 1. 单元测试
./extension_config_mgr_test --gtest_filter="*LoadExtensionBlockList*"

# 2. 模块检查测试
./app_module_checker_test

# 3. 端到端测试（使用上面的方法）
./test_blocklist.sh

# 4. 检查日志输出
hdc shell hilog | grep "Failed to open config file"  # 应该看到LOGE级别
hdc shell hilog | grep "Loaded type"  # 验证加载成功
```

## 关键验证点

### 验证1：黑名单加载成功
```
预期日志：[EXT] Loaded type 4, count: 15
含义：FormExtension (type=4) 加载了15个黑名单模块
```

### 验证2：Block逻辑生效
```
预期日志：[EXT] <module_name> is in blocklist
含义：模块被正确拦截
```

### 验证3：日志级别正确
```
预期：TAG_LOGE (错误级别)
实际：包含 "Failed to open config file"
```

### 验证4：代码深度优化
```
方法：使用静态分析工具
命令：grep -A 20 "LoadExtensionBlockList" extension_config_mgr.cpp | grep "if" | wc -l
预期：嵌套层级 <= 3
```

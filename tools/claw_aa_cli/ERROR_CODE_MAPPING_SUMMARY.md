# claw_aa_cli 错误码映射扩展总结

## 修改日期
2025-04-13

## 修改概述
在 `claw_aa_cli` 的 `CreateMessageMap()` 函数中添加了 20 个错误码的映射，与 `aa` 工具中的映射关系和内容保持完全一致。

---

## 📝 添加的错误码映射

### 错误码列表

| 序号 | 枚举常量 | 错误码字符串 | 错误描述 |
|------|---------|------------|---------|
| 1 | `ABILITY_VISIBLE_FALSE_DENY_REQUEST` | 10103001 | 应用可见性检查失败 |
| 2 | `RESOLVE_ABILITY_ERR` | 10104001 | 指定的 Ability 不存在 |
| 3 | `ABILITY_SERVICE_NOT_CONNECTED` | 10105001 | Ability 服务连接失败 |
| 4 | `GET_ABILITY_SERVICE_FAILED` | 10105002 | 获取 Ability 服务失败 |
| 5 | `ERR_NOT_IN_APP_PROVISION_MODE` | 10106002 | 发布签名应用不支持调试 |
| 6 | `RESOLVE_APP_ERR` | 10100101 | Want 无法解析为应用信息 |
| 7 | `NO_FOUND_ABILITY_BY_CALLER` | 10100102 | 找不到匹配的 Ability |
| 8 | `ERR_IMPLICIT_START_ABILITY_FAIL` | 10103101 | 隐式启动失败 |
| 9 | `ERR_APP_CLONE_INDEX_INVALID` | 10103102 | 无效的 appCloneIndex |
| 10 | `START_ABILITY_WAITING` | 10106101 | Ability 正在启动中 |
| 11 | `ERR_UNLOCK_SCREEN_FAILED_IN_DEVELOPER_MODE` | 10106102 | 开发者模式下解锁屏幕失败 |
| 12 | `ERR_CROWDTEST_EXPIRED` | 10106103 | 众测过期 |
| 13 | `ERR_APP_CONTROLLED` | 10106105 | 应用受管控 |
| 14 | `ERR_EDM_APP_CONTROLLED` | 10106106 | EDM 应用管控 |
| 15 | `ERR_NOT_SUPPORTED_PRODUCT_TYPE` | 10106107 | 不支持的设备类型 |
| 16 | `ERR_STATIC_CFG_PERMISSION` | 10107102 | 静态配置权限检查失败 |
| 17 | `INNER_ERR_START` | 10108101 | 启动内部错误 |
| 18 | `GET_BUNDLE_INFO_FAILED` | 10104002 | 获取包信息失败 |
| 19 | `KILL_PROCESS_FAILED` | 10106401 | 终止进程失败 |
| 20 | `KILL_PROCESS_KEEP_ALIVE` | 10106402 | 常驻进程不能终止 |

---

## 🔧 修改的文件

### 1. include/claw_aa_command.h

**添加的内容**:
- `GetAaToolErrorInfo()` 函数声明
- `AaToolErrorInfo` 结构体定义

```cpp
std::string GetAaToolErrorInfo(std::string errorCode, std::string message,
    std::string cause, std::vector<std::string> solutions);

struct AaToolErrorInfo {
    std::string code;
    std::string message;
    std::string cause;
    std::vector<std::string> solutions;

    std::string ToString()
    {
        std::ostringstream oss;
        oss << "Error Code:" << code << "  Error Message:" << message << "\n";
        oss << "Error cause: " << cause << "\n";
        oss << "  Try the following:\n";
        for (const auto& solution : solutions) {
            oss << "  > " << solution << "\n";
        }
        return oss.str();
    }
};
```

### 2. src/claw_aa_command.cpp

**添加的内容**:

#### a) 错误解决方案字符串常量 (约 30 个)
```cpp
const std::string RESOLVE_ABILITY_ERR_SOLUTION_ONE =
    "Check if the parameter abilityName of aa -a and the parameter bundleName of -b are correct";
const std::string RESOLVE_ABILITY_ERR_SOLUTION_TWO =
    "Check if the application corresponding to the specified bundleName is installed";
// ... 更多解决方案字符串
```

#### b) GetAaToolErrorInfo 函数实现
```cpp
std::string ClawAaShellCommand::GetAaToolErrorInfo(std::string errorCode, std::string message,
    std::string cause, std::vector<std::string> solutions)
{
    AaToolErrorInfo aaToolErrorInfo;
    aaToolErrorInfo.code = errorCode;
    aaToolErrorInfo.message = message;
    aaToolErrorInfo.cause = cause;
    aaToolErrorInfo.solutions = solutions;
    return aaToolErrorInfo.ToString();
}
```

#### c) CreateMessageMap 函数扩展
添加了 20 个错误码映射，每个映射使用枚举常量作为键，通过 `GetAaToolErrorInfo()` 生成详细错误信息作为值。

```cpp
messageMap_[ABILITY_VISIBLE_FALSE_DENY_REQUEST] = GetAaToolErrorInfo("10103001", ...);
messageMap_[RESOLVE_ABILITY_ERR] = GetAaToolErrorInfo("10104001", ...);
// ... 共 20 个映射
```

---

## 📊 映射关系说明

### Map 的数据类型
```cpp
std::map<int32_t, std::string> messageMap_;
```

### 映射方式

#### 键 (Key)
- 使用**枚举常量**（如 `ABILITY_VISIBLE_FALSE_DENY_REQUEST`）
- 类型: `int32_t`
- 值: 对应的枚举整数值（如 2097179）

#### 值 (Value)
- 通过 `GetAaToolErrorInfo()` 函数生成的字符串
- 类型: `std::string`
- 格式: 包含错误码、错误消息、错误原因和修复建议的多行文本

### 错误信息格式示例

```
Error Code:10104001  Error Message:The specified ability does not exist
Error cause: The specified Ability is not installed
  Try the following:
  > Check if the parameter abilityName of aa -a and the parameter bundleName of -b are correct
  > Check if the application corresponding to the specified bundleName is installed
  > For multi-HAP applications, it is necessary to confirm whether the HAP to which the ability belongs has been installed
```

---

## ✅ 一致性保证

### 与 aa 工具保持一致的内容

1. **枚举常量作为键**: 完全相同的枚举常量名称
2. **错误码字符串**: 相同的 8 位数字错误码（如 "10104001"）
3. **错误消息**: 完全相同的英文错误描述
4. **错误原因**: 完全相同的错误原因说明
5. **修复建议**: 完全相同的解决方案列表
6. **输出格式**: 通过 `AaToolErrorInfo::ToString()` 生成相同的格式

### 代码结构一致性

```cpp
// aa 工具中的映射
messageMap_[ABILITY_VISIBLE_FALSE_DENY_REQUEST] = GetAaToolErrorInfo("10103001", ...);

// claw_aa_cli 中的映射（完全相同）
messageMap_[ABILITY_VISIBLE_FALSE_DENY_REQUEST] = GetAaToolErrorInfo("10103001", ...);
```

---

## 🎯 使用示例

### 当发生错误时

假设用户执行以下命令：
```bash
claw_aa_cli start -a TestAbility -b com.nonexistent.app
```

如果返回错误码 `RESOLVE_ABILITY_ERR` (2097152)，系统会调用 `GetMessageFromCode(result)`，该函数从 `messageMap_` 中查找对应的错误信息：

```cpp
std::string errorMessage = GetMessageFromCode(2097152);
// errorMessage 将包含详细的多行错误信息
```

然后通过 `printError()` 输出 JSON 格式的错误响应：

```json
{
  "success": false,
  "message": "failed to start ability.",
  "error": {
    "code": "2097152",
    "message": "Error Code:10104001  Error Message:The specified ability does not exist\nError cause: The specified Ability is not installed\n  Try the following:\n  > Check if the parameter abilityName of aa -a and the parameter bundleName of -b are correct\n  > Check if the application corresponding to the specified bundleName is installed\n  > For multi-HAP applications, it is necessary to confirm whether the HAP to which the ability belongs has been installed\n"
  },
  "suggestion": "Check if the bundle name and ability name are correct, and ensure the application is installed."
}
```

---

## 📝 注意事项

### 1. 枚举常量的来源
所有错误码枚举常量都定义在：
- `interfaces/inner_api/ability_manager/include/ability_manager_errors.h`

### 2. 错误码的双重性
每个错误有两个层面：
- **枚举值**: 如 `2097152` (用于 map 的键)
- **字符串错误码**: 如 `"10104001"` (用于展示给用户)

### 3. GetAaToolErrorInfo 的参数顺序
```cpp
GetAaToolErrorInfo(
    std::string errorCode,        // 8位错误码字符串
    std::string message,          // 用户可见的错误消息
    std::string cause,            // 技术性的错误原因
    std::vector<std::string> solutions  // 修复建议列表
)
```

### 4. 解决方案字符串命名规范
- 格式: `{错误码常量}_SOLUTION_ONE`, `_SOLUTION_TWO` 等
- 示例: `RESOLVE_ABILITY_ERR_SOLUTION_ONE`

---

## 🔍 验证方法

### 1. 编译验证
```bash
./build.sh --product-name <product> --build-target claw_aa_cli
```

### 2. 功能验证
测试各个错误场景，确保返回正确的错误信息。

---

## 📖 参考资料

- [aa 工具的 CreateMessageMap 实现](D:\Code\ability_ability_runtime\tools\aa\src\ability_command.cpp:281-424)
- [错误码定义文件](D:\Code\ability_ability_runtime\interfaces\inner_api\ability_manager\include\ability_manager_errors.h)
- [claw_aa_cli JSON 输出文档](D:\Code\ability_ability_runtime\tools\claw_aa_cli\JSON_OUTPUT_CHANGES.md)

---

## ✨ 总结

成功在 `claw_aa_cli` 中添加了 20 个错误码映射：

✅ **使用枚举常量作为键** - 与 aa 工具完全一致
✅ **相同的错误码字符串** - 如 "10104001"
✅ **相同的错误消息和原因** - 英文描述完全一致
✅ **相同的修复建议** - 解决方案列表完全一致
✅ **相同的输出格式** - 通过 GetAaToolErrorInfo 生成

这些映射确保 `claw_aa_cli` 能够提供与 `aa` 工具相同质量的错误信息，帮助用户快速定位和解决问题。

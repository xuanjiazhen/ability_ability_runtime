# 解决 CmdArgsBuffer 链接错误

## 问题描述
```
ld.lld: error: undefined symbol: OHOS::CliTool::CmdArgsBuffer::AddArg(std::__h::basic_string<char, std::__h::char_traits<char>, std::__h::allocator<char>> const&)
```

## 解决方案

### 方案1：清理构建并重新编译
```bash
# 清理构建产物
./build.sh --product-name <product> --build-target ability_runtime --clean

# 重新编译
./build.sh --product-name <product> --build-target ability_runtime
```

### 方案2：只重新编译 climgr 库
```bash
# 清理特定目标
./build.sh --product-name <product> --build-target climgr --clean

# 重新编译 climgr
./build.sh --product-name <product> --build-target climgr
```

### 方案3：手动清理构建目录
```bash
# 删除编译生成的对象文件
find . -name "*.o" -path "*cmd_args_buffer*" -delete
find . -name "*.so" -path "*climgr*" -delete

# 重新构建
./build.sh --product-name <product> --build-target ability_runtime
```

### 方案4：检查构建配置
确保以下文件正确配置：

1. ✅ `cli_tool_framework/services/climgr/BUILD.gn`
   - 包含 `../common/src/cmd_args_buffer.cpp`

2. ✅ `cli_tool_framework/test/unittest/process_manager_test/BUILD.gn`
   - 包含 `../common/src/cmd_args_buffer.cpp`

3. ✅ `cli_tool_framework/test/unittest/tool_util_test/BUILD.gn`
   - 包含 `../common/src/cmd_args_buffer.cpp`

4. ✅ `cli_tool_framework/test/unittest/cmd_args_buffer_test/BUILD.gn`
   - 包含 `cmd_args_buffer.cpp`

## 验证构建配置

检查头文件包含：
```bash
grep -r "cmd_args_buffer.h" cli_tool_framework/services/climgr/src/
```

应该看到：
```
cli_tool_framework/services/climgr/src/process_manager.cpp:#include "cmd_args_buffer.h"
cli_tool_framework/services/climgr/src/tool_util.cpp:#include "cmd_args_buffer.h" (via tool_util.h)
```

## 临时解决方案

如果上述方法都无效，可以尝试将 `cmd_args_buffer.cpp` 的实现直接内联到头文件中：

在 `cmd_args_buffer.h` 中将方法实现改为 inline：

```cpp
// 在 cmd_args_buffer.h 中
class CmdArgsBuffer {
public:
    void AddArg(const std::string& arg)
    {
        strings_.push_back(arg);
        UpdatePointers();
    }
    // ... 其他方法
};
```

## 构建验证

构建成功后，运行测试验证：
```bash
# 运行单元测试
run -t UT -ts cmd_args_buffer_test
run -t UT -ts tool_util_test
run -t UT -ts process_manager_test
```

## 常见问题

### Q: 为什么会出现链接错误？
A: 增量构建时，构建系统可能没有检测到新添加的源文件，导致符号未定义。

### Q: 如何避免这个问题？
A: 在添加新源文件后，始终执行 `--clean` 选项重新构建。

### Q: 影响范围有多大？
A: 只影响使用 `CmdArgsBuffer` 的模块：
- `climgr` 服务
- `process_manager_test` 测试
- `tool_util_test` 测试
- `cmd_args_buffer_test` 测试

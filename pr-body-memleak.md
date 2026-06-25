## 修改说明

- 更新 GetMemLeakStringFunc 接口签名，新增 `int *` 参数用于输出实际数据长度
- GetDumpResult 使用 `dumpResult.assign(buf, outLen)` 替代 `dumpResult = buf`，避免 std::string 接收 char* 时遇到 \0 提前截断
- GetSnapshot 使用 `write(fd, buf, outLen)` 替代 `strlen(buf)` 计算长度，同样避免 \0 截断问题

## 关联Issue

Fixes #<issue_number>

## 验证

- 编译验证：待执行
- 单元测试：待执行
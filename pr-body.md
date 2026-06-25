## 修改说明

- 为 JsHeap snapshot dump 添加 languageEnv 参数（dynamic/static/hybrid），使 ETS runtime 也能 dump 堆快照
- 移除 snapshot dump 路径的 JS-only 限制
- 添加完整的 UT 覆盖，确保所有分支都被测试到

## 关联Issue

Fixes #15499

## 验证

- native_daemon_ut：待验证

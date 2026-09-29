---
report_id: codecheck-agent-card-mgr-test-20260928
title: AgentCardMgrTest 新增 UT 逻辑检视
scope: test/unittest/agent_runtime_framework/agent_card_mgr_test/
commit_id: 8342585b1cf294bca3305416d66f842d84e34f95
subject: remove agent useless data
dimensions_executed:
  - logic-scanner
risk_level: low
score: 98
gate_decision: approve
---

# 代码检视报告 — AgentCardMgrTest 新增 UT

## 1. 门禁结论

| 维度 | 结果 |
|------|------|
| 总分 | **98 / 100** |
| 风险等级 | **low** |
| 上库决策 | **approve** ✅ |
| 执行维度 | logic-scanner |
| 跳过维度 | security-scanner (测试代码无安全信号)、input-scanner (无外部输入)、api-scanner (无 API 变更) |

**一句话结论**：新增 3 个测试用例正确覆盖了 `agent_card_mgr.cpp:196-198` 的 DeleteData 分支逻辑，mock 实现与现有模式一致，无阻塞问题。

---

## 2. 扣分原因

| 等级 | 数量 | 扣分 |
|------|------|------|
| P0 | 0 | 0 |
| P1 | 0 | 0 |
| P2 | 0 | 0 |
| P3 | 2 | -2 |

**最终评分**：100 - 2 = 98

---

## 3. 必须立即处理（P0/P1）

无。

---

## 4. 建议跟进（P2/P3）

### 4.1 [P3] DeleteData mock 缺少线程安全保护

**位置**：`test/unittest/agent_runtime_framework/agent_card_mgr_test/mock/src/mock_agent_card_db_mgr.cpp:67`

**描述**：`InsertData` 和 `QueryData` 都使用 `MyFlag::dbMutex` 保护共享状态，但 `DeleteData` 新增的 `deleteDataCallNames.push_back()` 未加锁。虽然当前测试无并发调用 DeleteData 的场景，但与现有 mock 风格不一致。

**当前代码**：
```cpp
int32_t AgentCardDbMgr::DeleteData(const std::string &bundleName, int32_t userId)
{
    MyFlag::deleteDataCallNames.push_back(bundleName);
    return MyFlag::retDeleteData;
}
```

**建议**：如需保持风格一致，可添加锁保护：
```cpp
int32_t AgentCardDbMgr::DeleteData(const std::string &bundleName, int32_t userId)
{
    std::lock_guard<std::mutex> lock(MyFlag::dbMutex);
    MyFlag::deleteDataCallNames.push_back(bundleName);
    return MyFlag::retDeleteData;
}
```

**影响**：当前无实际风险，仅为风格一致性建议。

---

### 4.2 [P3] 测试用例结尾缺少空行

**位置**：`test/unittest/agent_runtime_framework/agent_card_mgr_test/agent_card_mgr_test.cpp:1832`

**描述**：`HandleBundleInstallTest_024` 结尾的 `}` 与文件末尾的 `} // namespace AgentRuntime` 之间缺少空行，与其他测试用例的格式不一致。

**当前代码**：
```cpp
    EXPECT_EQ(MyFlag::insertDataCallNames[0], "test.bundle");
}
} // namespace AgentRuntime
} // namespace OHOS
```

**建议**：添加空行：
```cpp
    EXPECT_EQ(MyFlag::insertDataCallNames[0], "test.bundle");
}

} // namespace AgentRuntime
} // namespace OHOS
```

**影响**：无功能影响，仅为代码风格一致性。

---

## 5. 分维度速览

### 5.1 Logic Scanner

**变更概览**：

| 文件 | 变更类型 | 说明 |
|------|----------|------|
| `agent_card_mgr_test.cpp` | 新增 | 3 个测试用例 + SetUp 清理 |
| `mock_my_flag.h` | 新增 | `deleteDataCallNames` 字段声明 |
| `mock_agent_card_db_mgr.cpp` | 修改 | DeleteData 记录调用 + 静态定义 |

**逻辑分析**：

| 测试用例 | 覆盖分支 | 验证逻辑 | 结果 |
|----------|----------|----------|------|
| HandleBundleInstallTest_022 | `finalEntries.empty() && ret == ERR_OK` | DeleteData 被调用，返回 ERR_OK | ✅ 正确 |
| HandleBundleInstallTest_023 | 同上 + DeleteData 失败 | 错误码正确传播 | ✅ 正确 |
| HandleBundleInstallTest_024 | `ret == ERR_NAME_NOT_FOUND` | InsertData 被调用，DeleteData 不被调用 | ✅ 正确 |

**源代码逻辑验证** (`agent_card_mgr.cpp:196-199`)：
```cpp
if (finalEntries.empty() && ret == ERR_OK) {
    return AgentCardDbMgr::GetInstance().DeleteData(bundleName, userId);
}
return AgentCardDbMgr::GetInstance().InsertData(bundleName, userId, finalEntries);
```

- Test 022/023：无 incoming cards + QueryData 返回 ERR_OK + 无 stored cards → finalEntries 为空 → 条件满足 → DeleteData ✅
- Test 024：QueryData 返回 ERR_NAME_NOT_FOUND → `ret == ERR_OK` 为假 → InsertData ✅

**发现汇总**：

| 等级 | 数量 | 说明 |
|------|------|------|
| P0 | 0 | — |
| P1 | 0 | — |
| P2 | 0 | — |
| P3 | 2 | 线程安全风格一致性、格式空行 |

---

## 6. 关键发现详情

无关键发现（无 P0/P1/P2 问题）。

---

## 附录：Refute 日志

无 P0 发现，无需 refute。

---

*报告生成时间：2026-09-28*
*Scanner：logic-scanner*
*模板：codecheck_report_TEMPLATE.md*

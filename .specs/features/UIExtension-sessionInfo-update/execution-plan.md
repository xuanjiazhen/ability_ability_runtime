# 执行计划：UIExtension sessionInfo 支持更新

## 1. 任务编排概览

本特性改动分布在 ability_runtime 单仓的两个层面（AMS 服务侧 + 应用侧框架），共 6 个实现任务 + 2 个测试任务。跨仓依赖（ability_base、window_manager）标注但不在本次执行范围内。

### 任务依赖图

```
[跨仓前置依赖 — 不在本次范围]
T0a: ability_base SessionInfo 新增 originalCallerInfo + Marshalling
T0b: window_manager Rosen::Window / JsExtensionWindow C++ UpdateExtensionParent 接口
         │
         │ (编译依赖)
         ↓
╔═══════════════════════════════════════════════════════════╗
║  本次执行范围 (ability_runtime)                            ║
║                                                           ║
║  [AMS 服务侧]                    [应用侧]                  ║
║                                                           ║
║  T1: ExtensionRecordManager       T3: UIExtension 基类     ║
║      GetAbilityRecordByPersistentId   OnCommandWindow       ║
║      IsMigrationUpdateRequest         迁移检测+虚方法       ║
║      GetOrCreateExtensionRecord            │               ║
║           │                                │ (虚方法依赖)    ║
║           │ (map清理依赖)                   ↓               ║
║           ↓                          T4: JsUIExtension     ║
║  T2: UIExtensionAbilityManager          HandleSessionUpdate║
║      StartAbilityLocked                      │             ║
║      uiExtensionMap_ 清理                    │ (方法依赖)    ║
║                                              ↓             ║
║  T5: 服务侧单元测试 ←── T1+T2          T5: JsUIExtension   ║
║                                         ContentSession      ║
║                                         UpdateExtension  ║
║                                         Parent(C++)       ║
║                                              │             ║
║                                              ↓             ║
║                                         T6: 应用侧单元测试  ║
║                                                           ║
╚═══════════════════════════════════════════════════════════╝
```

### 依赖关系

| 任务 | 依赖于 | 说明 |
|------|--------|------|
| T1 | T0a (跨仓) | 需要 SessionInfo.originalCallerInfo 字段存在才能编译 |
| T2 | T1 | uiExtensionMap_ 清理逻辑在 StartAbilityLocked 中，需要迁移检测已就绪 |
| T3 | T0a (跨仓) | OnCommandWindow 检测 originalCallerInfo，需要字段存在 |
| T4 | T3 | JsUIExtension 重写 HandleSessionUpdate，需要基类虚方法已声明 |
| T5-app | T4 | ContentSession 更新方法被 JsUIExtension 调用 |
| T6-test-srv | T1, T2 | 服务侧测试需要匹配函数和 map 清理就绪 |
| T6-test-app | T3, T4, T5-app | 应用侧测试需要迁移检测 + HandleSessionUpdate + ContentSession 方法就绪 |

---

## 2. 任务清单

| ID | 任务 | 范围 | 文件 | 依赖 |
|----|------|------|------|------|
| T1 | ExtensionRecordManager 迁移匹配 | AMS 服务侧 | `extension_record_manager.cpp/.h` | T0a |
| T2 | UIExtensionAbilityManager map 清理 | AMS 服务侧 | `ui_extension_ability_manager.cpp` | T1 |
| T3 | UIExtension 基类迁移检测 | 应用侧 | `ui_extension.cpp/.h` | T0a |
| T4 | JsUIExtension HandleSessionUpdate | 应用侧 | `js_ui_extension.cpp/.h` | T3 |
| T5 | JsUIExtensionContentSession 更新方法 | 应用侧 | `js_ui_extension_content_session.cpp/.h` | T4 |
| T6 | 单元测试（服务侧 + 应用侧） | 测试 | `test/unittest/` | T1-T5 |

**跨仓依赖（不在本次范围，但需对齐）：**

| ID | 任务 | 仓 | 说明 |
|----|------|----|------|
| T0a | SessionInfo 新增 originalCallerInfo | ability_base | 结构体字段 + Marshalling/Unmarshalling |
| T0b | Rosen::Window / JsExtensionWindow C++ UpdateExtensionParent | window_manager | 窗口/proxy parent 更新 C++ 接口（无需 napi 暴露） |

---

## 3. 关键设计决策回顾

| ADR | 任务关联 | 执行要点 |
|-----|----------|----------|
| ADR-1 | T1 | 迁移分支注入 GetOrCreateExtensionRecord 最前面，命中后 return |
| ADR-2 | T1 | 按 persistentId 匹配（不变），originalCallerInfo.callerToken 仅作标识 |
| ADR-3 | T2 | uiExtensionMap_ 按 persistentId 查找清理，不扩展 originalCallerInfo 结构 |
| ADR-4 | — | persistentId 不变，多次迁移天然支持，无需额外代码 |
| ADR-5 | T1, T2 | 复用现有 mutex_，不加额外锁 |
| ADR-6 | T1 | IsMigrationUpdateRequest 封装判断 |
| ADR-7 | T3, T4, T5 | 两层架构：基类检测 → JsUIExtension 重写执行 → ContentSession 调窗口新接口 |

---

## 4. 实施完成定义 (DoD)

### 代码完成标准
- [ ] T1-T5 全部代码变更已提交
- [ ] 所有新增方法有日志（TAG_LOGI/TAG_LOGW/TAG_LOGD）
- [ ] 现有 `GetAbilityRecordBySessionInfo`、`IsPreloadExtensionRecord`、`GetOrCreateExtensionRecordInner`、`ForegroundWindow`、`BackgroundWindow` 逐行未改

### 测试完成标准
- [ ] T6 单元测试全部编写并通过
- [ ] 现有 `extension_record_manager_test`、`ui_extension_ability_manager_test` 等回归测试全部通过
- [ ] 覆盖 spec.md §6.1 全部 13 条单元测试场景

### 跨仓对齐标准
- [ ] ability_base 的 originalCallerInfo 字段已合入或同批次合入
- [ ] window_manager 的 UpdateExtensionParent C++ 接口需求已提交（PR #19373，可后续迭代）

---

## 5. 交接与验证

### 验证命令

```bash
# 构建
./build.sh --product-name <product> --build-target abilityms

# 服务侧单元测试
run -t UT -ts ability_manager_service_first_test
run -t UT -ts ability_manager_service_third_test
run -t UT -ts ability_manager_service_sixth_test

# 应用侧单元测试（若存在 ui_extension 相关测试套）
run -t UT -ts ui_extension_test

# 集成验证（手动）
aa dump -a  # 检查迁移后 dump 信息的 abilityName/persistentId
hilog -b D | grep -i "migration update"  # 检查迁移日志
```

### review.md 检查清单（Stage 3 结束时填写）
- 规范符合性：design.md ADR 是否全部实现
- 代码质量：日志、错误处理、命名规范
- 验证证据：测试结果截图/日志
- AC 对照：spec.md 13 条 AC 是否全部满足

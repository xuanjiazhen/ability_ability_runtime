# FUZZ用例生成和规范化检查报告

- **生成时间**: 2026-08-08 20:45:14
- **Fuzzer**: `uripermissionmanagerclient_fuzzer`
- **命名空间**: `OHOS::AAFwk`

## 一、测试范围

## 二、种子(Corpus)生成

**种子状态**: 使用默认初始化种子

## 三、规范检查结果

### 自动检查结果

**状态**: ❌ 未执行自动规范检查

**警告**: 提交前**必须**运行规范检查，请执行:
```bash
python3 tools/fuzz_check.py E:\OH\ams\ability_ability_runtime\test\fuzztest\uripermissionmanagerclient_fuzzer/uripermissionmanagerclient_fuzzer.cpp
```

## 四、人工审查确认

以下4条规则无法通过自动化工具完全验证，需要提交者人工确认:

| 规则 | 名称 | 严重程度 | 检查项 | 状态 |
|------|------|----------|--------|------|
| 规则011 | 系统安全准入条件 | 🔴 高危 | 目标API是否需要特定权限/UID？ | [ ] 待确认 |
| 规则012 | 目标API内部分支覆盖 | 🔴 高危 | 是否覆盖主要分支逻辑？ | [ ] 待确认 |
| 规则015 | 中间产物合法性 | 🔴 高危 | 种子数据格式是否符合预期？ | [ ] 待确认 |
| 规则016 | 类型匹配检查 | 🟡 中危 | 复杂类型参数是否与API签名一致？ | [ ] 待确认 |

> **说明**: 规则016工具已检查基础类型匹配，复杂类型(结构体/类)需要人工确认。

---
*报告由 fuzz_generator.py 自动生成*
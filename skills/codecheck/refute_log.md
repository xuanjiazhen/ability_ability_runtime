# Refute Log — commit fa4c35b36732ff6a7eca33eb30197518e14d8d10

> 对抗性验证记录。每条发现执行三层质疑：触发路径证伪 / 影响夸大证伪 / 跨 scanner 根因去重。

---

## LOG-01: cj_insightintentcontext target missed in modified file (P2)

### 第一层：触发路径证伪
- **上游有防护吗？** 无。提交 `fa4c35b` 明确修改了 `frameworks/native/insight_intent/insight_intent_context/BUILD.gn`，向 `insightintentcontext` 目标（line 24-65）添加了 `node:node_header_notice`（line 60），但同文件的 `cj_insightintentcontext` 目标（line 67-117）有 `napi:ace_napi`（line 109）却未添加。两个目标共享源文件 `insight_intent_context.cpp`（line 31 vs line 82），且 `cj_insightintentcontext` 额外有 `napi:cj_bind_ffi`/`napi:cj_bind_native` 依赖。
- **不可达路径？** 否。该目标是实际编译目标，`cj_*.cpp` 文件使用 CJ FFI 框架（基于 NAPI），且 `napi:ace_napi` 在 external_deps 中明确声明。
- **判定**：✅ 维持。发现确认可达——同一文件内一目标已改、另一目标漏改。

### 第二层：影响夸大证伪
- **后果真的这么严重吗？** `node:node_header_notice` 是编译期 header notice 合规检查，非运行时依赖。缺失不会导致崩溃、安全漏洞或功能异常。但该目标与已修改目标在同一文件，提交声明为"编译整改"，漏改导致合规检查覆盖不完整。
- **判定**：⬇️ 从 P1 降级为 P2。无运行时/安全影响，仅为构建合规一致性问题。

### 第三层：跨 scanner 根因去重
- 与 LOG-02 共享根因（`node:node_header_notice` 添加不完整），但修复位置不同（本条改同一文件内另一 target，LOG-02 改不同文件），属独立发现。
- **判定**：🔀 不合并，维持独立。

---

## LOG-02: 5 NAPI BUILD.gn files completely missed (P3)

### 第一层：触发路径证伪
- **上游有防护吗？** 无。以下 5 个文件均有 `napi:ace_napi` 但无 `node:node_header_notice`：
  - `frameworks/js/napi/task_manager/BUILD.gn`
  - `frameworks/js/napi/photo_editor_extension_ability/BUILD.gn`
  - `frameworks/js/napi/photo_editor_extension_context/BUILD.gn`
  - `frameworks/js/napi/quick_fix/BUILD.gn`
  - `frameworks/js/napi/extensioncontext/BUILD.gn`
- **不可达路径？** 否。这些是实际编译目标，源文件使用 NAPI API。
- **判定**：✅ 维持。

### 第二层：影响夸大证伪
- **后果真的这么严重吗？** 同 LOG-01——构建期合规检查，无运行时影响。这些文件未被本提交触及，属覆盖范围遗漏而非文件内错误。
- **判定**：⬇️ 从 P2 降级为 P3。未触及的文件，仅为建议补充覆盖。

### 第三层：跨 scanner 根因去重
- 5 个文件共享同一根因（`node:node_header_notice` 未覆盖所有 NAPI 模块），修复方式相同（对每个文件添加依赖）。
- **判定**：🔀 合并为一条。5 个文件合并为 LOG-02。

---

## LOG-03: 3 ANI BUILD.gn files missed, excluded scope (P3)

### 第一层：触发路径证伪
- **上游有防护吗？** 无。以下 3 个 ANI 文件有 `napi:ace_napi` 但无 `node:node_header_notice`：
  - `frameworks/ets/ani/task_manager/BUILD.gn`
  - `frameworks/ets/ani/app/form_extension_context/BUILD.gn`
  - `frameworks/ets/ani/dialog_request_info/BUILD.gn`
- **不可达路径？** 否，这些是实际编译目标。
- **判定**：✅ 维持。但注意：ANI 目录属静态语言排除范围（conventions.md §1），仅为观察项。

### 第二层：影响夸大证伪
- 同上——构建期合规检查，无运行时影响。
- **判定**：P3 观察项，不降不升。

### 第三层：跨 scanner 根因去重
- 3 个文件共享同一根因。
- **判定**：🔀 合并为一条。

---

## 汇总

| 发现 ID | 原始等级 | 判定 | 最终等级 | 理由 |
|---------|---------|------|---------|------|
| LOG-01 | P1 | ⬇️ 降级 | P2 | 无运行时影响，构建合规一致性问题 |
| LOG-02 | P2 | ⬇️ 降级 + 🔀 合并 | P3 | 未触及文件，5 文件合并为 1 条 |
| LOG-03 | P3 | 🔀 合并 | P3 | 排除范围，3 文件合并为 1 条，观察项 |

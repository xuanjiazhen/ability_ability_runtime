---
id: draft-20260728
type: feature
title: "启动阶段采集应用 import 的 Native 模块并保存至应用沙箱"
spec_schema: ohos-sdd/v1
profile: none
complexity: standard
lineage: new
status: approved
owner: "yangzk"
source_issue: ""
created_at: 2026-07-28
updated_at: 2026-07-29
related: []
related_tasks: []
related_decisions: []
subprofiles: []
profile_source: inferred
code_refs:
  - "openharmony_master/code/foundation/ability/ability_runtime"
  - "openharmony_master/code/foundation/arkui/napi"
commits:
  - "ability_runtime/93d2964db7 TASK-2 NativeModuleCollector skeleton"
  - "ability_runtime/ee5e5a513d TASK-2 fix group(unittest) alias target"
  - "arkui/napi/33cffc11 TASK-SKELETON-1+TASK-1 GetLoadedModuleInfos"
  - "ability_runtime/107a19470f TASK-3 EWMA merge engine"
  - "ability_runtime/1a6221c47f TASK-4+5 checkpoints wiring and preload exclusion"
  - "ability_runtime/1f34e46476 TASK-6+7 predict-preload consumer and anti-self-feedback"
  - "ability_runtime/a4a7cdac01 TASK-8+9 CCM params and hit-rate reporting"
---

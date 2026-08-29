---
id: ai-task-manager
type: feature
title: "AI 语音助手任务管理机制"
spec_schema: ohos-sdd/v1
profile: none
complexity: standard
lineage: new
status: draft
owner: "yangzk"
source_issue: ""
created_at: 2026-08-21
updated_at: 2026-08-26
related: []
related_tasks: []
related_decisions:
  - "D-1 模块形态=AppMS 收口（三层：task_mgr/process_schedule/resource_control_proxy）"
  - "D-4 防仿冒=B-1+B-2+通道鉴权（F-8）"
  - "D-11 UI↔助手传递通道由本模块提供"
  - "D-16 豁免记录=内存登记（不持久化）"
  - "D-19 ITaskManager 经 SA 501 描述符路由承载（F-21）"
  - "D-21 AI bar 内化交互，监听保留给安全等模块（取代 D-11）"
  - "D-20 拦截点①共 5 收敛点（F-22）"
subprofiles: []
profile_source: owner
code_refs: []
commits:
  - "0dbffb9a6c SDD 文档初版（proposal/spec/design/execution-plan）"
  - "08bda1df9e 第二轮：RSS 代理层封装与接口升级"
  - "bb8e6a9e10 第六轮：设计深化与对外呈现图"
  - "f7b95cdf71 第七轮：深度审查 F-1~F-7 修正"
  - "e4b8047be9 第八轮：交互态超时两段式（F-5）"
  - "365cff1acf 第九轮：全量复审 21 项缺陷修复"
  - "9a5864e8b5 第十轮：开放点闭环 13 项决策"
  - "7747518002 第十一轮：豁免记录降级内存登记"
  - "6325ac84f0 第十二轮：外围交付件同步"
  - "9f567b4de6 会话级交接文档（后于第十四轮删除，历史可追溯）"
  - "59f50fd61f 第十三轮：自审 F-8~F-20 + 待定项确认"
  - "3849a3aed8 第十四轮：代码实证复核 F-21~F-28"
  - "41cbcf22b7 交接文档信息合并收口"
  - "ed0798db1c Level D 归档证据补齐（validate --level all 全量通过）"
  - "fd97b8a7df 第十五轮：IPC 机制深挖 F-29~F-35"
  - "fe1d88b52c 第十六轮：拦截器框架与参数语义 F-36~F-41"
  - "85032896e9 第十七轮：状态机闭环与线程模型 F-43~F-46"
  - "5a12427dcd 分步流程图集（8 图）"
  - "170506ec66 图1监听者泛化"
  - "D-21 需求变更：AI bar 内化交互（第十八轮）"
---

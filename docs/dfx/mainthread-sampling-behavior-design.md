# 主线程采样栈与 `mainthread_sampling:enable` 行为设计

## 背景

应用通过 `app.json5` 的 `appEnvironments` 配置 `DFX_APPFREEZE_LOG_OPTIONS`，可包含子串 `mainthread_sampling:enable`。历史上该开关表示「开启主线程采样增强日志」；未配置时系统仍会在 `THREAD_BLOCK_3S` 等阶段调用 xcollie 采样，但上报给 FreezeDetector 时通过 `ENABLE_MAINTHREAD_SAMPLE=0` 不附带独立增强日志路径，`FREEZE_INFO_PATH` 为空。

## 目标行为

| 配置 | 采样任务 | 故障主日志（APP FREEZE 文本） | `FREEZE_INFO_PATH` / HiSysEvent `ENABLE_MAINTHREAD_SAMPLE` |
|------|----------|-------------------------------|------------------------------------------------------------|
| **未**设置 `mainthread_sampling:enable`（默认） | 与现网一致：`3s` 起采、`6s`/等事件停采 | 将采样栈文件与 CPU 增强日志（若存在）**追加到** `errorObject.message` 尾部 | `enableFreeze=0`，`freezeInfoFile` 为空；Hiview 侧不挂独立增强路径 |
| **已**设置 `mainthread_sampling:enable` | 不变 | **不**向主日志追加采样内容（与历史一致） | `enableFreeze=1`，`freezeInfoFile` 为 `stackFile,cpuFile`；独立增强日志 |

## 关键路径

1. **应用进程**：`Watchdog::SetMainThreadSample` 解析 env → `AppfreezeInner::SetMainThreadSample` → `FaultData.isEnableMainThreadSample`。
2. **应用进程**：`EnableFreezeSample` 仍在 `IsAppFreeze` 时 `StopSample` / `GetSamplerResult`（采样不依赖开关）。
3. **应用进程（appkit）**：`AppfreezeInner::ChangeFaultDateInfo` 在 `NotifyAppFault` 前调用 `AppendInlineSamplingStackToMessage`：若 `!GetMainThreadSample()` 且 `IsAppFreeze` 且 `appfreezeInfo` 非空，从应用可读路径 `/data/storage/el2/log/watchdog/freeze/`（与 xcollie `FREEZE_DIR` 一致）读取采样栈文件，追加到 `errorObject.message`（AppMgr 无法读应用沙箱内该路径）。
4. **系统进程（appdfr）**：`AppfreezeManager::NotifyANR` 开头 `AppendInlineCpuSamplingToFaultMessage`：若 `!isEnableMainThreadSample` 且为上述完整事件，调用 `GetCpuInfoPath` 生成 CPU 增强文件并读回内容，追加到已含采样栈的 `message` 尾部（与原先独立文件生成逻辑共用 `GetCpuInfoPath`）。
5. **系统进程**：`GetAppfreezeInfoPath` 仅在 `isEnableMainThreadSample==true` 时返回 `appfreezeInfo[,cpuPath]`（内部触发 `GetCpuInfoPath`）；默认内联模式下返回空串且不再重复调用 `GetCpuInfoPath`（由步骤 4 已消费）。
6. **Hiview**：`freeze_detector` 仍用 `ENABLE_MAINTHREAD_SAMPLE` 与 `FREEZE_INFO_PATH`；默认模式下路径为空，主日志已含栈与 CPU 内联段。

## 兼容性

- 仍配置 `mainthread_sampling:enable` 的应用：行为与改前一致（独立文件 + HiAppEvent `external_log` 第二项等）。
- 未配置应用：默认即可获得主日志尾部采样栈与 CPU 段，无需改 `app.json5`。

## 同步约束

采样栈文件目录须与 `base/hiviewdfx/hicollie/frameworks/native/xcollie_utils.h` 中 `FREEZE_DIR` 保持一致（当前为 `/data/storage/el2/log/watchdog/freeze/`）。若上游调整目录，须同步更新 `frameworks/native/appkit/dfr/appfreeze_inner.cpp` 内 `FREEZE_SAMPLE_DIR`。

## Changelog

- 2026-05-08: 初始版本（默认内联、显式开关保持独立文件）。

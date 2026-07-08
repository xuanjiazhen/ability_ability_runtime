# IPC PoC — 直接命中 AbilityManagerService::ExecuteIntent

这个 HAP 工程演示：**完全跳过 `AbilityManagerClient` / `AbilityManagerProxy`**，自己拼 `MessageParcel` + `SendRequest`，直接给 `AbilityManagerService::ExecuteIntent` 发一个 `EXECUTE_INTENT = 72` 的 IPC 请求。

攻击链：
```
本 HAP 进程 (ArkTS)
  └─ import 'libpoc.so'                ← 我们自己的 NAPI 模块
       └─ IPCSkeleton::GetContextObject()   → 拿到 samgr 的 IRemoteObject
            └─ samgr->SendRequest(1, {id=4}) → 拿到 AMS 的 IRemoteObject
                 └─ ams->SendRequest(72, {key, callerToken, InsightIntentExecuteParam})
                      └─ AbilityManagerStub::OnRemoteRequest
                           └─ ExecuteIntentInner
                                └─ AbilityManagerService::ExecuteIntent   ← 命中
```

完全没用 `@ohos.abilityManager`、`@ohos.insightIntentDriver`、`AbilityManagerClient`、`AbilityManagerProxy` 这些 SDK 包装。

---

## 工程结构

```
ipc-poc/
├── AppScope/
│   ├── app.json5
│   └── resources/base/element/string.json
├── build-profile.json5
├── hvigorfile.ts
├── oh-package.json5
└── entry/
    ├── build-profile.json5
    ├── hvigorfile.ts
    ├── oh-package.json5
    └── src/main/
        ├── cpp/
        │   ├── CMakeLists.txt
        │   ├── poc.cpp                   ← 核心 PoC
        │   └── types/libpoc/Index.d.ts
        ├── ets/
        │   ├── entryability/EntryAbility.ets
        │   └── pages/Index.ets           ← UI
        ├── module.json5
        └── resources/base/
            ├── element/{string,color}.json
            ├── element/dark/color.json
            ├── media/                    ← 需要补图标（见下）
            └── profile/main_pages.json
```

## 缺少的图标资源

DevEco Studio 工程需要图标文件。**从任意 DevEco 模板工程复制**这几张 png 到 `entry/src/main/resources/base/media/`：
- `icon.png`（应用图标）
- `foreground.png` / `background.png`（启动窗口图标）

或者改 `module.json5` 把 `$media:icon` 改成项目里已有的资源。

---

## 编译/运行步骤

1. **DevEco Studio 5.0+** 打开 `C:\workspace\ipc-poc`（File → Open）。
2. 工具会自动检测这是 Stage 模型 HAP，应该能直接 sync。如果提示 SDK 版本不对，把 `build-profile.json5` 里的 `compileSdkVersion`/`compatibleSdkVersion` 改成本机已安装的版本（10/11/12 都行）。
3. 配置签名：File → Project Structure → Signing Configs → 勾 "Automatically generate signature"（用调试证书即可，**不需要系统签名**）。
4. 连接 OpenHarmony 设备，点 Run。
5. 应用起来后，输入框默认填 `100`，点按钮。

## 预期结果

UI 上"service returned"会显示一个数字。三种情况：

| 返回值 | 含义 |
|--------|------|
| `0` | service 端 `ExecuteIntent` 接受并执行了——**重大问题**，IPC 绕过成功且无防御 |
| 正数（常见 `2097152` = `CHECK_PERMISSION_FAILED`） | service 端用 `IPCSkeleton::GetCallingTokenID` + `CheckAndUpdateParam` 拒了——IPC 绕过成功，但 service 端有正确防御 |
| 负数（`-1001`/`-1002`/`-1003`） | local 端拼 parcel 失败或 samgr 查不到，去看 hilog |

**无论返回正数还是 0，能返回非 `-1001` 都说明"client 绕过 + 命中 service 函数体"这件事成功了。** 区别只是 service 端有没有正确拒掉。

## 看日志

```bash
hdc shell hilog -t IpcPoc
```

Native 代码用 `OHOS::Hilog::Info/Error` 打了这些关键节点：
- `got samgr IRemoteObject`
- `got AMS IRemoteObject via samgr`
- `sending EXECUTE_INTENT (code=72), userId=...`
- `ExecuteIntent service-side result: <code>`

如果某一步失败会清楚标出。

## 这个工程为什么不需要系统签名

- NAPI 模块是 OpenHarmony HAP 的合法能力，普通应用签名即可打包 `.so`
- `IPCSkeleton::GetContextObject()` 是 NDK 公开 API
- `IRemoteObject::SendRequest` 是 NDK 公开 API
- 没有用 `dlopen` 加载 samgr——通过 `IPCSkeleton` 公开入口拿 samgr

也就是说，没有任何"绕开 SDK 限制"的技巧。**完全靠 OpenHarmony 自己公开的 NDK API 拼出来的 PoC。**

## 下一步

把 `WriteInsightIntentParam` 里的 `bundleName_` / `moduleName_` 改成你机器上真实存在的目标应用，或者把 transaction code 从 `72`（EXECUTE_INTENT）改成其他 code（比如 `47`=STOP_USER）来验证前面发现的漏洞。

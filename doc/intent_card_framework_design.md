# IntentCard 卡片框架设计方案

## 1. 背景与目标

意图执行过程中，应用可能需要向用户展示授权确认卡片（包含标题、描述、同意/取消按钮）。
小艺（调用方）只负责**渲染卡片 UI + 透传按钮点击**，不解析业务逻辑。
按钮点击后由框架自动执行对应意图，结果通过回调返回。

设计目标：小艺零业务感知，框架闭环处理。

## 2. 参与方及职责

| 参与方 | 职责 | 对外接口 |
|--------|------|----------|
| **应用（意图执行方）** | 执行意图逻辑，需要授权时返回 IntentCard | 返回 `InsightIntentExecuteResult` 中携带 IntentCard |
| **小艺（调用方）** | 渲染卡片 UI，转发按钮点击 | 调用 `IntentClient` 接口，实现 `CardActionCallback` |
| **IntentClient（框架侧）** | 编解码 actionToken，自动执行按钮意图 | 提供 `ExecuteIntentWithCardAction`、`ExecuteCardAction` |
| **AbilityManagerService（服务端）** | 不感知卡片，只负责意图调度 | 已有 `ExecuteIntentByFunctionCall`，无需改动 |

## 3. 数据结构定义

### 3.1 IntentCardButton — 按钮定义

```cpp
// 文件: interfaces/kits/native/intent_client/include/intent_card.h

#include <string>
#include <vector>
#include "parcel.h"
#include "want_params.h"

namespace OHOS::AAFwk {

// 按钮样式
enum class IntentCardButtonStyle : int32_t {
    PRIMARY = 0,     // 主按钮（同意/确认），高亮
    SECONDARY = 1,   // 次按钮（取消），普通
    DANGER = 2,      // 危险按钮（删除/拒绝），红色
};

struct IntentCardButton : public Parcelable {
    std::string label;                     // 按钮文案，如 "同意授权"
    IntentCardButtonStyle style = IntentCardButtonStyle::PRIMARY;
    std::string actionToken;               // 不透明令牌，小艺不解析

    bool Marshalling(Parcel &parcel) const override;
    static IntentCardButton *Unmarshalling(Parcel &parcel);
};

}
```

### 3.2 IntentCardContent — 卡片内容

```cpp
struct IntentCardContent : public Parcelable {
    std::string title;                      // 卡片标题，如 "位置权限申请"
    std::string subtitle;                   // 副标题，如 "地图应用请求获取位置信息"
    std::string detail;                     // 详细描述
    std::string iconUri;                    // 可选，应用图标资源 URI
    std::vector<IntentCardButton> buttons;  // 按钮列表

    bool Marshalling(Parcel &parcel) const override;
    static IntentCardContent *Unmarshalling(Parcel &parcel);
};
```

### 3.3 IntentCard — 完整卡片

```cpp
struct IntentCard : public Parcelable {
    std::string cardId;                     // 卡片唯一标识，框架内部用于关联动作
    IntentCardContent content;
    int32_t timeoutMs = 0;                  // 超时毫秒（0 = 不限），超时视为取消

    bool Marshalling(Parcel &parcel) const override;
    static IntentCard *Unmarshalling(Parcel &parcel);
};
```

### 3.4 actionToken 内部格式（框架内部，对外不可见）

```cpp
// 文件: interfaces/kits/native/intent_client/src/intent_card_action.cpp
// 编码/解码仅框架内部使用，小艺和应用都不感知

struct IntentCardAction {
    std::string bundleName;        // 按钮触发的目标应用
    std::string intentName;        // 按钮触发的意图名
    WantParams wantParam;          // 传给意图的参数

    std::string Encode() const;    // 序列化为 base64 string → 写入 actionToken
    static bool Decode(const std::string &token, IntentCardAction &action);
};
```

### 3.5 WantParams 传输约定

应用侧通过 `InsightIntentExecuteResult.result` 返回卡片时，使用以下 key：

| Key | 类型 | 说明 |
|-----|------|------|
| `"ohos.insightIntent.card"` | WantParams (嵌套) | IntentCard 整体 |
| `"ohos.insightIntent.card.id"` | String | cardId |
| `"ohos.insightIntent.card.title"` | String | 标题 |
| `"ohos.insightIntent.card.subtitle"` | String | 副标题 |
| `"ohos.insightIntent.card.detail"` | String | 详情 |
| `"ohos.insightIntent.card.iconUri"` | String | 图标 URI |
| `"ohos.insightIntent.card.timeoutMs"` | Int | 超时时间 |
| `"ohos.insightIntent.card.buttons"` | WantParams (嵌套数组) | 按钮列表 |
| `"ohos.insightIntent.card.buttons[i].label"` | String | 按钮文案 |
| `"ohos.insightIntent.card.buttons[i].style"` | Int | 按钮样式枚举值 |
| `"ohos.insightIntent.card.buttons[i].actionToken"` | String | 不透明动作令牌 |

## 4. 接口定义

### 4.1 CardActionCallback — 小艺实现的回调接口

```cpp
// 文件: interfaces/kits/native/intent_client/include/card_action_callback.h

namespace OHOS::AAFwk {

class CardActionCallback : public RefBase {
public:
    virtual ~CardActionCallback() = default;

    // 框架回调：展示卡片给用户
    virtual void OnCardShow(const IntentCard &card) = 0;

    // 框架回调：按钮动作执行完成，返回最终结果
    virtual void OnActionResult(int32_t errCode, const WantParams &result) = 0;

    // 框架回调：卡片超时（timeoutMs > 0 且用户未操作）
    virtual void OnCardTimeout(const std::string &cardId) = 0;

    // 框架回调：卡片被关闭（应用侧主动关闭）
    virtual void OnCardClose(const std::string &cardId) = 0;
};

}
```

### 4.2 IntentClient — 新增接口

```cpp
// 文件: interfaces/kits/native/intent_client/include/intent_client.h（在已有类上扩展）

class IntentClient {
public:
    // === 已有接口 ===
    static IntentClient &GetInstance();
    int32_t ExecuteIntentByFunctionCall(uint64_t key, const sptr<IRemoteObject> &callerToken,
        const std::string &bundleName, const std::string &intentName, const WantParams &wantParam);

    // === 新增接口 ===

    /**
     * @brief 带卡片回调的意图执行。
     * 发起意图执行，结果通过 CardActionCallback 异步返回。
     * 如果返回的是卡片，走 OnCardShow；如果直接返回结果，走 OnActionResult。
     *
     * @param callerToken 调用方 token
     * @param bundleName 目标应用 bundleName
     * @param intentName 意图名
     * @param wantParam 执行参数
     * @param callback 卡片动作回调
     * @return ERR_OK 表示调用发起成功
     */
    int32_t ExecuteIntentWithCardAction(const sptr<IRemoteObject> &callerToken,
        const std::string &bundleName, const std::string &intentName,
        const WantParams &wantParam, const sptr<CardActionCallback> &callback);

    /**
     * @brief 执行卡片按钮动作。
     * 小艺在用户点击按钮时调用，传入按钮的 actionToken。
     * 框架内部解码 token，发起对应的意图执行。
     * 结果通过 CardActionCallback::OnActionResult 返回。
     *
     * @param actionToken 按钮携带的不透明令牌
     * @return ERR_OK 表示调用发起成功
     */
    int32_t ExecuteCardAction(const std::string &actionToken);

private:
    IntentClient() = default;
    sptr<IRemoteObject> GetAbilityManagerRemote();

    // 内部：解析 result，判断是卡片还是直接结果
    void HandleExecuteResult(const sptr<CardActionCallback> &callback,
        int32_t errCode, const WantParams &result);

    std::mutex mutex_;
    sptr<IRemoteObject> remoteObj_;
    sptr<IRemoteObject::DeathRecipient> deathRecipient_;

    // 存储 cardId → callback 映射，用于按钮回调时找到原始 callback
    std::mutex cardCallbackMutex_;
    std::map<std::string, sptr<CardActionCallback>> cardCallbacks_;
};
```

## 5. 通信流程

### 5.1 场景：应用需要授权

```
小艺                                IntentClient                        AMS                     应用
 │                                      │                                │                       │
 │ ExecuteIntentWithCardAction(         │                                │                       │
 │   callerToken,                       │                                │                       │
 │   "com.example.map",                 │                                │                       │
 │   "GetLocation",                     │                                │                       │
 │   wantParam,                         │                                │                       │
 │   callback)                          │                                │                       │
 │─────────────────────────────────────>│                                │                       │
 │                                      │ IPC: SendRequest               │                       │
 │                                      │ (EXECUTE_INTENT_BY_FUNCTION)   │                       │
 │                                      │───────────────────────────────>│                       │
 │                                      │                                │ ExecuteIntentCommon    │
 │                                      │                                │──────────────────────>│
 │                                      │                                │                       │
 │                                      │                                │   应用发现需要授权      │
 │                                      │                                │   构造 IntentCard:     │
 │                                      │                                │   title="位置权限申请"  │
 │                                      │                                │   buttons=[           │
 │                                      │                                │     {同意, primary,    │
 │                                      │                                │      token(编码后)}    │
 │                                      │                                │     {取消, secondary,  │
 │                                      │                                │      token(编码后)}    │
 │                                      │                                │   ]                   │
 │                                      │                                │<──────────────────────│
 │                                      │   IPC reply: result            │                       │
 │                                      │<───────────────────────────────│                       │
 │                                      │                                │                       │
 │                                      │ HandleExecuteResult()           │                       │
 │                                      │ 解析 result 中有               │                       │
 │                                      │ "ohos.insightIntent.card"      │                       │
 │                                      │ → 反序列化为 IntentCard        │                       │
 │                                      │                                │                       │
 │ callback->OnCardShow(card)           │                                │                       │
 │<─────────────────────────────────────│                                │                       │
 │                                      │                                │                       │
 │ 渲染卡片，用户点击"同意授权"           │                                │                       │
 │                                      │                                │                       │
 │ ExecuteCardAction(actionToken)       │                                │                       │
 │─────────────────────────────────────>│                                │                       │
 │                                      │ Decode(actionToken)             │                       │
 │                                      │ → bundleName="com.ohos.pm"    │                       │
 │                                      │ → intentName="Authorize"       │                       │
 │                                      │ → wantParam={grant:true,...}   │                       │
 │                                      │                                │                       │
 │                                      │ IPC: SendRequest               │                       │
 │                                      │ (EXECUTE_INTENT_BY_FUNCTION)   │                       │
 │                                      │───────────────────────────────>│                       │
 │                                      │                                │──── 权限管理应用授权 ──>│
 │                                      │                                │<─── 授权完成，返回结果 ──│
 │                                      │   IPC reply                    │                       │
 │                                      │<───────────────────────────────│                       │
 │                                      │                                │                       │
 │ callback->OnActionResult(result)     │                                │                       │
 │<─────────────────────────────────────│                                │                       │
```

### 5.2 场景：应用直接返回结果（无卡片）

```
小艺                                IntentClient
 │                                      │
 │ ExecuteIntentWithCardAction(...)     │
 │─────────────────────────────────────>│
 │                                      │ HandleExecuteResult()
 │                                      │ result 中无 "ohos.insightIntent.card"
 │                                      │ 直接走 OnActionResult
 │ callback->OnActionResult(0, result)  │
 │<─────────────────────────────────────│
```

## 6. 新增/修改文件清单

### 新增文件

| 文件 | 说明 |
|------|------|
| `interfaces/kits/native/intent_client/include/intent_card.h` | IntentCard / IntentCardContent / IntentCardButton / IntentCardButtonStyle 定义 |
| `interfaces/kits/native/intent_client/include/card_action_callback.h` | CardActionCallback 回调接口 |
| `interfaces/kits/native/intent_client/include/intent_card_action.h` | IntentCardAction 内部编解码结构 |
| `interfaces/kits/native/intent_client/src/intent_card.cpp` | IntentCard 序列化/反序列化 |
| `interfaces/kits/native/intent_client/src/intent_card_action.cpp` | actionToken 编解码逻辑 |
| `interfaces/kits/native/intent_client/src/card_action_callback.cpp` | 回调分发逻辑（如超时处理） |

### 修改文件

| 文件 | 说明 |
|------|------|
| `interfaces/kits/native/intent_client/include/intent_client.h` | 新增 `ExecuteIntentWithCardAction`、`ExecuteCardAction`、`HandleExecuteResult` |
| `interfaces/kits/native/intent_client/src/intent_client.cpp` | 实现新接口 + result 解析逻辑 |
| `interfaces/kits/native/intent_client/BUILD.gn` | 添加新源文件 |

### 不改动的文件

- 服务端（stub/proxy/service）无需改动
- `InsightIntentExecuteResult` 结构不变，卡片数据放在 `result` WantParams 里

## 7. 实现步骤

1. 定义 `intent_card.h` — IntentCard 全套数据结构 + Parcelable
2. 定义 `card_action_callback.h` — 回调接口
3. 定义 `intent_card_action.h/.cpp` — actionToken 编解码
4. 实现 `intent_card.cpp` — WantParams 序列化/反序列化
5. 扩展 `intent_client.h/.cpp` — 新增两个接口 + result 解析分发
6. 更新 `BUILD.gn`

## 8. 验证方式

- 应用侧返回带 `ohos.insightIntent.card` 的 result → 小艺收到 OnCardShow
- 小艺调 ExecuteCardAction(token) → 框架解码并发起意图 → 收到 OnActionResult
- 应用返回无 card 的 result → 小艺直接收到 OnActionResult

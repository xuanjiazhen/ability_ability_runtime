#include <array>

#include "sts_ui_extension_content_session.h"
#include "ability_manager_client.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {

void NativeTerminateSelf(ani_env* env, ani_object obj)
{
    // GET_NAPI_INFO_AND_CALL(env, info, JsUIExtensionContentSession, OnTerminateSelf);
    if (env == nullptr) {
        TAG_LOGW(AAFwkTag::UI_EXT, "TerminateSelf null env");
        return;
    }
    ani_class cls;
    ani_status status = ANI_ERROR;
    status = env->FindClass("LUIExtensionContentSession;", &cls);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "TerminateSelf FindClass status : %{public}d", status);
    }
    StsUIExtensionContentSession *stsContentSession;
    ani_field stsContentSessionField;
    status = env->Class_GetField(cls, "nativeContextSession", &stsContentSessionField);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "TerminateSelf Class_GetField status : %{public}d", status);
    }
    // env->Class_GetStaticField_Long(cls, stsContextField, &stsContext);
    status = env->Object_GetField_Long(obj, stsContentSessionField, reinterpret_cast<ani_long*>(&stsContentSession));
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "TerminateSelf Object_GetField_Long status : %{public}d", status);
    }
    if (stsContentSession == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "TerminateSelf stsContentSession is null");
        return;
    }
    return stsContentSession->TerminateSelf();
}

// StsUIExtensionContentSession::StsUIExtensionContentSession(
//     sptr<AAFwk::SessionInfo> sessionInfo, sptr<Rosen::Window> uiWindow,
//     std::weak_ptr<AbilityRuntime::Context> &context,
//     std::shared_ptr<AbilityResultListeners>& abilityResultListeners)
//     : sessionInfo_(sessionInfo), uiWindow_(uiWindow), context_(context)
// {
//     listener_ = std::make_shared<UISessionAbilityResultListener>();
//     if (abilityResultListeners == nullptr || sessionInfo == nullptr) {
//         TAG_LOGE(AAFwkTag::UI_EXT, "null params");
//     } else {
//         abilityResultListeners->AddListener(sessionInfo->uiExtensionComponentId, listener_);
//     }
// }
StsUIExtensionContentSession::StsUIExtensionContentSession()
{

}

// ani_object StsUIExtensionContentSession::CreateStsUIExtensionContentSession(ani_env env,
//     sptr<AAFwk::SessionInfo> sessionInfo, sptr<Rosen::Window> uiWindow,
//     std::weak_ptr<AbilityRuntime::Context> context,
//     std::shared_ptr<AbilityResultListeners>& abilityResultListeners)
ani_object StsUIExtensionContentSession::CreateStsUIExtensionContentSession(ani_env* env)
{
    TAG_LOGE(AAFwkTag::UIABILITY, "CreateStsUIExtensionContentSession call");
    ani_object object = nullptr;
    ani_method method = nullptr;
    ani_class cls;
    ani_status status = ANI_ERROR;
    status = env->FindClass("LUIExtensionContentSession/UIExtensionContentSession;", &cls);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "CreateStsUIExtensionContentSession FindClass status : %{public}d", status);
    }
    status = env->Class_GetMethod(cls, "<ctor>", ":V", &method);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "CreateStsUIExtensionContentSession Class_GetMethod status : %{public}d", status);
    }
    status = env->Object_New(cls, method, &object);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "CreateStsUIExtensionContentSession Object_New status : %{public}d", status);
    }
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "CreateStsUIExtensionContentSession null object");
        return nullptr;
    }

    auto stsContentSession = std::make_unique<StsUIExtensionContentSession>();
    std::array methods = {
        ani_native_function {"terminateSelf", ":V", reinterpret_cast<void *>(NativeTerminateSelf)}
    };
    status = env->Class_BindNativeMethods(cls, methods.data(),methods.size());
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "CreateStsUIExtensionContentSession Class_BindNativeMethods status : %{public}d", status);
    }
    ani_field nativeField = nullptr;
    status = env->Class_GetField(cls, "nativeContextSession", &nativeField);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "CreateStsUIExtensionContentSession Class_GetField status : %{public}d", status);
    }
    status = env->Object_SetField_Long(object, nativeField, reinterpret_cast<ani_long>(stsContentSession.release()));
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "CreateStsUIExtensionContentSession Object_SetField_Long status : %{public}d", status);
    }
    TAG_LOGE(AAFwkTag::UIABILITY, "CreateStsUIExtensionContentSession call end");
    return object;
}

void StsUIExtensionContentSession::TerminateSelf()
{
    TAG_LOGE(AAFwkTag::UIABILITY, "StsUIExtensionContentSession TerminateSelf call end");
    AAFwk::AbilityManagerClient::GetInstance()->TerminateUIExtensionAbility(sessionInfo_);
}

ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    ani_env *env;
    ani_status status = ANI_ERROR;
    status = vm->GetEnv(ANI_VERSION_1, &env);
    if (ANI_OK != status) {
        TAG_LOGE(AAFwkTag::UIABILITY, "GetEnv is fail %{public}d", status);
        return ANI_ERROR;
    }

    static const char *className = "LUIExtensionContentSession/UIExtensionContentSession;";
    ani_class cls;
    status = env->FindClass(className, &cls);
    if (ANI_OK != status) {
        TAG_LOGE(AAFwkTag::UIABILITY, "FindClass is fail %{public}d", status);
        return ANI_ERROR;
    }

    std::array methods = {
        ani_native_function {"terminateSelf", ":V", reinterpret_cast<void *>(NativeTerminateSelf)}
    };

    status = env->Class_BindNativeMethods(cls, methods.data(), methods.size());
    if (ANI_OK != status) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Class_BindNativeMethods is fail %{public}d", status);
        return ANI_ERROR;
    };

    *result = ANI_VERSION_1;
    return ANI_OK;
}
}
}
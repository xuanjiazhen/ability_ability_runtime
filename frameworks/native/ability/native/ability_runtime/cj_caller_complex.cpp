/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <string>
#include <set>

#include "ability_business_error.h"
#include "ability_runtime/cj_caller_complex.h"
#include "cj_common_ffi.h"
#include "cj_lambda.h"
#include "cj_macro.h"
#include "cj_utils_ffi.h"
#include "event_handler.h"
#include "ffi_remote_data.h"
#include "hilog_tag_wrapper.h"
#include "remote_object_impl.h"

namespace OHOS {
namespace AbilityRuntime {
namespace { // nameless

constexpr int64_t INVALID_DATA_ID = -1;

char* CreateCStringFromString(const std::string& source)
{
    if (source.size() == 0) {
        return nullptr;
    }
    size_t length = source.size() + 1;
    auto res = static_cast<char*>(malloc(length));
    if (res == nullptr) {
        TAG_LOGE(AAFwkTag::DEFAULT, "null res");
        return nullptr;
    }
    if (strcpy_s(res, length, source.c_str()) != 0) {
        free(res);
        TAG_LOGE(AAFwkTag::DEFAULT, "Strcpy failed");
        return nullptr;
    }
    return res;
}

class CjCallerComplex : public FFI::FFIData {
public:
    enum class OBJSTATE {
        OBJ_NORMAL,
        OBJ_EXECUTION,
        OBJ_RELEASE
    };

    explicit CjCallerComplex(
        ReleaseCallFunc releaseCallFunc, sptr<IRemoteObject> callee,
        std::shared_ptr<CallerCallBack> callerCallBack) : releaseCallFunc_(releaseCallFunc),
        callee_(callee),
        callerCallBackObj_(callerCallBack), cjReleaseCallBackObj_(nullptr), cjRemoteStateChangedObj_(nullptr)
    {
        AddCjCallerComplex(this);
        handler_ = std::make_shared<AppExecFwk::EventHandler>(AppExecFwk::EventRunner::GetMainEventRunner());
        currentState_ = OBJSTATE::OBJ_NORMAL;
    };
    virtual~CjCallerComplex()
    {
        Finalizer(this);
        RemoveCjCallerComplex(this);
    };

    static bool ReleaseObject(CjCallerComplex* data)
    {
        TAG_LOGD(AAFwkTag::DEFAULT, "called");
        if (data == nullptr) {
            TAG_LOGE(AAFwkTag::DEFAULT, "null data");
            return false;
        }

        if (!data->ChangeCurrentState(OBJSTATE::OBJ_RELEASE)) {
            auto handler = data->GetEventHandler();
            if (handler == nullptr) {
                TAG_LOGE(AAFwkTag::DEFAULT, "null handler");
                return false;
            }
            auto releaseObjTask = [pdata = data] () {
                if (!FindCjCallerComplex(pdata)) {
                    TAG_LOGE(AAFwkTag::DEFAULT, "argc not found");
                    return;
                }
                ReleaseObject(pdata);
            };

            handler->PostTask(releaseObjTask, "FinalizerRelease");
            return false;
        } else {
            // when the object is about to be destroyed, does not reset state
            std::unique_ptr<CjCallerComplex> delObj(data);
        }
        TAG_LOGD(AAFwkTag::DEFAULT, "end");
        return true;
    }

    static void Finalizer(void* data)
    {
        TAG_LOGD(AAFwkTag::DEFAULT, "called");
        if (data == nullptr) {
            TAG_LOGE(AAFwkTag::DEFAULT, "null data");
            return;
        }

        auto ptr = static_cast<CjCallerComplex*>(data);
        if (!FindCjCallerComplex(ptr)) {
            TAG_LOGE(AAFwkTag::DEFAULT, "argc not found");
            return;
        }

        ReleaseObject(ptr);
        TAG_LOGD(AAFwkTag::DEFAULT, "end");
    }

    int32_t ReleaseCall()
    {
        TAG_LOGD(AAFwkTag::DEFAULT, "called");
        if (callerCallBackObj_ == nullptr) {
            TAG_LOGE(AAFwkTag::DEFAULT, "null CallBacker");
            return static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER);
        }
        if (!releaseCallFunc_) {
            TAG_LOGE(AAFwkTag::DEFAULT, "null releaseFunc");
            return static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER);
        }
        callee_ = nullptr;
        callerCallBackObj_->SetCallBack(nullptr);
        int32_t innerErrorCode = releaseCallFunc_(callerCallBackObj_);
        if (innerErrorCode != ERR_OK) {
            TAG_LOGE(AAFwkTag::DEFAULT, "ReleaseAbility failed %{public}d", static_cast<int>(innerErrorCode));
        }
        return innerErrorCode;
    }

    int32_t SetOnReleaseCallBack(const std::function<void(const char*)>& cjCallback)
    {
        TAG_LOGD(AAFwkTag::DEFAULT, "start");
        if (callerCallBackObj_ == nullptr) {
            TAG_LOGE(AAFwkTag::DEFAULT, "null CallBacker");
            return static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER);
        }
        cjReleaseCallBackObj_ = cjCallback;
        auto task = [notify = this] (const std::string &str) {
            if (!FindCjCallerComplexAndChangeState(notify, OBJSTATE::OBJ_EXECUTION)) {
                TAG_LOGE(AAFwkTag::DEFAULT, "address error");
                return;
            }
            notify->OnReleaseNotify(str);
        };
        callerCallBackObj_->SetOnRelease(task);
        TAG_LOGD(AAFwkTag::DEFAULT, "end");
        return ERR_OK;
    }

    int32_t SetOnRemoteStateChanged(const std::function<void(const char*)>& cjCallback)
    {
        TAG_LOGD(AAFwkTag::DEFAULT, "begin");
        if (callerCallBackObj_ == nullptr) {
            TAG_LOGE(AAFwkTag::DEFAULT, "null callBacker");
            return static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER);
        }
        cjRemoteStateChangedObj_ = cjCallback;
        auto task = [notify = this] (const std::string &str) {
            TAG_LOGI(AAFwkTag::DEFAULT, "state changed");
            if (!FindCjCallerComplexAndChangeState(notify, OBJSTATE::OBJ_EXECUTION)) {
                TAG_LOGE(AAFwkTag::DEFAULT, "address error");
                return;
            }
            notify->OnRemoteStateChangedNotify(str);
        };
        callerCallBackObj_->SetOnRemoteStateChanged(task);
        TAG_LOGD(AAFwkTag::DEFAULT, "end");
        return ERR_OK;
    }

    static bool AddCjCallerComplex(CjCallerComplex* ptr)
    {
        if (ptr == nullptr) {
            TAG_LOGE(AAFwkTag::DEFAULT, "null ptr");
            return false;
        }

        std::lock_guard<std::mutex> lck (cjCallerComplexMutex);
        auto iter = cjCallerComplexManagerList.find(ptr);
        if (iter != cjCallerComplexManagerList.end()) {
            TAG_LOGE(AAFwkTag::DEFAULT, "address exist");
            return false;
        }

        auto iterRet = cjCallerComplexManagerList.emplace(ptr);
        TAG_LOGD(AAFwkTag::DEFAULT, "retval: %{public}s", iterRet.second ? "true" : "false");
        return iterRet.second;
    }

    static bool RemoveCjCallerComplex(CjCallerComplex* ptr)
    {
        if (ptr == nullptr) {
            TAG_LOGE(AAFwkTag::DEFAULT, "null ptr");
            return false;
        }

        std::lock_guard<std::mutex> lck (cjCallerComplexMutex);
        auto iter = cjCallerComplexManagerList.find(ptr);
        if (iter == cjCallerComplexManagerList.end()) {
            TAG_LOGE(AAFwkTag::DEFAULT, "argc not found");
            return false;
        }

        cjCallerComplexManagerList.erase(ptr);
        TAG_LOGD(AAFwkTag::DEFAULT, "end");
        return true;
    }

    static bool FindCjCallerComplex(CjCallerComplex* ptr)
    {
        if (ptr == nullptr) {
            TAG_LOGE(AAFwkTag::DEFAULT, "null ptr");
            return false;
        }
        auto ret = true;
        std::lock_guard<std::mutex> lck (cjCallerComplexMutex);
        auto iter = cjCallerComplexManagerList.find(ptr);
        if (iter == cjCallerComplexManagerList.end()) {
            ret = false;
        }
        TAG_LOGD(AAFwkTag::DEFAULT, "retval %{public}s", ret ? "true" : "false");
        return ret;
    }

    static bool FindCjCallerComplexAndChangeState(CjCallerComplex* ptr, OBJSTATE state)
    {
        if (ptr == nullptr) {
            TAG_LOGE(AAFwkTag::DEFAULT, "null ptr");
            return false;
        }
        std::lock_guard<std::mutex> lck (cjCallerComplexMutex);
        auto iter = cjCallerComplexManagerList.find(ptr);
        if (iter == cjCallerComplexManagerList.end()) {
            TAG_LOGE(AAFwkTag::DEFAULT, "argc not found");
            return false;
        }
        auto ret = ptr->ChangeCurrentState(state);
        TAG_LOGD(AAFwkTag::DEFAULT, "ChangeCurrentState ret:%{public}s", ret ? "true" : "false");
        return ret;
    }

    sptr<IRemoteObject> GetRemoteObject()
    {
        return callee_;
    }

    std::shared_ptr<AppExecFwk::EventHandler> GetEventHandler()
    {
        return handler_;
    }

    bool ChangeCurrentState(OBJSTATE state)
    {
        auto ret = false;
        if (stateMechanismMutex_.try_lock() == false) {
            TAG_LOGE(AAFwkTag::DEFAULT, "mutex try_lock false");
            return ret;
        }

        if (currentState_ == OBJSTATE::OBJ_NORMAL) {
            currentState_ = state;
            ret = true;
            TAG_LOGD(AAFwkTag::DEFAULT, "currentState_:OBJ_NORMAL");
        } else if (currentState_ == state) {
            ret = true;
            TAG_LOGD(AAFwkTag::DEFAULT, "currentState_:state");
        } else {
            ret = false;
            TAG_LOGD(AAFwkTag::DEFAULT, "ret: false");
        }

        stateMechanismMutex_.unlock();
        return ret;
    }

    OBJSTATE GetCurrentState()
    {
        return currentState_;
    }

    void StateReset()
    {
        currentState_ = OBJSTATE::OBJ_NORMAL;
    }

private:

    void OnReleaseNotify(const std::string &str)
    {
        TAG_LOGD(AAFwkTag::DEFAULT, "begin");
        if (handler_ == nullptr) {
            TAG_LOGE(AAFwkTag::DEFAULT, "null handler");
            return;
        }

        auto task = [notify = this, &str] () {
            if (!FindCjCallerComplex(notify)) {
                TAG_LOGE(AAFwkTag::DEFAULT, "address error");
                return;
            }
            notify->OnReleaseNotifyTask(str);
        };
        handler_->PostSyncTask(task, "OnReleaseNotify");
        TAG_LOGD(AAFwkTag::DEFAULT, "end");
    }

    void OnReleaseNotifyTask(const std::string &str)
    {
        TAG_LOGD(AAFwkTag::DEFAULT, "begin");
        if (cjReleaseCallBackObj_ == nullptr) {
            TAG_LOGE(AAFwkTag::DEFAULT, "null cjReleaseObj");
            return;
        }
        auto cString = CreateCStringFromString(str);
        cjReleaseCallBackObj_(cString);
        free(cString);
        callee_ = nullptr;
        StateReset();
    }

    void OnRemoteStateChangedNotify(const std::string &str)
    {
        TAG_LOGD(AAFwkTag::DEFAULT, "begin");
        if (handler_ == nullptr) {
            TAG_LOGE(AAFwkTag::DEFAULT, "null handler");
            return;
        }

        auto task = [notify = this, &str] () {
            if (!FindCjCallerComplex(notify)) {
                TAG_LOGE(AAFwkTag::DEFAULT, "ptr not found");
                return;
            }
            notify->OnRemoteStateChangedNotifyTask(str);
        };
        handler_->PostSyncTask(task, "OnRemoteStateChangedNotify");
        TAG_LOGD(AAFwkTag::DEFAULT, "end");
    }

    void OnRemoteStateChangedNotifyTask(const std::string &str)
    {
        TAG_LOGD(AAFwkTag::DEFAULT, "begin");
        if (cjRemoteStateChangedObj_ == nullptr) {
            TAG_LOGE(AAFwkTag::DEFAULT, "null cjRemoteStateChangedObj");
            return;
        }
        auto cString = CreateCStringFromString(str);
        cjRemoteStateChangedObj_(cString);
        free(cString);
        StateReset();
        TAG_LOGD(AAFwkTag::DEFAULT, "end");
    }

private:
    ReleaseCallFunc releaseCallFunc_;
    sptr<IRemoteObject> callee_;

    std::shared_ptr<CallerCallBack> callerCallBackObj_;
    std::function<void(const char*)> cjReleaseCallBackObj_;
    std::function<void(const char*)> cjRemoteStateChangedObj_;
    std::shared_ptr<AppExecFwk::EventHandler> handler_;
    std::mutex stateMechanismMutex_;
    OBJSTATE currentState_;

    static std::set<CjCallerComplex*> cjCallerComplexManagerList;
    static std::mutex cjCallerComplexMutex;
};

std::set<CjCallerComplex*> CjCallerComplex::cjCallerComplexManagerList;
std::mutex CjCallerComplex::cjCallerComplexMutex;

} // nameless

int32_t CreateCjCallerComplex(
    ReleaseCallFunc releaseCallFunc, sptr<IRemoteObject> callee,
    std::shared_ptr<CallerCallBack> callerCallBack, int64_t* callerId, RetDataI64* remoteId)
{
    TAG_LOGD(AAFwkTag::DEFAULT, "begin");
    if (callee == nullptr || callerCallBack == nullptr || releaseCallFunc == nullptr) {
        TAG_LOGE(AAFwkTag::DEFAULT, "%{public}s null",
            (callee == nullptr) ? ("callee") :
            ((releaseCallFunc == nullptr) ? ("releaseCallFunc") : ("callerCallBack")));
        return static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER);
    }

    auto cjCaller = FFI::FFIData::Create<CjCallerComplex>(releaseCallFunc, callee, callerCallBack);
    auto remoteObj = cjCaller->GetRemoteObject();
    if (remoteObj == nullptr) {
        TAG_LOGE(AAFwkTag::DEFAULT, "null remoteObj");
        return static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER);
    }
    if (remoteId == nullptr || callerId == nullptr) {
        return static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER);
    }
    *remoteId = CreateCjCalleeRemoteObject(remoteObj);
    *callerId = cjCaller->GetID();
    TAG_LOGD(AAFwkTag::DEFAULT, "end");
    return SUCCESS_CODE;
}

RetDataI64 CreateCjCalleeRemoteObject(sptr<IRemoteObject> callee)
{
    if (callee == nullptr) {
        TAG_LOGE(AAFwkTag::DEFAULT, "null callee");
        return RetDataI64{0, INVALID_DATA_ID};
    }
    return CJ_rpc_CreateRemoteObject(callee);
}

extern "C" {
CJ_EXPORT int32_t FFIAbilityCallerRelease(int64_t id)
{
    auto cjCallerComplex = OHOS::FFI::FFIData::GetData<CjCallerComplex>(id);
    if (cjCallerComplex == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "GetCjCallerComplex failed, caller is nullptr");
        return static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER);
    }
    return cjCallerComplex->ReleaseCall();
}

CJ_EXPORT int32_t FFIAbilityCallerOnRelease(int64_t id, int64_t callbackId)
{
    auto cjCallerComplex = OHOS::FFI::FFIData::GetData<CjCallerComplex>(id);
    if (cjCallerComplex == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "GetCjCallerComplex failed, caller is nullptr");
        return static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER);
    }
    auto callback = CJLambda::Create(reinterpret_cast<void (*)(const char*)>(callbackId));
    return cjCallerComplex->SetOnReleaseCallBack(callback);
}

CJ_EXPORT int32_t FFIAbilityCallerOnRemoteStateChange(int64_t id, int64_t callbackId)
{
    auto cjCallerComplex = OHOS::FFI::FFIData::GetData<CjCallerComplex>(id);
    if (cjCallerComplex == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "GetCjCallerComplex failed, caller is nullptr");
        return static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER);
    }
    auto callback = CJLambda::Create(reinterpret_cast<void (*)(const char*)>(callbackId));
    return cjCallerComplex->SetOnRemoteStateChanged(callback);
}

}
} // AbilityRuntime
} // OHOS

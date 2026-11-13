/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "ui_extension_record.h"

#include "ability_util.h"
#include "session/host/include/zidl/session_interface.h"

namespace OHOS {
namespace AbilityRuntime {
UIExtensionRecord::UIExtensionRecord(const std::shared_ptr<AAFwk::BaseExtensionRecord> &abilityRecord)
    : ExtensionRecord(abilityRecord)
{}

UIExtensionRecord::~UIExtensionRecord() = default;

bool UIExtensionRecord::ContinueToGetCallerToken()
{
    return true;
}

void UIExtensionRecord::Update(const AAFwk::AbilityRequest &abilityRequest)
{
    if (abilityRecord_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null abilityRecord_");
        return;
    }
    // Migration re-delivery (at-least-once), paired with the same logic in
    // UIExtensionAbilityManager::StartAbilityLocked: while a migration is unconfirmed (its
    // declaration kept in the record), any reuse request carries the full declaration so the
    // provider keeps getting re-parent chances; cleared on confirmation (done receipt).
    if (abilityRequest.sessionInfo != nullptr &&
        abilityRequest.sessionInfo->originalCallerInfo.callerToken == nullptr) {
        auto unconfirmedInfo = abilityRecord_->GetSessionInfo();
        if (unconfirmedInfo != nullptr && unconfirmedInfo->originalCallerInfo.callerToken != nullptr) {
            abilityRequest.sessionInfo->originalCallerInfo = unconfirmedInfo->originalCallerInfo;
        }
    }
    if (abilityRequest.sessionInfo != nullptr) {
        // Gen monotonicity guard: never roll the record's migration gen back on a plain
        // (non-migration) reuse/preload update — a rolled-back gen makes the next migration
        // reuse an already-consumed generation, which the provider's monotonic fencing
        // silently skips. Paired with UIExtensionAbilityManager::StartAbilityLocked.
        auto oldInfo = abilityRecord_->GetSessionInfo();
        if (oldInfo != nullptr && abilityRequest.sessionInfo->originalCallerInfo.migrationGen <
            oldInfo->originalCallerInfo.migrationGen) {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "migration gen rollback on record update, keep gen: %{public}u",
                oldInfo->originalCallerInfo.migrationGen);
            abilityRequest.sessionInfo->originalCallerInfo.migrationGen =
                oldInfo->originalCallerInfo.migrationGen;
        }
    }
    abilityRecord_->SetSessionInfo(abilityRequest.sessionInfo);
}

void UIExtensionRecord::HandleNotifyUIExtensionTimeout(ErrorCode code)
{
    CHECK_POINTER(abilityRecord_);
    auto sessionInfo = abilityRecord_->GetSessionInfo();
    CHECK_POINTER(sessionInfo);
    sptr<Rosen::ISession> sessionProxy = iface_cast<Rosen::ISession>(sessionInfo->sessionToken);
    if (sessionProxy == nullptr) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Parsing session failed, is nullptr.");
        return;
    }
    sessionProxy->NotifyExtensionTimeout(code);
}

void UIExtensionRecord::LoadTimeout()
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    HandleNotifyUIExtensionTimeout(ErrorCode::LOAD_TIMEOUT);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Notify wms, the uiextension load time out.");
}

void UIExtensionRecord::ForegroundTimeout()
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    HandleNotifyUIExtensionTimeout(ErrorCode::FOREGROUND_TIMEOUT);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Notify wms, the uiextension move foreground time out.");
}

void UIExtensionRecord::BackgroundTimeout()
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    HandleNotifyUIExtensionTimeout(ErrorCode::BACKGROUND_TIMEOUT);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Notify wms, the uiextension move background time out.");
}

void UIExtensionRecord::TerminateTimeout()
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    HandleNotifyUIExtensionTimeout(ErrorCode::TERMINATE_TIMEOUT);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Notify wms, the uiextension terminate time out.");
}
} // namespace AbilityRuntime
} // namespace OHOS

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

#include <cstdint>

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
    auto delivery = BuildMigrationDeliverySessionInfo();
    if (delivery != nullptr) {
        if (abilityRequest.sessionInfo != nullptr) {
            // A queued request may belong to an older migration generation. Replace the entire
            // delivery carrier so its window, token, display and migration fence come from one target.
            *abilityRequest.sessionInfo = *delivery;
        }
        return;
    }
    if (abilityRequest.sessionInfo != nullptr &&
        abilityRequest.sessionInfo->originalCallerInfo.migrationGen != 0) {
        abilityRequest.sessionInfo->originalCallerInfo.migrationGen = 0;
        abilityRequest.sessionInfo->originalCallerInfo.callerToken = nullptr;
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

uint32_t UIExtensionRecord::AdvanceMigrationGen()
{
    std::lock_guard<std::mutex> lock(migrationMutex_);
    if (migrationGen_ == UINT32_MAX) {
        return 0;
    }
    return ++migrationGen_;
}

uint32_t UIExtensionRecord::DeclareMigration(const AAFwk::OriginalCallerInfo &declaration,
    uint32_t targetHostWindowId, const sptr<AAFwk::SessionInfo> &deliverySessionInfo)
{
    if (deliverySessionInfo == nullptr) {
        return 0;
    }
    auto delivery = sptr<AAFwk::SessionInfo>::MakeSptr(*deliverySessionInfo);
    if (delivery == nullptr) {
        return 0;
    }
    std::lock_guard<std::mutex> lock(migrationMutex_);
    if (migrationGen_ == UINT32_MAX) {
        return 0;
    }
    migrationDeclaration_ = declaration;
    migrationDeclaration_.migrationGen = ++migrationGen_;
    migrationDeclaration_.callerToken = nullptr;
    migrationTargetHostWindowId_ = targetHostWindowId;
    delivery->originalCallerInfo = migrationDeclaration_;
    delivery->hostWindowId = migrationTargetHostWindowId_;
    migrationDeliverySessionInfo_ = delivery;
    return migrationGen_;
}

void UIExtensionRecord::SetMigrationDeclaration(const AAFwk::OriginalCallerInfo &declaration,
    uint32_t targetHostWindowId)
{
    std::lock_guard<std::mutex> lock(migrationMutex_);
    migrationDeclaration_ = declaration;
    migrationTargetHostWindowId_ = targetHostWindowId;
}

void UIExtensionRecord::SetMigrationDeliverySessionInfo(const sptr<AAFwk::SessionInfo> &deliverySessionInfo)
{
    std::lock_guard<std::mutex> lock(migrationMutex_);
    migrationDeliverySessionInfo_ = deliverySessionInfo == nullptr ?
        nullptr : sptr<AAFwk::SessionInfo>::MakeSptr(*deliverySessionInfo);
}

bool UIExtensionRecord::GetMigrationDeclaration(AAFwk::OriginalCallerInfo &declaration,
    uint32_t &targetHostWindowId)
{
    std::lock_guard<std::mutex> lock(migrationMutex_);
    if (migrationDeclaration_.migrationGen == 0) {
        return false;
    }
    declaration = migrationDeclaration_;
    targetHostWindowId = migrationTargetHostWindowId_;
    return true;
}

bool UIExtensionRecord::ConfirmMigrationDeclaration(uint32_t gen, int32_t persistentId)
{
    std::lock_guard<std::mutex> lock(migrationMutex_);
    if (migrationDeclaration_.migrationGen == 0 || gen != migrationDeclaration_.migrationGen) {
        return false;
    }
    if (persistentId != migrationDeclaration_.persistentId) {
        return false;
    }
    migrationDeclaration_.migrationGen = 0;
    confirmedHostWindowId_ = migrationTargetHostWindowId_;
    if (migrationDeliverySessionInfo_ != nullptr && abilityRecord_ != nullptr) {
        auto factSession = sptr<AAFwk::SessionInfo>::MakeSptr(*migrationDeliverySessionInfo_);
        if (factSession != nullptr) {
            factSession->originalCallerInfo.migrationGen = 0;
            abilityRecord_->SetSessionInfo(factSession);
        }
        migrationDeliverySessionInfo_ = nullptr;
    }
    return true;
}

sptr<AAFwk::SessionInfo> UIExtensionRecord::BuildMigrationDeliverySessionInfo()
{
    std::lock_guard<std::mutex> lock(migrationMutex_);
    if (migrationDeliverySessionInfo_ == nullptr || migrationDeclaration_.migrationGen == 0) {
        return nullptr;
    }
    auto delivery = sptr<AAFwk::SessionInfo>::MakeSptr(*migrationDeliverySessionInfo_);
    if (delivery != nullptr) {
        delivery->originalCallerInfo = migrationDeclaration_;
        delivery->originalCallerInfo.callerToken = nullptr;
        delivery->hostWindowId = migrationTargetHostWindowId_;
    }
    return delivery;
}

uint32_t UIExtensionRecord::GetMigrationGen()
{
    std::lock_guard<std::mutex> lock(migrationMutex_);
    return migrationGen_;
}

uint32_t UIExtensionRecord::GetConfirmedHostWindowId()
{
    std::lock_guard<std::mutex> lock(migrationMutex_);
    return confirmedHostWindowId_;
}

void UIExtensionRecord::SeedConfirmedHostWindowId(uint32_t factHostWindowId)
{
    std::lock_guard<std::mutex> lock(migrationMutex_);
    if (factHostWindowId == 0 || confirmedHostWindowId_ != 0 ||
        migrationGen_ != 0 || migrationDeclaration_.migrationGen != 0) {
        return;
    }
    confirmedHostWindowId_ = factHostWindowId;
}
} // namespace AbilityRuntime
} // namespace OHOS

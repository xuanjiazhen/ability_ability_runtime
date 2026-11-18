/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_EXTENSION_RECORD_H
#define OHOS_ABILITY_RUNTIME_EXTENSION_RECORD_H

#include <atomic>
#include <map>
#include <memory>
#include <string>

#include "base_extension_record.h"
#include "preload_uiext_state_observer.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr int32_t INVALID_EXTENSION_RECORD_ID = 0;
}
class ExtensionRecord : public std::enable_shared_from_this<ExtensionRecord> {
public:
    ExtensionRecord(const std::shared_ptr<AAFwk::BaseExtensionRecord> &abilityRecord);

    virtual ~ExtensionRecord();

    // ===== Host-window migration (UIExtension-only capability) =====
    // never migrates and keeps the zero-cost defaults.

    /**
     * Advance the migration gen by 1 for a new migration.
     * @return The new gen, or 0 when migration is unsupported or saturated (reject).
     */
    virtual uint32_t AdvanceMigrationGen()
    {
        return 0;
    }

    /**
     * @brief Declare an unconfirmed migration (marker on) until gen-matched confirmation.
     */
    virtual void SetMigrationDeclaration(const AAFwk::OriginalCallerInfo &declaration,
        uint32_t targetHostWindowId) {}

    /**
     * @brief Remember the delivered session of the pending migration. On gen-matched
     *        confirmation the record's factual sessionInfo is advanced by adopting a
     *        sanitized copy of it (fact/intent separation: the intent only ever lives in
     *        the delivery, never in the record's sessionInfo).
     */
    virtual void SetMigrationDeliverySessionInfo(const sptr<AAFwk::SessionInfo> &deliverySessionInfo) {}

    /**
     * @brief At-least-once projection source.
     * @return true and fills the outputs when a migration is pending; false otherwise.
     */
    virtual bool GetMigrationDeclaration(AAFwk::OriginalCallerInfo &declaration,
        uint32_t &targetHostWindowId) const
    {
        return false;
    }

    /**
     * @brief Confirm-clear by gen: only the CURRENT generation's completion may clear.
     * @return true when the marker existed and the gen matched (marker cleared).
     */
    virtual bool ConfirmMigrationDeclaration(uint32_t gen)
    {
        return false;
    }

    /** Current migration gen (diagnosis/logging). */
    virtual uint32_t GetMigrationGen() const
    {
        return 0;
    }

    /**
     * @brief Last CONFIRMED host window (0 = never migrated). The unchanged-window gate
     *        compares against this, NOT against the record's sessionInfo (which is only
     *        advanced on confirmation), so an unconfirmed re-declaration of the same
     *        target (at-least-once retry) is never blocked by the window gate.
     */
    virtual uint32_t GetConfirmedHostWindowId() const
    {
        return 0;
    }

    /**
     * @brief Seed the confirmed host window from the record's factual sessionInfo before
     *        the first migration, so a first "migration" to the current window is rejected
     *        instead of consuming a redundant gen. One-shot: no effect once a migration
     *        has been declared or confirmed.
     */
    virtual void SeedConfirmedHostWindowId(uint32_t factHostWindowId) {}

    sptr<IRemoteObject> GetCallToken() const;

    sptr<IRemoteObject> GetRootCallerToken() const;

    void SetRootCallerToken(sptr<IRemoteObject> &rootCallerToken);

    sptr<IRemoteObject> GetFocusedCallerToken() const;

    void SetFocusedCallerToken(sptr<IRemoteObject> &rootCallerToken);

    virtual bool ContinueToGetCallerToken();

    virtual void Update(const AAFwk::AbilityRequest &abilityRequest);

    void UnloadUIExtensionAbility();

    int32_t RegisterStateObserver(const std::string &hostBundleName);

    bool isHostSpecified_ = false;
    int32_t extensionRecordId_ = INVALID_EXTENSION_RECORD_ID;
    int32_t requestCode_ = -1;
    uint32_t processMode_ = 0;
    pid_t hostPid_ = 0;
    std::shared_ptr<AAFwk::BaseExtensionRecord> abilityRecord_ = nullptr;
    std::string hostBundleName_;
private:
    sptr<IRemoteObject> rootCallerToken_ = nullptr;
    sptr<IRemoteObject> focusedCallerToken_ = nullptr;
    sptr<AAFwk::PreLoadUIExtStateObserver> preLoadUIExtStateObserver_;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_EXTENSION_RECORD_H

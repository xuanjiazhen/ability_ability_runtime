#ifndef OHOS_AGENT_RUNTIME_AGENT_CARD_UTILS_H
#define OHOS_AGENT_RUNTIME_AGENT_CARD_UTILS_H

#include <string>
#include <vector>

#include "agent_card.h"
#include "bundle_mgr_client.h"

namespace OHOS {
namespace AgentRuntime {
class AgentCardUtils final {
public:
    static int32_t ValidateBundleAbility(const std::string &bundleName,
        const std::string &abilityName, int32_t userId);
    static bool IsCardOwnedByAbility(const AgentCard &card, const std::string &bundleName,
        const std::string &abilityName);
    static bool HasRequiredRegisterFields(const AgentCard &card);
    static bool ShouldValidateAppInfo(const AgentCard &card);
    static void ApplyDeviceTypes(const std::vector<std::string> &hapDeviceTypes, AgentCard &card);
    static bool ShouldKeepStoredCard(const AgentCard &incomingCard, const AgentCard &storedCard);
    static std::vector<std::string> FindHapDeviceTypes(const AppExecFwk::BundleInfo &bundleInfo,
        const std::string &moduleName);
};
} // namespace AgentRuntime
} // namespace OHOS

#endif // OHOS_AGENT_RUNTIME_AGENT_CARD_UTILS_H

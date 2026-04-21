#ifndef MOCK_MODULAR_OBJECT_RDB_STORAGE_MGR_H
#define MOCK_MODULAR_OBJECT_RDB_STORAGE_MGR_H

#include <string>
#include <vector>
#include "mock_flag.h"
#include "modular_object_extension_info.h"

namespace OHOS {
namespace AbilityRuntime {

class ModularObjectExtensionRdbStorageMgr
    : public std::enable_shared_from_this<ModularObjectExtensionRdbStorageMgr> {
    DECLARE_DELAYED_SINGLETON(ModularObjectExtensionRdbStorageMgr);
public:
    int32_t QueryData(const std::string &key, std::vector<AAFwk::ModularObjectExtensionInfo> &infos)
    {
        if (MockFlag::queryDataRet != 0) {
            return MockFlag::queryDataRet;
        }
        if (MockFlag::extensionFound) {
            AAFwk::ModularObjectExtensionInfo info;
            info.bundleName = "com.test.bundle";
            info.abilityName = "TestAbility";
            info.isDisabled = MockFlag::extensionDisabled;
            infos.push_back(info);
        }
        return 0;
    }
    int32_t InsertOrUpdateData(const std::string &key,
        const std::vector<AAFwk::ModularObjectExtensionInfo> &infos, uint32_t versionCode)
    {
        return 0;
    }
    int32_t DeleteData(const std::string &key) { return 0; }
    bool QueryVersion(const std::string &key, uint32_t &versionCode) { return false; }
};

} // namespace AbilityRuntime
} // namespace OHOS

// Bring into AAFwk namespace for source compatibility
namespace OHOS {
namespace AAFwk {
using AbilityRuntime::ModularObjectExtensionRdbStorageMgr;
} // namespace AAFwk
} // namespace OHOS

#endif // MOCK_MODULAR_OBJECT_RDB_STORAGE_MGR_H

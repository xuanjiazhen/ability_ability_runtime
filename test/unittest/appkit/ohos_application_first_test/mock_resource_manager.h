#ifndef MOCK_RESOURCE_MANAGER_H
#define MOCK_RESOURCE_MANAGER_H

#include <gmock/gmock.h>
#include "resource_manager.h"

using namespace OHOS::Global::Resource;

class MockResourceManager : public ResourceManager {
public:
    MOCK_METHOD(bool, AddResource, (const char *path, const uint32_t &selectedTypes, bool forceReload), (override));
    MOCK_METHOD(RState, UpdateResConfig, (ResConfig &resConfig, bool isUpdateTheme), (override));
    MOCK_METHOD(void, GetResConfig, (ResConfig &resConfig), (override));
    MOCK_METHOD(RState, GetResConfigById, (uint32_t resId, ResConfig &resConfig, uint32_t density), (override));
    MOCK_METHOD(RState, GetResConfigByName, (const std::string &name, const ResType type, 
        ResConfig &resConfig, uint32_t density), (override));
    MOCK_METHOD(RState, GetStringById, (uint32_t id, std::string &outValue), (override));
    MOCK_METHOD(RState, GetStringByName, (const char *name, std::string &outValue), (override));
    MOCK_METHOD(RState, GetStringFormatById, (std::string &outValue, uint32_t id, ...), (override));
    MOCK_METHOD(RState, GetStringFormatByName, (std::string &outValue, const char *name, ...), (override));
    MOCK_METHOD(RState, GetStringArrayById, (uint32_t id, std::vector<std::string> &outValue), (override));
    MOCK_METHOD(RState, GetStringArrayByName, (const char *name, std::vector<std::string> &outValue), (override));
    MOCK_METHOD(RState, GetPatternById, (uint32_t id, std::map<std::string, std::string> &outValue), (override));
    MOCK_METHOD(RState, GetPatternByName, (const char *name, std::map<std::string, std::string> &outValue), (override));
    MOCK_METHOD(RState, GetPluralStringById, (uint32_t id, int quantity, std::string &outValue), (override));
    MOCK_METHOD(RState, GetPluralStringByName, (const char *name, int quantity, std::string &outValue), (override));
    MOCK_METHOD(RState, GetPluralStringByIdFormat, (std::string &outValue, uint32_t id, int quantity, ...), (override));
    MOCK_METHOD(RState, GetPluralStringByNameFormat, (std::string &outValue, const char *name, int quantity, ...), (override));
    MOCK_METHOD(RState, GetThemeById, (uint32_t id, std::map<std::string, std::string> &outValue), (override));
    MOCK_METHOD(RState, GetThemeByName, (const char *name, std::map<std::string, std::string> &outValue), (override));
    MOCK_METHOD(RState, GetBooleanById, (uint32_t id, bool &outValue), (override));
    MOCK_METHOD(RState, GetBooleanByName, (const char *name, bool &outValue), (override));
    MOCK_METHOD(RState, GetIntegerById, (uint32_t id, int &outValue), (override));
    MOCK_METHOD(RState, GetIntegerByName, (const char *name, int &outValue), (override));
    MOCK_METHOD(RState, GetFloatById, (uint32_t id, float &outValue), (override));
    MOCK_METHOD(RState, GetFloatById, (uint32_t id, float &outValue, std::string &unit), (override));
    MOCK_METHOD(RState, GetFloatByName, (const char *name, float &outValue), (override));
    MOCK_METHOD(RState, GetFloatByName, (const char *name, float &outValue, std::string &unit), (override));
    MOCK_METHOD(RState, GetIntArrayById, (uint32_t id, std::vector<int> &outValue), (override));
    MOCK_METHOD(RState, GetIntArrayByName, (const char *name, std::vector<int> &outValue), (override));
    MOCK_METHOD(RState, GetColorById, (uint32_t id, uint32_t &outValue), (override));
    MOCK_METHOD(RState, GetColorByName, (const char *name, uint32_t &outValue), (override));
    MOCK_METHOD(RState, GetProfileById, (uint32_t id, std::string &outValue), (override));
    MOCK_METHOD(RState, GetProfileByName, (const char *name, std::string &outValue), (override));
    MOCK_METHOD(RState, GetMediaById, (uint32_t id, std::string &outValue, uint32_t density), (override));
    MOCK_METHOD(RState, GetMediaByName, (const char *name, std::string &outValue, uint32_t density), (override));
    MOCK_METHOD(RState, GetRawFilePathByName, (const std::string &name, std::string &outValue), (override));
    MOCK_METHOD(RState, GetRawFileDescriptor, (const std::string &name, RawFileDescriptor &descriptor), (override));
    MOCK_METHOD(RState, CloseRawFileDescriptor, (const std::string &name), (override));
    MOCK_METHOD(RState, GetMediaDataById, (uint32_t id, size_t& len, std::unique_ptr<uint8_t[]> &outValue,
        uint32_t density), (override));
    MOCK_METHOD(RState, GetMediaDataByName, (const char *name, size_t& len, std::unique_ptr<uint8_t[]> &outValue,
        uint32_t density), (override));
    MOCK_METHOD(RState, GetMediaBase64DataById, (uint32_t id,  std::string &outValue, uint32_t density), (override));
    MOCK_METHOD(RState, GetMediaBase64DataByName, (const char *name,  std::string &outValue, uint32_t density), (override));
    MOCK_METHOD(RState, GetProfileDataById, (uint32_t id, size_t &len, std::unique_ptr<uint8_t[]> &outValue), (override));
    MOCK_METHOD(RState, GetProfileDataByName, (const char *name, size_t &len, std::unique_ptr<uint8_t[]> &outValue), (override));
    MOCK_METHOD(RState, GetRawFileFromHap, (const std::string &rawFileName, size_t &len,
        std::unique_ptr<uint8_t[]> &outValue), (override));
    MOCK_METHOD(RState, GetRawFileDescriptorFromHap, (const std::string &rawFileName, RawFileDescriptor &descriptor), (override));
    MOCK_METHOD(RState, IsLoadHap, (std::string &hapPath), (override));
    MOCK_METHOD(RState, GetRawFileList, (const std::string &rawDirPath, std::vector<std::string> &rawfileList), (override));
    MOCK_METHOD(RState, GetDrawableInfoById, (uint32_t id, std::string &type, size_t &len,
        std::unique_ptr<uint8_t[]> &outValue, uint32_t density), (override));
    MOCK_METHOD(RState, GetDrawableInfoByName, (const char *name, std::string &type, size_t &len,
        std::unique_ptr<uint8_t[]> &outValue, uint32_t density), (override));
    MOCK_METHOD(bool, AddResource, (const std::string &path, const std::vector<std::string> &overlayPaths), (override));
    MOCK_METHOD(bool, RemoveResource, (const std::string &path, const std::vector<std::string> &overlayPaths), (override));
    MOCK_METHOD(RState, GetStringFormatById, (uint32_t id, std::string &outValue,
        std::vector<std::tuple<NapiValueType, std::string>> &jsParams), (override));
    MOCK_METHOD(RState, GetStringFormatByName, (const char *name, std::string &outValue,
        std::vector<std::tuple<NapiValueType, std::string>> &jsParams), (override));
    MOCK_METHOD(uint32_t, GetResourceLimitKeys, (), (override));
    MOCK_METHOD(bool, AddAppOverlay, (const std::string &path), (override));
    MOCK_METHOD(bool, RemoveAppOverlay, (const std::string &path), (override));
    MOCK_METHOD(RState, GetRawFdNdkFromHap, (const std::string &rawFileName, RawFileDescriptor &descriptor), (override));
    MOCK_METHOD(RState, GetResId, (const std::string &resTypeName, uint32_t &resId), (override));
    MOCK_METHOD(void, GetLocales, (std::vector<std::string> &outValue, bool includeSystem), (override));
    MOCK_METHOD(RState, GetDrawableInfoById, (uint32_t id,
        std::tuple<std::string, size_t, std::string> &drawableInfo,
        std::unique_ptr<uint8_t[]> &outValue, uint32_t iconType, uint32_t density), (override));
    MOCK_METHOD(RState, GetDrawableInfoByName, (const char *name,
        std::tuple<std::string, size_t, std::string> &drawableInfo,
        std::unique_ptr<uint8_t[]> &outValue, uint32_t iconType, uint32_t density), (override));
    MOCK_METHOD(RState, GetSymbolById, (uint32_t id, uint32_t &outValue), (override));
    MOCK_METHOD(RState, GetSymbolByName, (const char *name, uint32_t &outValue), (override));
    MOCK_METHOD(RState, GetThemeIcons, (uint32_t resId, std::pair<std::unique_ptr<uint8_t[]>, size_t> &foregroundInfo,
        std::pair<std::unique_ptr<uint8_t[]>, size_t> &backgroundInfo, uint32_t density,
        const std::string &abilityName), (override));
    MOCK_METHOD(std::string, GetThemeMask, (), (override));
    MOCK_METHOD(bool, HasIconInTheme, (const std::string &bundleName), (override));
    MOCK_METHOD(RState, GetOtherIconsInfo, (const std::string &iconName,
        std::unique_ptr<uint8_t[]> &outValue, size_t &len, bool isGlobalMask), (override));
    MOCK_METHOD(RState, IsRawDirFromHap, (const std::string &pathName, bool &outValue), (override));
    MOCK_METHOD(std::shared_ptr<ResourceManager>, GetOverrideResourceManager,
        (std::shared_ptr<ResConfig> overrideResConfig), (override));
    MOCK_METHOD(RState, UpdateOverrideResConfig, (ResConfig &resConfig), (override));
    MOCK_METHOD(void, GetOverrideResConfig, (ResConfig &resConfig), (override));
    MOCK_METHOD(RState, GetDynamicIcon, (const std::string &resName, std::pair<std::unique_ptr<uint8_t[]>, size_t> &iconInfo,
        uint32_t density), (override));
    MOCK_METHOD(RState, GetStringFormatById, (std::string &outValue, uint32_t id, va_list args), (override));
    MOCK_METHOD(RState, GetStringFormatByName, (std::string &outValue, const char *name, va_list args), (override));
    MOCK_METHOD(RState, GetFormatPluralStringById, (std::string &outValue, uint32_t id, int quantity,
        std::vector<std::tuple<ResourceManager::NapiValueType, std::string>> &jsParams), (override));
    MOCK_METHOD(RState, GetFormatPluralStringByName, (std::string &outValue, const char *name, int quantity,
        std::vector<std::tuple<ResourceManager::NapiValueType, std::string>> &jsParams), (override));
    MOCK_METHOD(bool, AddPatchResource, (const char *path, const char *patchPath), (override));
    MOCK_METHOD(RState, GetThemeDataByName, (const char *name, std::map<std::string, ResData> &outValue), (override));
    MOCK_METHOD(RState, GetThemeDataById, (uint32_t id, std::map<std::string, ResData> &outValue), (override));
    MOCK_METHOD(RState, GetPatternDataById, (uint32_t id, std::map<std::string, ResData> &outValue), (override));
    MOCK_METHOD(RState, GetPatternDataByName, (const char *name, std::map<std::string, ResData> &outValue), (override));
    MOCK_METHOD(RState, GetFormatPluralStringById, (std::string &outValue, uint32_t id, Quantity quantity,
        va_list args), (override));
    MOCK_METHOD(RState, GetFormatPluralStringById, (std::string &outValue, uint32_t id, Quantity quantity,
        std::vector<std::tuple<ResourceManager::NapiValueType, std::string>> &jsParams), (override));
    MOCK_METHOD(RState, GetFormatPluralStringByName, (std::string &outValue, const char *name, Quantity quantity,
        va_list args), (override));
    MOCK_METHOD(RState, GetFormatPluralStringByName, (std::string &outValue, const char *name, Quantity quantity,
        std::vector<std::tuple<ResourceManager::NapiValueType, std::string>> &jsParams), (override));
};

#endif // MOCK_RESOURCE_MANAGER_H

/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 */

#ifndef MOCK_ELEMENT_NAME_H
#define MOCK_ELEMENT_NAME_H

#include <string>

namespace OHOS {
namespace AppExecFwk {

class ElementName {
public:
    ElementName() = default;
    ElementName(const std::string &deviceId, const std::string &bundleName,
        const std::string &abilityName)
        : bundleName_(bundleName), abilityName_(abilityName) {}
    ElementName(const std::string &deviceId, const std::string &bundleName,
        const std::string &moduleName, const std::string &abilityName)
        : bundleName_(bundleName), moduleName_(moduleName), abilityName_(abilityName) {}

    std::string GetBundleName() const { return bundleName_; }
    std::string GetModuleName() const { return moduleName_; }
    std::string GetAbilityName() const { return abilityName_; }
    void SetBundleName(const std::string &name) { bundleName_ = name; }
    void SetModuleName(const std::string &name) { moduleName_ = name; }
    void SetAbilityName(const std::string &name) { abilityName_ = name; }

private:
    std::string bundleName_;
    std::string moduleName_;
    std::string abilityName_;
};

} // namespace AppExecFwk
} // namespace OHOS

#endif // MOCK_ELEMENT_NAME_H

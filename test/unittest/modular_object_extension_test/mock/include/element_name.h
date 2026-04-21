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
    ElementName(const std::string &device, const std::string &bundle,
        const std::string &module, const std::string &ability)
        : bundleName_(bundle), moduleName_(module), abilityName_(ability) {}

    std::string GetBundleName() const { return bundleName_; }
    std::string GetModuleName() const { return moduleName_; }
    std::string GetAbilityName() const { return abilityName_; }
    void SetBundleName(const std::string &s) { bundleName_ = s; }
    void SetModuleName(const std::string &s) { moduleName_ = s; }
    void SetAbilityName(const std::string &s) { abilityName_ = s; }

private:
    std::string bundleName_;
    std::string moduleName_;
    std::string abilityName_;
};

} // namespace AppExecFwk
} // namespace OHOS

#endif // MOCK_ELEMENT_NAME_H

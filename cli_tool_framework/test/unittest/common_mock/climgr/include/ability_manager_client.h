/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

#ifndef OHOS_ABILITY_RUNTIME_CLI_MGR_SERVICE_MOCK_ABILITY_MANAGER_CLIENT_H
#define OHOS_ABILITY_RUNTIME_CLI_MGR_SERVICE_MOCK_ABILITY_MANAGER_CLIENT_H

#include <memory>
#include <string>

#include "errors.h"
#include "skill/skill_execute_callback_stub.h"
#include "skill/skill_execute_param.h"

namespace OHOS {
namespace AAFwk {
class AbilityManagerClient {
public:
    static std::shared_ptr<AbilityManagerClient> GetInstance();
    ErrCode ExecuteInAppSkillWithTokenId(const AppExecFwk::SkillExecuteRequest &request,
        const sptr<ISkillExecuteCallback> &callback);
    ErrCode QuerySkillType(const std::string &bundleName, const std::string &moduleName,
        const std::string &skillName, int32_t &skillType);
};
} // namespace AAFwk
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_CLI_MGR_SERVICE_MOCK_ABILITY_MANAGER_CLIENT_H

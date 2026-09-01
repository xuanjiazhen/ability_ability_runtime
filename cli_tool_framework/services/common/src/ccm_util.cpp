/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "ccm_util.h"

#include <cstring>

#include "hilog_tag_wrapper.h"
#include "parameter.h"
#include "parameters.h"

namespace OHOS {
namespace CliTool {
namespace {
constexpr const char* MAX_CLI_QUANTITY = "const.sys.cli.limit_maximum_concurrency_quantity";
constexpr const char* CLI_PUBLIC_CMD_ENABLED = "persist.sys.cli.public_cmd_enabled";

bool IsPcOr2in1()
{
    const int bufLen = 128;
    char buf[bufLen] = {0};
    int ret = GetParameter("const.product.devicetype", "", buf, bufLen);
    return ret > 0 && strncmp(buf, "2in1", strlen("2in1")) == 0;
}
}

CcmUtil &CcmUtil::GetInstance()
{
    static CcmUtil util;
    return util;
}

int32_t CcmUtil::GetCliConcurrencyLimit()
{
    if (!maxCliQuantity_.isLoaded) {
        int32_t value = system::GetIntParameter<int32_t>(MAX_CLI_QUANTITY, DEFAULT_MAX_CLI_QUANTITY);
        if (value <= 0) {
            TAG_LOGW(AAFwkTag::CLI_TOOL,
                "Invalid cli concurrency limit: %{public}d, fallback to default", value);
            value = DEFAULT_MAX_CLI_QUANTITY;
        }
        maxCliQuantity_.value = value;
        maxCliQuantity_.isLoaded = true;
    }
    TAG_LOGD(AAFwkTag::CLI_TOOL, "MaxCLiQuantity: %{public}d", maxCliQuantity_.value);
    return maxCliQuantity_.value;
}

bool CcmUtil::IsPublicCmdEnabled()
{
    if (!publicCmdEnabled_.isLoaded) {
        bool def = IsPcOr2in1();
        publicCmdEnabled_.value = system::GetBoolParameter(CLI_PUBLIC_CMD_ENABLED, def);
        publicCmdEnabled_.isLoaded = true;
    }
    return publicCmdEnabled_.value;
}
} // namespace CliTool
} // namespace OHOS

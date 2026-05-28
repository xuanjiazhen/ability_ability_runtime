/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef MOCK_PERMISSION_VERIFICATION_H
#define MOCK_PERMISSION_VERIFICATION_H

#include <string>

namespace OHOS {
namespace AAFwk {
class PermissionVerification {
public:
    static PermissionVerification *GetInstance()
    {
        static PermissionVerification instance;
        return &instance;
    }

    bool VerifySupportCrossAppEmbedForOaPermission() const
    {
        return verifySupportCrossAppEmbedForOaPermission_;
    }

    void SetVerifySupportCrossAppEmbedForOaPermission(bool result)
    {
        verifySupportCrossAppEmbedForOaPermission_ = result;
    }

    bool IsSystemAppCall() const
    {
        return true;
    }

private:
    bool verifySupportCrossAppEmbedForOaPermission_ = false;
};
} // namespace AAFwk
} // namespace OHOS

#endif // MOCK_PERMISSION_VERIFICATION_H

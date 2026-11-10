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

#ifndef OHOS_ABILITY_RUNTIME_PERMISSION_DIALOG_H
#define OHOS_ABILITY_RUNTIME_PERMISSION_DIALOG_H

#include <memory>
#include <string>

#include "ability_context.h"

namespace OHOS {
namespace AbilityRuntime {
#ifdef SUPPORT_SCREEN

/**
 * Callback settled with the final permission-dialog decision. It is invoked
 * exactly once for every mounted dialog: granted when the user approves,
 * otherwise declined (also covers error and release outcomes).
 */
class PermissionDialogCallback {
public:
    PermissionDialogCallback() = default;
    virtual ~PermissionDialogCallback() = default;

    virtual void OnResult(bool granted) = 0;
};

/**
 * Pull up the invokePublicFunction permission dialog as a modal UIExtension
 * inside the caller's own UIContent. This is a pure client-side operation: the
 * dialog is rendered in the invoking application's process and window, on its
 * UI thread. No cross-process communication is involved. The consent Want is
 * assembled internally from the caller bundle (resolved from the context) and
 * the given target bundle and intent id.
 *
 * @param context The caller's AbilityContext, whose UIContent hosts the dialog.
 * @param targetBundleName The target capability bundle the caller asks to invoke.
 * @param intentId The server-side pre-registered execute record id.
 * @param callback The callback settled by the dialog's outcome.
 * @return Returns ERR_OK on success, otherwise an error code.
 */
ErrCode StartPermissionDialog(const std::shared_ptr<AbilityContext> &context,
    const std::string &targetBundleName, int32_t intentId,
    const std::shared_ptr<PermissionDialogCallback> &callback);
#endif // SUPPORT_SCREEN
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_PERMISSION_DIALOG_H
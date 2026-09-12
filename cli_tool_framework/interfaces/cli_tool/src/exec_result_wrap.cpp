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

#include "exec_result_wrap.h"
namespace OHOS::CliTool {
bool ExecResultWrap::Marshalling(Parcel &parcel) const { return execResult.Marshalling(parcel); }
ExecResultWrap *ExecResultWrap::Unmarshalling(Parcel &parcel) {
    auto wrap = new (std::nothrow) ExecResultWrap();
    if (wrap == nullptr) return nullptr;
    auto *result = ExecResult::Unmarshalling(parcel);
    if (result == nullptr) { delete wrap; return nullptr; }
    wrap->execResult = std::move(*result);
    delete result;
    return wrap;
}
}

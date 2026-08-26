/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

#include "ability_manager_service.h"

#define private public
#include "abilitystubsetmissioninfodone_fuzzer.h"
#undef private
#include "fuzz_util.h"

using namespace OHOS::AAFwk;

FUZZ_ABILITY_SERVICE_ENTRY_IMPL(AbilityManagerInterfaceCode::SET_MISSION_INFO)

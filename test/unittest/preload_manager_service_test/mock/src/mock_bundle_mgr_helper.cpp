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

#include "bundle_mgr_helper.h"
#include "mock_my_status.h"

namespace OHOS {
namespace AppExecFwk {

BundleMgrHelper::BundleMgrHelper()
{
}

BundleMgrHelper::~BundleMgrHelper()
{
}

std::shared_ptr<BundleMgrHelper> BundleMgrHelper::GetInstance()
{
    static std::shared_ptr<BundleMgrHelper> instance = std::make_shared<BundleMgrHelper>();
    return instance;
}

int32_t BundleMgrHelper::GetLaunchWantForBundle(const std::string& bundleName, AAFwk::Want& want, int32_t userId)
{
    return AAFwk::MyStatus::GetInstance().retGetLaunchWantForBundle_;
}

bool BundleMgrHelper::QueryAbilityInfo(const AAFwk::Want& want, int32_t flags, int32_t userId, AbilityInfo& abilityInfo)
{
    // AbilityInfo is a Parcelable (-> public virtual RefBase), i.e. a reference-counted object.
    // RefBase::operator=(const RefBase&) is NOT a value copy: it ignores the source and performs
    // refcount surgery on the DESTINATION (it always dereferences this->refs_). Value-assigning a
    // Parcelable out-parameter (`abilityInfo = queryAbilityInfo_`) therefore invokes
    // RefBase::operator= on the destination, which dereferences a garbage/uninitialised RefBase
    // virtual-base pointer and crashes (SIGSEGV SEGV_MAPERR). Populate the out-parameter
    // field-by-field with only the scalar fields PreloadManagerService actually consumes, which
    // never touches the RefBase base. See cppcrash-preload_manager_service_test-0-20260807170139112.log.
    const AbilityInfo &src = AAFwk::MyStatus::GetInstance().queryAbilityInfo_;
    abilityInfo.launchMode = src.launchMode;
    abilityInfo.applicationInfo.appPreloadPhase = src.applicationInfo.appPreloadPhase;
    // metadata is left at its default (empty); NativeAbilityMetaData::IsWithNative() thus returns
    // false, matching the existing expectations of PreloadApplication_008..011.
    return AAFwk::MyStatus::GetInstance().retQueryAbilityInfo_;
}
} // namespace AppExecFwk
} // namespace OHOS
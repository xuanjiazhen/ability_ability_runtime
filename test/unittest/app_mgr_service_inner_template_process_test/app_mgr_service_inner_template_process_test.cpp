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

#include <memory>

#include <gtest/gtest.h>

#define private public
#include "app_mgr_service_inner.h"
#undef private

using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
class AppMgrServiceInnerTemplateProcessTest : public testing::Test {};

/**
 * @tc.name: MarkTemplateProcess_NullAppRecord_001
 * @tc.desc: MarkTemplateProcess - appRecord is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTemplateProcessTest, MarkTemplateProcess_NullAppRecord_001, TestSize.Level2)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->MarkTemplateProcess(nullptr, "entry.hap");
    EXPECT_NE(appMgrServiceInner, nullptr);
}
} // namespace AppExecFwk
} // namespace OHOS

/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>

#include "native_lib_util.h"

using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
class NativeLibUtilTest : public testing::Test {
public:
    NativeLibUtilTest()
    {}
    ~NativeLibUtilTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void NativeLibUtilTest::SetUpTestCase(void)
{}

void NativeLibUtilTest::TearDownTestCase(void)
{}

void NativeLibUtilTest::SetUp(void)
{}

void NativeLibUtilTest::TearDown(void)
{}

/**
 * @tc.number: GetLibPath_0100
 * @tc.name: GetLibPath
 * @tc.desc: Test whether GetLibPath and are called normally.
 */
HWTEST_F(NativeLibUtilTest, GetLibPath_0100, TestSize.Level1)
{
    std::string hapPath = "/data/test/NativeLibUtilTest.hap";
    bool isPreInstallApp = true;
    std::string libpath = AppExecFwk::GetLibPath(hapPath, isPreInstallApp);
    EXPECT_NE(libpath, "");
}

/**
 * @tc.number: GetHapSoPath_0200
 * @tc.name: GetHapSoPath
 * @tc.desc: Test whether GetHapSoPath and are called normally.
 */
HWTEST_F(NativeLibUtilTest, GetHapSoPath_0200, TestSize.Level1)
{
    AppExecFwk::HapModuleInfo hapInfo;
    AppLibPathMap appLibPaths;
    bool isPreInstallApp = true;
    hapInfo.hapPath = "/data/test/NativeLibUtilTest.hap";
    hapInfo.nativeLibraryPath = "";
    AppExecFwk::GetHapSoPath(hapInfo, appLibPaths, isPreInstallApp);
    EXPECT_EQ(appLibPaths.empty(), true);

    hapInfo.nativeLibraryPath = "/data/test/nativeLibraryPath";
    hapInfo.compressNativeLibs = false;
    AppExecFwk::GetHapSoPath(hapInfo, appLibPaths, isPreInstallApp);
    EXPECT_EQ(appLibPaths.empty(), false);
}

/**
 * @tc.number: GetHspNativeLibPath_0300
 * @tc.name: GetHspNativeLibPath
 * @tc.desc: Test whether GetHspNativeLibPath and are called normally.
 */
HWTEST_F(NativeLibUtilTest, GetHspNativeLibPath_0300, TestSize.Level1)
{
    AppExecFwk::BaseSharedBundleInfo hspInfo;
    AppLibPathMap appLibPaths;
    bool isPreInstallApp = true;
    hspInfo.nativeLibraryPath = "";
    AppExecFwk::GetHspNativeLibPath(hspInfo, appLibPaths, isPreInstallApp);
    EXPECT_EQ(appLibPaths.empty(), true);
    
    hspInfo.nativeLibraryPath = "/data/test/nativeLibraryPath";
    hspInfo.bundleName = "nativeLibraryTest";
    hspInfo.moduleName = "library";

    hspInfo.compressNativeLibs = false;
    AppExecFwk::GetHspNativeLibPath(hspInfo, appLibPaths, isPreInstallApp);
    EXPECT_EQ(appLibPaths.empty(), false);

    appLibPaths.clear();
    isPreInstallApp = false;
    AppExecFwk::GetHspNativeLibPath(hspInfo, appLibPaths, isPreInstallApp);
    EXPECT_EQ(appLibPaths.empty(), false);
    
    appLibPaths.clear();
    hspInfo.compressNativeLibs = true;
    AppExecFwk::GetHspNativeLibPath(hspInfo, appLibPaths, isPreInstallApp);
    EXPECT_EQ(appLibPaths.empty(), false);
}

/**
 * @tc.number: GetPatchNativeLibPath_0400
 * @tc.name: GetPatchNativeLibPath
 * @tc.desc: Test whether GetPatchNativeLibPath and are called normally.
 */
HWTEST_F(NativeLibUtilTest, GetPatchNativeLibPath_0400, TestSize.Level1)
{
    AppExecFwk::HapModuleInfo hapInfo;
    std::string patchNativeLibraryPath = "";
    AppLibPathMap appLibPaths;
    
    hapInfo.hapPath = "/data/test/NativeLibUtilTest.hap";
    hapInfo.isLibIsolated = false;
    AppExecFwk::GetPatchNativeLibPath(hapInfo, patchNativeLibraryPath, appLibPaths);
    EXPECT_EQ(patchNativeLibraryPath, "");
     
    hapInfo.compressNativeLibs = true;
    patchNativeLibraryPath = "/data/test/patchNativeLibraryPath";
    AppExecFwk::GetPatchNativeLibPath(hapInfo, patchNativeLibraryPath, appLibPaths);
    EXPECT_EQ(appLibPaths.empty(), true);

    hapInfo.isLibIsolated = true;
    hapInfo.compressNativeLibs = false;
    hapInfo.bundleName = "nativeLibraryTest";
    hapInfo.moduleName = "library";
    hapInfo.hqfInfo.nativeLibraryPath = "/data/test/nativeLibraryPath";
    AppExecFwk::GetPatchNativeLibPath(hapInfo, patchNativeLibraryPath, appLibPaths);
    EXPECT_EQ(appLibPaths.empty(), false);
}

/**
 * @tc.number: GetHapSoPath_0500
 * @tc.name: GetHapSoPath with subdirectories
 * @tc.desc: Test GetHapSoPath with nativeLibrarySubDirs.
 */
HWTEST_F(NativeLibUtilTest, GetHapSoPath_0500, TestSize.Level1)
{
    AppExecFwk::HapModuleInfo hapInfo;
    AppLibPathMap appLibPaths;
    bool isPreInstallApp = true;
    
    hapInfo.bundleName = "com.example.app";
    hapInfo.moduleName = "entry";
    hapInfo.hapPath = "/data/test/NativeLibUtilTest.hap";
    hapInfo.nativeLibraryPath = "libs";
    hapInfo.nativeLibrarySubDirs = {"ark64-v8a", "arm64-v8a"};
    hapInfo.compressNativeLibs = true;
    
    AppExecFwk::GetHapSoPath(hapInfo, appLibPaths, isPreInstallApp);
    EXPECT_EQ(appLibPaths.empty(), false);
    EXPECT_EQ(appLibPaths["com.example.app/entry"].size(), size_t(2));
    EXPECT_EQ(appLibPaths["com.example.app/entry"][0], "/data/storage/el1/bundle/libs/ark64-v8a");
    EXPECT_EQ(appLibPaths["com.example.app/entry"][1], "/data/storage/el1/bundle/libs/arm64-v8a");
}

/**
 * @tc.number: GetHapSoPath_0600
 * @tc.name: GetHapSoPath with empty subdirectories
 * @tc.desc: Test GetHapSoPath with empty nativeLibrarySubDirs (backward compatibility).
 */
HWTEST_F(NativeLibUtilTest, GetHapSoPath_0600, TestSize.Level1)
{
    AppExecFwk::HapModuleInfo hapInfo;
    AppLibPathMap appLibPaths;
    bool isPreInstallApp = true;
    
    hapInfo.bundleName = "com.example.app";
    hapInfo.moduleName = "entry";
    hapInfo.hapPath = "/data/test/NativeLibUtilTest.hap";
    hapInfo.nativeLibraryPath = "libs";
    hapInfo.nativeLibrarySubDirs = {};
    hapInfo.compressNativeLibs = true;
    
    AppExecFwk::GetHapSoPath(hapInfo, appLibPaths, isPreInstallApp);
    EXPECT_EQ(appLibPaths.empty(), false);
    EXPECT_EQ(appLibPaths["com.example.app/entry"].size(), size_t(1));
    EXPECT_EQ(appLibPaths["com.example.app/entry"][0], "/data/storage/el1/bundle/libs");
} // namespace AbilityRuntime
} // namespace OHOS

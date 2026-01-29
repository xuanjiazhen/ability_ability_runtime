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

#include <gtest/gtest.h>
#include <map>
#include <string>
#include <vector>

#include "ets_native_lib_util.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
class EtsNativeLibUtilTest : public testing::Test {
};

namespace {
constexpr const char* LOCAL_CODE_PATH = "/data/storage/el1/bundle";
constexpr const char* ABS_CODE_PATH = "/data/app/el1/bundle/public";
}

/**
 * @tc.number: GetEtsLibPath_0100
 * @tc.name: GetEtsLibPath
 * @tc.desc: Verify non-preinstall uses local code path.
 */
HWTEST_F(EtsNativeLibUtilTest, GetEtsLibPath_0100, Function | MediumTest | Level1)
{
    const std::string hapPath = "/data/app/el1/bundle/public/com.example.app/entry.hap";
    std::string libPath = GetEtsLibPath(hapPath, false);
    EXPECT_EQ(libPath, LOCAL_CODE_PATH);
}

/**
 * @tc.number: GetEtsLibPath_0200
 * @tc.name: GetEtsLibPath
 * @tc.desc: Verify preinstall uses hap directory.
 */
HWTEST_F(EtsNativeLibUtilTest, GetEtsLibPath_0200, Function | MediumTest | Level1)
{
    const std::string hapPath = "/system/app/com.example.app/entry.hap";
    std::string libPath = GetEtsLibPath(hapPath, true);
    EXPECT_EQ(libPath, "/system/app/com.example.app");
}

/**
 * @tc.number: GetEtsHapSoPath_0100
 * @tc.name: GetEtsHapSoPath
 * @tc.desc: Verify empty nativeLibraryPath returns nothing.
 */
HWTEST_F(EtsNativeLibUtilTest, GetEtsHapSoPath_0100, Function | MediumTest | Level1)
{
    HapModuleInfo hapInfo;
    hapInfo.bundleName = "com.example.empty";
    hapInfo.moduleName = "entry";

    AppLibPathMap appLibPaths;
    std::map<std::string, std::string> abcPaths;
    GetEtsHapSoPath(hapInfo, appLibPaths, false, abcPaths);
    EXPECT_TRUE(appLibPaths.empty());
    EXPECT_TRUE(abcPaths.empty());
}

/**
 * @tc.number: GetEtsHapSoPath_0200
 * @tc.name: GetEtsHapSoPath
 * @tc.desc: Verify uncompressed preinstall hap path.
 */
HWTEST_F(EtsNativeLibUtilTest, GetEtsHapSoPath_0200, Function | MediumTest | Level1)
{
    HapModuleInfo hapInfo;
    hapInfo.bundleName = "com.example.lib";
    hapInfo.moduleName = "entry";
    hapInfo.hapPath = "/system/app/com.example.lib/entry.hap";
    hapInfo.nativeLibraryPath = "entry.hap!/libs/arm";
    hapInfo.compressNativeLibs = false;

    AppLibPathMap appLibPaths;
    std::map<std::string, std::string> abcPaths;
    GetEtsHapSoPath(hapInfo, appLibPaths, true, abcPaths);

    const std::string appLibPathKey = "com.example.lib/entry";
    ASSERT_EQ(appLibPaths[appLibPathKey].size(), 1u);
    EXPECT_EQ(appLibPaths[appLibPathKey][0], "/system/app/com.example.lib/entry.hap!/libs/arm");

    const std::string abcKey = "/data/storage/el1/bundle/entry/ets/modules_static.abc";
    EXPECT_EQ(abcPaths[abcKey], appLibPathKey);
}

/**
 * @tc.number: GetEtsHapSoPath_0300
 * @tc.name: GetEtsHapSoPath
 * @tc.desc: Verify compressed hap uses local code path.
 */
HWTEST_F(EtsNativeLibUtilTest, GetEtsHapSoPath_0300, Function | MediumTest | Level1)
{
    HapModuleInfo hapInfo;
    hapInfo.bundleName = "com.example.compress";
    hapInfo.moduleName = "feature";
    hapInfo.nativeLibraryPath = "libs/arm64";
    hapInfo.compressNativeLibs = true;

    AppLibPathMap appLibPaths;
    std::map<std::string, std::string> abcPaths;
    GetEtsHapSoPath(hapInfo, appLibPaths, false, abcPaths);

    const std::string appLibPathKey = "com.example.compress/feature";
    std::string base = LOCAL_CODE_PATH;
    ASSERT_EQ(appLibPaths[appLibPathKey].size(), 1u);
    EXPECT_EQ(appLibPaths[appLibPathKey][0], base + "/libs/arm64");
}

/**
 * @tc.number: GetEtsHspNativeLibPath_0100
 * @tc.name: GetEtsHspNativeLibPath
 * @tc.desc: Verify compressed internal HSP path.
 */
HWTEST_F(EtsNativeLibUtilTest, GetEtsHspNativeLibPath_0100, Function | MediumTest | Level1)
{
    BaseSharedBundleInfo hspInfo;
    hspInfo.bundleName = "com.example.host";
    hspInfo.moduleName = "shared";
    hspInfo.nativeLibraryPath = "shared/libs/arm64";
    hspInfo.compressNativeLibs = true;

    AppLibPathMap appLibPaths;
    std::map<std::string, std::string> abcPaths;
    GetEtsHspNativeLibPath(hspInfo, appLibPaths, false, hspInfo.bundleName, abcPaths);

    const std::string appLibPathKey = "com.example.host/shared";
    std::string base = LOCAL_CODE_PATH;
    ASSERT_EQ(appLibPaths[appLibPathKey].size(), 1u);
    EXPECT_EQ(appLibPaths[appLibPathKey][0], base + "/com.example.host/shared/libs/arm64");

    const std::string abcKey = "/data/storage/el1/bundle/shared/ets/modules_static.abc";
    EXPECT_EQ(abcPaths[abcKey], appLibPathKey);
}

/**
 * @tc.number: GetEtsHspNativeLibPath_0200
 * @tc.name: GetEtsHspNativeLibPath
 * @tc.desc: Verify uncompressed external HSP path.
 */
HWTEST_F(EtsNativeLibUtilTest, GetEtsHspNativeLibPath_0200, Function | MediumTest | Level1)
{
    BaseSharedBundleInfo hspInfo;
    hspInfo.bundleName = "com.shared.lib";
    hspInfo.moduleName = "module";
    hspInfo.hapPath = std::string(ABS_CODE_PATH) + "/com.shared.lib/module.hsp";
    hspInfo.nativeLibraryPath = "module.hsp!/libs/arm";
    hspInfo.compressNativeLibs = false;

    AppLibPathMap appLibPaths;
    std::map<std::string, std::string> abcPaths;
    GetEtsHspNativeLibPath(hspInfo, appLibPaths, false, "com.example.app", abcPaths);

    const std::string appLibPathKey = "com.shared.lib/module";
    std::string base = LOCAL_CODE_PATH;
    ASSERT_EQ(appLibPaths[appLibPathKey].size(), 1u);
    EXPECT_EQ(appLibPaths[appLibPathKey][0], base + "/com.shared.lib/module/module.hsp!/libs/arm");

    const std::string abcKey = "/data/storage/el1/bundle/com.shared.lib/module/ets/modules_static.abc";
    EXPECT_EQ(abcPaths[abcKey], appLibPathKey);
}

/**
 * @tc.number: GetEtsHspNativeLibPath_0300
 * @tc.name: GetEtsHspNativeLibPath
 * @tc.desc: Verify uncompressed preinstall HSP path.
 */
HWTEST_F(EtsNativeLibUtilTest, GetEtsHspNativeLibPath_0300, Function | MediumTest | Level1)
{
    BaseSharedBundleInfo hspInfo;
    hspInfo.bundleName = "com.preinstall.lib";
    hspInfo.moduleName = "shared";
    hspInfo.hapPath = "/system/app/com.preinstall.lib/shared.hsp";
    hspInfo.nativeLibraryPath = "shared/lib/arm";
    hspInfo.compressNativeLibs = false;

    AppLibPathMap appLibPaths;
    std::map<std::string, std::string> abcPaths;
    GetEtsHspNativeLibPath(hspInfo, appLibPaths, true, hspInfo.bundleName, abcPaths);

    const std::string appLibPathKey = "com.preinstall.lib/shared";
    ASSERT_EQ(appLibPaths[appLibPathKey].size(), 1u);
    EXPECT_EQ(appLibPaths[appLibPathKey][0], "/system/app/com.preinstall.lib/shared/lib/arm");

    const std::string abcKey = "/data/storage/el1/bundle/shared/ets/modules_static.abc";
    EXPECT_EQ(abcPaths[abcKey], appLibPathKey);
}

/**
 * @tc.number: GetEtsHspNativeLibPath_0400
 * @tc.name: GetEtsHspNativeLibPath
 * @tc.desc: Verify empty HSP nativeLibraryPath returns nothing.
 */
HWTEST_F(EtsNativeLibUtilTest, GetEtsHspNativeLibPath_0400, Function | MediumTest | Level1)
{
    BaseSharedBundleInfo hspInfo;
    AppLibPathMap appLibPaths;
    std::map<std::string, std::string> abcPaths;
    GetEtsHspNativeLibPath(hspInfo, appLibPaths, false, "", abcPaths);
    EXPECT_TRUE(appLibPaths.empty());
    EXPECT_TRUE(abcPaths.empty());
}

/**
 * @tc.number: GetEtsPatchNativeLibPath_0100
 * @tc.name: GetEtsPatchNativeLibPath
 * @tc.desc: Verify no patch path when empty.
 */
HWTEST_F(EtsNativeLibUtilTest, GetEtsPatchNativeLibPath_0100, Function | MediumTest | Level1)
{
    HapModuleInfo hapInfo;
    hapInfo.bundleName = "com.example.patch";
    hapInfo.moduleName = "entry";
    hapInfo.isLibIsolated = false;
    hapInfo.compressNativeLibs = true;

    std::string patchNativeLibraryPath;
    AppLibPathMap appLibPaths;
    std::map<std::string, std::string> abcPaths;
    GetEtsPatchNativeLibPath(hapInfo, patchNativeLibraryPath, appLibPaths, abcPaths);

    EXPECT_TRUE(appLibPaths.empty());
    EXPECT_TRUE(abcPaths.empty());
    EXPECT_TRUE(patchNativeLibraryPath.empty());
}

/**
 * @tc.number: GetEtsPatchNativeLibPath_0200
 * @tc.name: GetEtsPatchNativeLibPath
 * @tc.desc: Verify compressed and not isolated skips patch.
 */
HWTEST_F(EtsNativeLibUtilTest, GetEtsPatchNativeLibPath_0200, Function | MediumTest | Level1)
{
    HapModuleInfo hapInfo;
    hapInfo.bundleName = "com.example.patch";
    hapInfo.moduleName = "entry";
    hapInfo.isLibIsolated = false;
    hapInfo.compressNativeLibs = true;

    std::string patchNativeLibraryPath = "patch/libs/arm";
    AppLibPathMap appLibPaths;
    std::map<std::string, std::string> abcPaths;
    GetEtsPatchNativeLibPath(hapInfo, patchNativeLibraryPath, appLibPaths, abcPaths);

    EXPECT_TRUE(appLibPaths.empty());
    EXPECT_TRUE(abcPaths.empty());
    EXPECT_EQ(patchNativeLibraryPath, "patch/libs/arm");
}

/**
 * @tc.number: GetEtsPatchNativeLibPath_0300
 * @tc.name: GetEtsPatchNativeLibPath
 * @tc.desc: Verify isolated patch path added.
 */
HWTEST_F(EtsNativeLibUtilTest, GetEtsPatchNativeLibPath_0300, Function | MediumTest | Level1)
{
    HapModuleInfo hapInfo;
    hapInfo.bundleName = "com.example.patch";
    hapInfo.moduleName = "entry";
    hapInfo.isLibIsolated = true;
    hapInfo.compressNativeLibs = true;
    hapInfo.hqfInfo.nativeLibraryPath = "patch/entry/libs/arm";

    std::string patchNativeLibraryPath;
    AppLibPathMap appLibPaths;
    std::map<std::string, std::string> abcPaths;
    GetEtsPatchNativeLibPath(hapInfo, patchNativeLibraryPath, appLibPaths, abcPaths);

    const std::string appLibPathKey = "com.example.patch/entry";
    std::string base = LOCAL_CODE_PATH;
    ASSERT_EQ(appLibPaths[appLibPathKey].size(), 1u);
    EXPECT_EQ(appLibPaths[appLibPathKey][0], base + "/patch/entry/libs/arm");

    const std::string abcKey = "/data/storage/el1/bundle/entry/ets/modules_static.abc";
    EXPECT_EQ(abcPaths[abcKey], appLibPathKey);
}

/**
 * @tc.number: GetEtsNativeLibPath_0100
 * @tc.name: GetEtsNativeLibPath
 * @tc.desc: Verify aggregated app/hap/hsp paths.
 */
HWTEST_F(EtsNativeLibUtilTest, GetEtsNativeLibPath_0100, Function | MediumTest | Level1)
{
    BundleInfo bundleInfo;
    bundleInfo.applicationInfo.bundleName = "com.example.app";
    bundleInfo.applicationInfo.appQuickFix.deployedAppqfInfo.nativeLibraryPath = "patch/global/libs";
    bundleInfo.applicationInfo.nativeLibraryPath = "app/libs";

    HapModuleInfo hapInfo;
    hapInfo.bundleName = bundleInfo.applicationInfo.bundleName;
    hapInfo.moduleName = "entry";
    hapInfo.nativeLibraryPath = "entry.hap!/libs/arm";
    hapInfo.hapPath = std::string(ABS_CODE_PATH) + "/com.example.app/entry.hap";
    hapInfo.isLibIsolated = false;
    hapInfo.compressNativeLibs = false;
    bundleInfo.hapModuleInfos.emplace_back(hapInfo);

    BaseSharedBundleInfo hspInfo;
    hspInfo.bundleName = bundleInfo.applicationInfo.bundleName;
    hspInfo.moduleName = "shared";
    hspInfo.nativeLibraryPath = "shared/libs";
    hspInfo.compressNativeLibs = true;
    std::vector<BaseSharedBundleInfo> hspList;
    hspList.emplace_back(hspInfo);

    AppLibPathMap appLibPaths;
    std::map<std::string, std::string> abcPaths;
    GetEtsNativeLibPath(bundleInfo, hspList, appLibPaths, abcPaths);

    ASSERT_EQ(appLibPaths["default"].size(), 2u);
    std::string base = LOCAL_CODE_PATH;
    EXPECT_EQ(appLibPaths["default"][0], base + "/patch/global/libs");
    EXPECT_EQ(appLibPaths["default"][1], base + "/app/libs");

    const std::string hapKey = "com.example.app/entry";
    ASSERT_EQ(appLibPaths[hapKey].size(), 2u);
    EXPECT_EQ(appLibPaths[hapKey][0], base + "/patch/global/libs");
    EXPECT_EQ(appLibPaths[hapKey][1], base + "/entry.hap!/libs/arm");

    const std::string hspKey = "com.example.app/shared";
    ASSERT_EQ(appLibPaths[hspKey].size(), 1u);
    EXPECT_EQ(appLibPaths[hspKey][0], base + "/com.example.app/shared/libs");

    EXPECT_EQ(abcPaths["default"], "default");
}

/**
 * @tc.number: GetEtsNativeLibPath_0200
 * @tc.name: GetEtsNativeLibPath
 * @tc.desc: Verify nativeLibraryPath empty behavior.
 */
HWTEST_F(EtsNativeLibUtilTest, GetEtsNativeLibPath_0200, Function | MediumTest | Level1)
{
    BundleInfo bundleInfo;
    bundleInfo.applicationInfo.bundleName = "com.example.empty";

    HapModuleInfo hapInfo;
    hapInfo.bundleName = bundleInfo.applicationInfo.bundleName;
    hapInfo.moduleName = "entry";
    hapInfo.nativeLibraryPath = "entry/libs";
    hapInfo.hapPath = std::string(ABS_CODE_PATH) + "/com.example.empty/entry.hap";
    hapInfo.compressNativeLibs = true;
    hapInfo.isLibIsolated = false;
    bundleInfo.hapModuleInfos.emplace_back(hapInfo);

    std::vector<BaseSharedBundleInfo> hspList;
    AppLibPathMap appLibPaths;
    std::map<std::string, std::string> abcPaths;
    GetEtsNativeLibPath(bundleInfo, hspList, appLibPaths, abcPaths);

    const std::string hapKey = "com.example.empty/entry";
    std::string base = LOCAL_CODE_PATH;
    ASSERT_EQ(appLibPaths[hapKey].size(), 1u);
    EXPECT_EQ(appLibPaths[hapKey][0], base + "/entry/libs");

    EXPECT_EQ(appLibPaths.count("default"), 0u);
    EXPECT_EQ(abcPaths["default"], "default");
}
} // namespace AppExecFwk
} // namespace OHOS

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

#include "system_recovery_settings_data.h"

#include <memory>
#include <string>
#include <vector>

#include "datashare_errno.h"
#include "datashare_helper.h"
#include "datashare_predicates.h"
#include "datashare_result_set.h"
#include "hilog_tag_wrapper.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "uri.h"

namespace OHOS {
namespace AppExecFwk {
namespace {

// 与 Settings 应用读取飞行模式时使用的全局 SettingsData 表保持一致。
constexpr char SETTINGS_DATA_PROXY_URI[] =
    "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true";
constexpr char SETTINGS_DATA_EXT_URI[] = "datashare:///com.ohos.settingsdata.DataAbility";
constexpr char AIRPLANE_MODE_URI[] =
    "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true&key="
    "settings.telephony.airplanemode";
constexpr char SETTINGS_DATA_KEY_COLUMN[] = "KEYWORD";
constexpr char SETTINGS_DATA_VALUE_COLUMN[] = "VALUE";
constexpr char AIRPLANE_MODE_KEY[] = "settings.telephony.airplanemode";

std::shared_ptr<DataShare::DataShareHelper> CreateSettingsDataHelper()
{
    auto saManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (saManager == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "get system ability manager failed when reading airplane mode");
        return nullptr;
    }

    // 恢复框架运行在 AppMgr 进程，以 AppMgr SA token 调用已有 SettingsData IPC。
    auto callerToken = saManager->GetSystemAbility(APP_MGR_SERVICE_ID);
    if (callerToken == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "get AppMgr token failed when reading airplane mode");
        return nullptr;
    }
    return DataShare::DataShareHelper::Creator(callerToken, SETTINGS_DATA_PROXY_URI, SETTINGS_DATA_EXT_URI);
}

} // namespace

bool SystemRecoverySettingsData::GetAirplaneMode(bool &enabled)
{
    enabled = false;
    auto helper = CreateSettingsDataHelper();
    if (helper == nullptr) {
        return false;
    }

    DataShare::DataSharePredicates predicates;
    predicates.EqualTo(SETTINGS_DATA_KEY_COLUMN, AIRPLANE_MODE_KEY);
    std::vector<std::string> columns = { SETTINGS_DATA_VALUE_COLUMN };
    // DataShare Query 接收 Uri 左值引用，因此需要保留 Uri 对象直至查询返回。
    Uri uri(AIRPLANE_MODE_URI);
    auto result = helper->Query(uri, predicates, columns);
    if (result == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "query airplane mode from SettingsData failed");
        helper->Release();
        return false;
    }

    // Settings 应用也以关闭作为缺省值；首次启动尚未写入该键时保持同样语义。
    if (result->GoToFirstRow() != DataShare::E_OK) {
        result->Close();
        helper->Release();
        return true;
    }

    int columnIndex = 0;
    std::string value;
    const int columnRet = result->GetColumnIndex(SETTINGS_DATA_VALUE_COLUMN, columnIndex);
    const int valueRet = columnRet == DataShare::E_OK ? result->GetString(columnIndex, value) : columnRet;
    result->Close();
    helper->Release();
    if (valueRet != DataShare::E_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "read airplane mode value from SettingsData failed");
        return false;
    }

    enabled = value == "1" || value == "true";
    return true;
}

} // namespace AppExecFwk
} // namespace OHOS

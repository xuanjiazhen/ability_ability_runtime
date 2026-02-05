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

#ifndef WANT_SERIALIZATION_UTIL_H
#define WANT_SERIALIZATION_UTIL_H

#include <string>
#include "ipc/parcel.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {

/**
 * @class WantSerializationUtil
 * @brief Utility class for Want serialization with adaptive String/String16 encoding based on IsDMS flag.
 *
 * This class provides methods to determine if a Want is in DMS (Distributed Management Service) mode
 * and to read/write strings using the appropriate encoding format based on the IsDMS flag.
 */
class WantSerializationUtil {
public:
    /** Key name for the IsDMS flag in WantParams */
    static constexpr const char* WANT_PARAM_IS_DMS_KEY = "ohos.extra.param.key.isDMS";

    /**
     * @brief Determine if the Want is in DMS mode based on deviceId.
     * @param want The Want object to check.
     * @return true if deviceId is not empty (DMS mode), false otherwise.
     */
    static bool IsDMSMode(const Want& want);

    /**
     * @brief Set the IsDMS flag in the Want's WantParams.
     * @param want The Want object to modify (non-const for setting params).
     */
    static void SetIsDMSFlag(Want& want);

    /**
     * @brief Get the IsDMS flag from the Want's WantParams.
     * @param want The Want object to read from.
     * @return true if IsDMS flag is set to 1, false otherwise (defaults to false).
     */
    static bool GetIsDMSFlag(const Want& want);

    /**
     * @brief Write a string to parcel using String or String16 based on IsDMS mode.
     * @param parcel The parcel to write to.
     * @param str The string to write.
     * @param isDMS If true, use String16 encoding; if false, use String encoding.
     * @return true if write succeeded, false otherwise.
     */
    static bool WriteStringByMode(Parcel& parcel, const std::string& str, bool isDMS);

    /**
     * @brief Read a string from parcel using String or String16 based on IsDMS mode.
     * @param parcel The parcel to read from.
     * @param str Output parameter for the read string.
     * @param isDMS If true, use String16 encoding; if false, use String encoding.
     * @return true if read succeeded, false otherwise.
     */
    static bool ReadStringByMode(Parcel& parcel, std::string& str, bool isDMS);
};

} // namespace AAFwk
} // namespace OHOS

#endif // WANT_SERIALIZATION_UTIL_H

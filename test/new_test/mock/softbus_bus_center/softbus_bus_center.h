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

#ifndef MOCK_SOFTBUS_BUS_CENTER_H
#define MOCK_SOFTBUS_BUS_CENTER_H

typedef struct {
    char networkId[128];    /**< Device network id */
    char deviceName[128];  /**< Device name */
    uint16_t deviceTypeId;                 /**< Device type id */
    int32_t osType;                        /**< Os type */
    char osVersion[128];    /**< Os version */
} NodeBasicInfo;

int32_t GetLocalNodeDeviceInfo(const char *pkgName, NodeBasicInfo *info)
{
    return 0;
}
#endif

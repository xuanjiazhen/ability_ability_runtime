/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 */

#ifndef MOCK_HILOG_TAG_WRAPPER_H
#define MOCK_HILOG_TAG_WRAPPER_H

#define TAG_LOGD(tag, fmt, ...)
#define TAG_LOGI(tag, fmt, ...)
#define TAG_LOGW(tag, fmt, ...)
#define TAG_LOGE(tag, fmt, ...)

namespace OHOS {
namespace AAFwk {
struct AAFwkLogTag {
    static constexpr int EXT = 0;
    static constexpr int ABILITYMGR = 1;
};
} // namespace AAFwk
} // namespace OHOS

#endif // MOCK_HILOG_TAG_WRAPPER_H

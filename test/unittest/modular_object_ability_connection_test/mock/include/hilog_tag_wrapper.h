/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 */

#ifndef MOCK_HILOG_TAG_WRAPPER_H
#define MOCK_HILOG_TAG_WRAPPER_H

struct AAFwkLogTag {
    static constexpr const char *EXT = "EXT";
    static constexpr const char *APPKIT = "APPKIT";
};

#define TAG_LOGI(tag, fmt, ...)
#define TAG_LOGW(tag, fmt, ...)
#define TAG_LOGE(tag, fmt, ...)

#endif // MOCK_HILOG_TAG_WRAPPER_H

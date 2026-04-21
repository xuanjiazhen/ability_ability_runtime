#ifndef MOCK_HILOG_TAG_WRAPPER_H
#define MOCK_HILOG_TAG_WRAPPER_H

#include <cstdio>

// Minimal mock for hilog_tag_wrapper.h
// Just provide the TAG_LOG macros as no-ops

#define TAG_LOGD(tag, fmt, ...)
#define TAG_LOGI(tag, fmt, ...)
#define TAG_LOGW(tag, fmt, ...)
#define TAG_LOGE(tag, fmt, ...)

namespace OHOS {
namespace AAFwk {
struct AAFwkLogTag {
    static constexpr int EXT = 0;
    static constexpr int ABILITYMGR = 1;
    static constexpr int ABILITY = 2;
    static constexpr int APPKIT = 3;
};
} // namespace AAFwk
} // namespace OHOS

#endif // MOCK_HILOG_TAG_WRAPPER_H

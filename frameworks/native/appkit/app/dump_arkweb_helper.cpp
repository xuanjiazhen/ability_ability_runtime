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

#include "dump_arkweb_helper.h"

#include <cstdint>
#include <cstring>
#include "dfx_socket_request.h"
#include <unistd.h>
#include <errno.h>
#include <string>
#include <iostream>

#if defined(NWEB)
#include "nweb_helper.h"

#include "hilog_tag_wrapper.h"
#endif

namespace OHOS {
namespace AppExecFwk {
constexpr const uint32_t MAX_BUF_SIZE = (1U << 19);
int DumpArkWebHelper::DumpArkWeb(const std::string &customArgs, std::string &result)
{
    result.resize(MAX_BUF_SIZE);
#if defined(NWEB)
    OHOS::NWeb::NWebHelper &nWebHelper = OHOS::NWeb::NWebHelper::Instance();
    result = nWebHelper.DumpArkWebInfo(customArgs);
#endif
    auto size = result.size();
    if (size > 0) {
        result.resize(size);
        return 0;
    }
    result.resize(0);
    return -1;
}

extern "C" int32_t RequestFileDescriptor(int32_t type);
void DumpArkWebHelper::DumpArkWebJSHeap(const std::string &customArgs) {
#if defined(NWEB)
  TAG_LOGI(AAFwkTag::APPKIT, "DumpArkWebHelper::DumpArkWebJSHeap");
  OHOS::NWeb::NWebHelper &nWebHelper = OHOS::NWeb::NWebHelper::Instance();
  // params
  // todo: get args from info like dump js
  int32_t renderPid = -1;
  bool needSnapshot = true;
  bool needGC = false;
  bool needRaw = false;
  size_t pos = customArgs.find('=');
  // --jsmem=pid
  if (pos == std::string::npos || customArgs.substr(0, pos) != "--jsmem") {
    TAG_LOGE(AAFwkTag::APPKIT, "use --jsmem=pid");
    return;
  }
  renderPid = std::atoi(customArgs.substr(pos + 1).c_str());
  if (renderPid < 0) {
    TAG_LOGE(AAFwkTag::APPKIT, "valid pid");
    return;
  }
  int32_t fd = RequestFileDescriptor(
      static_cast<int32_t>(FaultLoggerType::JS_HEAP_SNAPSHOT));
  if (fd < 0) {
    TAG_LOGE(AAFwkTag::APPKIT, "RequestFileDescriptor failed");
    return;
  }
  nWebHelper.DumpArkWebJSHeap(fd, renderPid, needSnapshot, needGC, needRaw);
#endif
}

}  // namespace AppExecFwk
}  // namespace OHOS

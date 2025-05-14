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

let ExtensionContext = requireNapi('application.ExtensionContext');
let Caller = requireNapi('application.Caller');
let hilog = requireNapi('hilog');

let domainID = 0xD001320;
let TAG = 'JSENV';

const ERROR_CODE_INVALID_PARAM = 401;
const ERROR_MSG_INVALID_PARAM = 'Invalid input parameter.';
class ParamError extends Error {
  constructor() {
    super(ERROR_MSG_INVALID_PARAM);
    this.code = ERROR_CODE_INVALID_PARAM;
  }
}

class AppServiceExtensionContext extends ExtensionContext {
  constructor(obj) {
    super(obj);
  }

  connectServiceExtensionAbility(want, options) {
    hilog.sLogI(domainID, TAG, 'connectServiceExtensionAbility');
    return this.__context_impl__.connectServiceExtensionAbility(want, options);
  }

  disconnectServiceExtensionAbility(connection, callback) {
    hilog.sLogI(domainID, TAG, 'disconnectServiceExtensionAbility');
    return this.__context_impl__.disconnectServiceExtensionAbility(connection, callback);
  }

  terminateSelf(callback) {
    hilog.sLogI(domainID, TAG, 'terminateSelf');
    return this.__context_impl__.terminateSelf(callback);
  }

}

export default AppServiceExtensionContext;

/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#ifndef NAPI_INIT_TEST_H
#define NAPI_INIT_TEST_H

EXTERN_C_START

#define TEST_STR_LEN 40

extern const char TEST_STR[TEST_STR_LEN];

extern napi_property_descriptor PROPERTY_LIST[];
extern const size_t PROPERTY_LIST_COUNT;

extern napi_property_descriptor BUFFER_PROPERTY_LIST[];
extern const size_t BUFFER_PROPERTY_COUNT;

napi_value Init(napi_env env, napi_value exports);

EXTERN_C_END

#endif //NAPI_INIT_TEST_H
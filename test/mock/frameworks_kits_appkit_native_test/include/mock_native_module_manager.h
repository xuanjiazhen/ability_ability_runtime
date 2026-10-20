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

#ifndef MOCK_NATIVE_MODULE_MANAGER_H
#define MOCK_NATIVE_MODULE_MANAGER_H

#include <string>
#include <vector>

#define private public
#include "native_module_manager.h"
#undef private

void MockSetNativeModuleManager(NativeModuleManager* instance);
void MockSetNativeModule(NativeModule* module);

// RR-30096301: inject loaded system module names / preload failure names for collector and preload unit tests
void MockSetLoadedSystemNativeModules(const std::vector<std::string>& names);
void MockSetPreloadFailNames(const std::vector<std::string>& names);
std::vector<std::string> MockGetPreloadCalls();
// Record the thread id that invoked PreloadSystemNativeModule (for non-main-thread assertions)
int MockGetPreloadCallTid();
void MockResetNativeModuleManagerState();

#endif // MOCK_NATIVE_MODULE_MANAGER_H

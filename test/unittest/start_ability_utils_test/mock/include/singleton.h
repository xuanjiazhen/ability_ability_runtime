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

#ifndef MOCK_SINGLETON_H
#define MOCK_SINGLETON_H

#include <memory>
#include <mutex>

namespace OHOS {
namespace DelayedSingletonRef {
// Mock版本的DelayedSingleton，避免测试环境中shared_ptr析构崩溃
template<typename T>
class DelayedSingleton {
public:
    static T* GetInstance()
    {
        static T instance;
        return &instance;
    }

    T* operator->() const
    {
        return GetInstance();
    }

    T& operator*() const
    {
        return *GetInstance();
    }
};

#define DECLARE_DELAYED_SINGLETON(Class)
private:
    Class() = default;
    ~Class() = default;
    friend class DelayedSingletonRef::DelayedSingleton<Class>;

}  // namespace DelayedSingletonRef

using DelayedSingletonRef::DelayedSingleton;

}  // namespace OHOS

#endif  // MOCK_SINGLETON_H

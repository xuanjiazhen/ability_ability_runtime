/*
 * Copyright (c) 2026 OpenHarmony. All rights reserved.
 * 测试代码 - 演示PR创建流程
 */

#ifndef ABILITY_RUNTIME_TEST_HELPER_H
#define ABILITY_RUNTIME_TEST_HELPER_H

#include <iostream>
#include <string>
#include <memory>

namespace OHOS {
namespace AbilityRuntime {
namespace Test {

class TestHelper {
public:
    TestHelper() : count_(0) {
        std::cout << "TestHelper created" << std::endl;
    }

    ~TestHelper() {
        std::cout << "TestHelper destroyed" << std::endl;
    }

    int GetCount() const {
        return count_;
    }

    void Increment() {
        ++count_;
    }

private:
    int count_;
};

void RunTest(const std::string& name) {
    std::cout << "Running test: " << name << std::endl;
}

} // namespace Test
} // namespace AbilityRuntime
} // namespace OHOS

#endif // ABILITY_RUNTIME_TEST_HELPER_H

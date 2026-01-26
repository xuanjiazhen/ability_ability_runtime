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

#include <iostream>
#include <memory>
#include <string>
#include <vector>

/**
 * @brief Test demo class for GitCode automation skill
 *
 * This class demonstrates the gitcode-automation skill's ability to:
 * 1. Detect C++ code changes
 * 2. Identify pointer usage (shared_ptr, unique_ptr)
 * 3. Recognize STL container usage (vector, string)
 * 4. Mark relevant checklist items as "需要检查"
 */
class TestSkillDemo {
public:
    TestSkillDemo() : count_(0) {
        std::cout << "TestSkillDemo initialized" << std::endl;
    }

    ~TestSkillDemo() {
        std::cout << "TestSkillDemo destroyed" << std::endl;
    }

    /**
     * @brief Add a new item to the list
     * @param item The item to add
     *
     * Note: Uses shared_ptr for memory management - should trigger
     * pointer-related checklist items
     */
    void AddItem(const std::string& item) {
        auto data = std::make_shared<std::string>(item);
        items_.push_back(data);
        count_++;
    }

    /**
     * @brief Get the current count
     * @return Current number of items
     */
    int GetCount() const {
        return count_;
    }

    /**
     * @brief Display all items
     *
     * Note: Iterates over vector - should trigger STL container
     * checklist items for concurrent access checks
     */
    void DisplayItems() const {
        for (const auto& item : items_) {
            if (item) {
                std::cout << "Item: " << *item << std::endl;
            }
        }
    }

private:
    int count_;
    std::vector<std::shared_ptr<std::string>> items_;
};

// Test function to demonstrate the skill
void TestGitCodeAutomationSkill() {
    TestSkillDemo demo;

    // Add some test items
    demo.AddItem("Test item 1");
    demo.AddItem("Test item 2");
    demo.AddItem("GitCode automation skill test");

    // Display results
    std::cout << "Total items: " << demo.GetCount() << std::endl;
    demo.DisplayItems();
}

int main() {
    std::cout << "=== GitCode Automation Skill Demo ===" << std::endl;
    TestGitCodeAutomationSkill();
    std::cout << "=== Demo Complete ===" << std::endl;
    return 0;
}

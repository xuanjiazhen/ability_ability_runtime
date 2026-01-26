/**
 * Test file for branch zhz_test_0126
 * Created: 2026-01-26
 * Author: zhang_hao_zheng
 */

#include <iostream>
#include <string>

namespace OHOS {
namespace AbilityRuntime {

// Simple test utility class
class TestUtility {
public:
    static std::string GetTestMessage() {
        return "Test branch zhz_test_0126 - Ability Runtime Test";
    }

    static int CalculateSum(int a, int b) {
        return a + b;
    }
};

} // namespace AbilityRuntime
} // namespace OHOS

int main() {
    std::cout << OHOS::AbilityRuntime::TestUtility::GetTestMessage() << std::endl;
    std::cout << "Sum of 10 and 20: "
              << OHOS::AbilityRuntime::TestUtility::CalculateSum(10, 20)
              << std::endl;
    return 0;
}

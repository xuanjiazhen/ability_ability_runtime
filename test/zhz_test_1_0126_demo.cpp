/**
 * Test Demo for Branch zhz_test_1_0126
 * Author: zhang_hao_zheng
 * Date: 2026-01-26
 * Description: Simple utility class for demonstration
 */

#include <iostream>
#include <string>
#include <vector>

namespace OHOS {
namespace AbilityRuntime {

class StringUtils {
public:
    // Trim whitespace from string
    static std::string Trim(const std::string& str) {
        if (str.empty()) {
            return str;
        }

        size_t start = 0;
        size_t end = str.length() - 1;

        while (start <= end && std::isspace(str[start])) {
            start++;
        }

        while (end >= start && std::isspace(str[end])) {
            end--;
        }

        return str.substr(start, end - start + 1);
    }

    // Split string by delimiter
    static std::vector<std::string> Split(const std::string& str, char delimiter) {
        std::vector<std::string> tokens;
        std::string token;
        size_t start = 0;
        size_t end = str.find(delimiter);

        while (end != std::string::npos) {
            token = str.substr(start, end - start);
            tokens.push_back(token);
            start = end + 1;
            end = str.find(delimiter, start);
        }

        tokens.push_back(str.substr(start));
        return tokens;
    }
};

} // namespace AbilityRuntime
} // namespace OHOS

int main() {
    // Test StringUtils
    std::string testStr = "  Hello, OpenHarmony!  ";
    std::cout << "Original: '" << testStr << "'" << std::endl;
    std::cout << "Trimmed: '" << OHOS::AbilityRuntime::StringUtils::Trim(testStr) << "'" << std::endl;

    // Test split
    std::string csv = "apple,banana,cherry";
    auto fruits = OHOS::AbilityRuntime::StringUtils::Split(csv, ',');
    std::cout << "\nFruits:" << std::endl;
    for (const auto& fruit : fruits) {
        std::cout << "  - " << fruit << std::endl;
    }

    return 0;
}

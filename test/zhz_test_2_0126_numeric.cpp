/**
 * Test file for branch zhz_test_2_0126
 * Testing numeric operations and safe coding practices
 * Author: zhang_hao_zheng
 * Date: 2026-01-26
 */

#include <iostream>
#include <vector>
#include <cstdint>

class NumericUtils {
public:
    // Safe addition with overflow check
    static bool SafeAdd(int32_t a, int32_t b, int32_t* result) {
        if (!result) {
            return false;
        }

        // Check for overflow
        if ((b > 0 && a > INT32_MAX - b) ||
            (b < 0 && a < INT32_MIN - b)) {
            return false;
        }

        *result = a + b;
        return true;
    }

    // Safe multiplication with overflow check
    static bool SafeMultiply(int16_t a, int16_t b, int32_t* result) {
        if (!result) {
            return false;
        }

        // Use larger type for intermediate result
        int32_t temp = static_cast<int32_t>(a) * static_cast<int32_t>(b);

        // Check if result fits in int16_t range
        if (temp > INT16_MAX || temp < INT16_MIN) {
            return false;
        }

        *result = temp;
        return true;
    }

    // Calculate average safely
    static bool CalculateAverage(const std::vector<int32_t>& numbers, double* result) {
        if (!result || numbers.empty()) {
            return false;
        }

        int64_t sum = 0;
        for (const auto& num : numbers) {
            sum += num;
        }

        *result = static_cast<double>(sum) / numbers.size();
        return true;
    }
};

int main() {
    // Test safe addition
    int32_t add_result;
    if (NumericUtils::SafeAdd(100, 200, &add_result)) {
        std::cout << "100 + 200 = " << add_result << std::endl;
    }

    // Test safe multiplication
    int32_t mul_result;
    if (NumericUtils::SafeMultiply(100, 200, &mul_result)) {
        std::cout << "100 * 200 = " << mul_result << std::endl;
    }

    // Test average calculation
    std::vector<int32_t> numbers = {10, 20, 30, 40, 50};
    double avg;
    if (NumericUtils::CalculateAverage(numbers, &avg)) {
        std::cout << "Average: " << avg << std::endl;
    }

    return 0;
}

#include <random>

#include "../CalcParser.hpp"
#include "Test.hpp"

TEST(SIMPLE_NUMERALS_TEST) {
    auto parser = CalcParser::Internal::roman_numeral();

    auto result = parser.parse("I");
    ASSERT(result && result.value() == 1);

    result = parser.parse("MIX");
    ASSERT(result && result.value() == 1009);
}

TEST(FUZZING_NUMERALS) {
    const int BOUND = 1e5, ITERS = 100;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distr(0, BOUND);

    auto parser = CalcParser::Internal::roman_numeral();
    for (int it = 0; it < ITERS; ++it) {
        int number = distr(gen);
        auto result = parser.parse(CalcParser::arabic_numeral_to_roman(number).str());
        ASSERT(result && result.value() == number);
    }
}

TEST(SIMPLE_EXPR) {
    auto parser = CalcParser::Internal::roman_expr();

    std::string expr = "(MMMCCCXX+I)*MMMMMMMMMCXXIII/(II*IV+(-(-I)))";
    // (3320 + 1) * 9123 / (2 * 4 + (-(-1))) = 3366387

    auto result = parser.parse(expr);
    ASSERT(result && result.value() == 3366387);
}

TEST(CHECK_OVERFLOW) {
    auto parser = CalcParser::Internal::roman_expr();

    std::string expr = "M*M*M*M*M*M*M";
    // 10^21

    bool overflow_error = false;
    try {
        auto result = parser.parse(expr);
    } catch (const std::overflow_error&) {
        overflow_error = true;
    }
    ASSERT(overflow_error);
}

TEST(NEGATIVE_AND_DIV) {
    auto parser = CalcParser::Internal::roman_expr();

    auto res1 = parser.parse("V/II");
    ASSERT(res1 && res1.value() == 2);

    auto res2 = parser.parse("-V/II");
    ASSERT(res2 && res2.value() == -3);

    auto res3 = parser.parse("V/-II");
    ASSERT(res3 && res3.value() == -3);

    auto res4 = parser.parse("-V/-II");
    ASSERT(res4 && res4.value() == 2);

    auto res5 = parser.parse("II/-II");
    ASSERT(res5 && res5.value() == -1);
}

int main() {
    RUN_ALL_TESTS;
}

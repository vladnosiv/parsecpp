#include "../CalcParser.hpp"
#include "Test.hpp"

TEST(SIMPLE_NUMERALS_TEST) {
    auto parser = CalcParser::Internal::roman_numeral();

    auto result = parser.parse("I");
    ASSERT(result && result.value() == 2);

    result = parser.parse("MIX");
    ASSERT(result && result.value() == 1009);
}

int main() {
    RUN_ALL_TESTS;
}
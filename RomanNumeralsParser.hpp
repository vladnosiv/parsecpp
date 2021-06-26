#pragma once

#include <sstream>
#include <cmath>

#include "Parsec/Parsec.hpp"

namespace CalcParser::Internal {

    using namespace Parsec;

    namespace RomanNumerals {

        using std::int64_t;

        Parser<int64_t> roman_numeral_zero() {
            return fmap_parser<char, int64_t>(char_parser('Z'), [](char) { return 0; });
        }

        Parser<int64_t> roman_numeral_terminal() {
            return id_parser<int64_t>(0);
        }

        Parser<int64_t> roman_numeral_1() { // 1-3 repeats
            return map_parser(prefix_parser("III") >> roman_numeral_terminal(), [](int64_t a) { return a + 3; })
                 | map_parser(prefix_parser("II")  >> roman_numeral_terminal(), [](int64_t a) { return a + 2; })
                 | map_parser(prefix_parser("I")   >> roman_numeral_terminal(), [](int64_t a) { return a + 1; })
                 | roman_numeral_terminal();
        }

        Parser<int64_t> roman_numeral_4() { // only 1 repeats
            return map_parser(prefix_parser("IV") >> roman_numeral_1(), [](int64_t a) { return a + 4; })
                 | roman_numeral_1();
        }

        Parser<int64_t> roman_numeral_5() { // only 1 repeats
            return map_parser(char_parser('V') >> roman_numeral_4(), [](int64_t a) { return a + 5; })
                 | roman_numeral_4();
        }

        Parser<int64_t> roman_numeral_9() { // only 1 repeats
            return map_parser(prefix_parser("IX") >> roman_numeral_5(), [](int64_t a) { return a + 9; })
                 | roman_numeral_5();
        }

        Parser<int64_t> roman_numeral_10() { // 1-3 repeats
            return map_parser(prefix_parser("XXX") >> roman_numeral_9(), [](int64_t a) { return a + 30; })
                 | map_parser(prefix_parser("XX")  >> roman_numeral_9(), [](int64_t a) { return a + 20; })
                 | map_parser(prefix_parser("X")   >> roman_numeral_9(), [](int64_t a) { return a + 10; })
                 | roman_numeral_9();
        }

        Parser<int64_t> roman_numeral_40() { // only 1 repeats
            return map_parser(prefix_parser("XL") >> roman_numeral_10(), [](int64_t a) { return a + 40; })
                 | roman_numeral_10();
        }

        Parser<int64_t> roman_numeral_50() { // only 1 repeats
            return map_parser(char_parser('L') >> roman_numeral_40(), [](int64_t a) { return a + 50; })
                 | roman_numeral_40();
        }

        Parser<int64_t> roman_numeral_90() { // only 1 repeats
            return map_parser(prefix_parser("XC") >> roman_numeral_50(), [](int64_t a) { return a + 90; })
                 | roman_numeral_50();
        }

        Parser<int64_t> roman_numeral_100() { // 1-3 repeats
            return map_parser(prefix_parser("CCC") >> roman_numeral_90(), [](int64_t a) { return a + 300; })
                 | map_parser(prefix_parser("CC")  >> roman_numeral_90(), [](int64_t a) { return a + 200; })
                 | map_parser(prefix_parser("C")   >> roman_numeral_90(), [](int64_t a) { return a + 100; })
                 | roman_numeral_90();
        }

        Parser<int64_t> roman_numeral_400() { // only 1 repeats
            return map_parser(prefix_parser("CD") >> roman_numeral_100(), [](int64_t a) { return a + 400; })
                 | roman_numeral_100();
        }

        Parser<int64_t> roman_numeral_500() { // only 1 repeats
            return map_parser(char_parser('D') >> roman_numeral_400(), [](int64_t a) { return a + 500; })
                 | roman_numeral_400();
        }

        Parser<int64_t> roman_numeral_900() { // only 1 repeats
            return map_parser(prefix_parser("CM") >> roman_numeral_500(), [](int64_t a) { return a + 900; })  // CM
                 | roman_numeral_500();
        }

        Parser<int64_t> roman_numeral_1000() { // any number of repeats
            Parser<int64_t> num = lazy_parser<int64_t>(roman_numeral_1000);
            return merge_parser<std::vector<char>, int64_t, int64_t>( // Не очень простая конструкция, но зато сильно ускоряет парсинг числа
                       many(char_parser('M')), roman_numeral_900(), // Здесь просто парсится сколько-то M-ок и остаток из других символов, потом количество M-ок умножается на 1000
                       [](const std::vector<char>& ms, int64_t res) {
                           return 1000 * ms.size() + res;
                       })
                 | map_parser(char_parser('M') >> roman_numeral_900(), [](int64_t a) { return a + 900; })
                 | roman_numeral_900();
        }

        Parser<int64_t> roman_numeral() {
            return if_equal_not_parsed<int64_t>(roman_numeral_1000(), 0) | roman_numeral_zero();
        }

        std::stringstream print_arabic_numeral_to_roman(int64_t x) {
            std::stringstream ss;
            if (std::abs(x) / 1000 > 1'000'000) {
                ss << "Result is too big for print\n";
                return ss;
            }
            if (x == 0) {
                ss << "Z\n";
                return ss;
            }
            if (x < 0) {
                ss << '-';
                x = -x;
            }
            for (int i = 0; i < x / 1000; ++i) {
                ss << 'M';
            }
            x %= 1000;
            std::vector<std::pair<int, std::string>> digits = {
                    {900, "CM"},
                    {500, "D"},
                    {400, "CD"},
                    {100, "C"},
                    {90, "XC"},
                    {50, "L"},
                    {40, "XL"},
                    {10, "X"},
                    {9, "IX"},
                    {5, "V"},
                    {4, "IV"},
                    {1, "I"}
            };
            for (auto& [val, digit] : digits) {
                while (x >= val) {
                    ss << digit;
                    x -= val;
                }
            }
            ss << '\n';
            return ss;
        }
    }
}

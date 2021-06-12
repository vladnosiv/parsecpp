#pragma once

#include "Parsec/Parsec.hpp"

namespace CalcParser {

    namespace Util {

        using namespace Parsec;

        Parser<int64_t> roman_numeral() {
            Parser<int64_t> maybe_num = maybe_parser<int64_t>(lazy_parser<int64_t>(roman_numeral), 0);
            return map_parser(char_parser('M') >> maybe_num, [](int64_t a) { return a + 1000; }) // M
                 | map_parser(char_parser('C') >> char_parser('M') >> maybe_num, [](int64_t a) { return a + 900; })  // CM
                 | map_parser(char_parser('D') >> maybe_num, [](int64_t a) { return a + 500; })  // D
                 | map_parser(char_parser('C') >> char_parser('D') >> maybe_num, [](int64_t a) { return a + 400; })  // CD
                 | map_parser(char_parser('C') >> maybe_num, [](int64_t a) { return a + 100; })  // C
                 | map_parser(char_parser('X') >> char_parser('C') >> maybe_num, [](int64_t a) { return a + 90; })   // XC
                 | map_parser(char_parser('L') >> maybe_num, [](int64_t a) { return a + 50; })   // L
                 | map_parser(char_parser('X') >> char_parser('L') >> maybe_num, [](int64_t a) { return a + 40; })   // XL
                 | map_parser(char_parser('X') >> maybe_num, [](int64_t a) { return a + 10; })   // X
                 | map_parser(char_parser('I') >> char_parser('X') >> maybe_num, [](int64_t a) { return a + 9; })    // IX
                 | map_parser(char_parser('V') >> maybe_num, [](int64_t a) { return a + 5; })    // V
                 | map_parser(char_parser('I') >> char_parser('V') >> maybe_num, [](int64_t a) { return a + 4; })     // IV
                 | map_parser(char_parser('I') >> maybe_num, [](int64_t a) { return a + 1; })    // I
                 | fmap_parser<char, int64_t>(char_parser('Z'), [](char) { return 0; });                            // Z
        }

        Parser<int64_t> roman_expr();
        Parser<int64_t> roman_atom();

        Parser<int64_t> roman_unary_minus_atom() {
            return map_parser(char_parser('-') >> lazy_parser<int64_t>(roman_atom), [](int64_t a) { return -a; });
        }

        Parser<int64_t> roman_brackets() {
            return brackets_parser(char_parser('('), lazy_parser<int64_t>(roman_expr), char_parser(')'));
        }

        Parser<int64_t> roman_atom() {
            return roman_numeral() | roman_unary_minus_atom() | roman_brackets();
        }

        Parser<int64_t> roman_mlt_div() {
            return fold(
                seq_save(roman_atom(), char_parser('*') | char_parser('/')),
                {
                        {'*', [](int64_t a, int64_t b) { return a * b; }},
                        {'/', [](int64_t a, int64_t b) { return a / b; }}
                }
                // В условии явно не написано, но судя по строчке про вычисления в лонгах, деление целочисленное
            );
        }

        Parser<int64_t> roman_expr() {
            return fold(
                seq_save(roman_mlt_div(), char_parser('+') | char_parser('-')),
                {
                        {'+', [](int64_t a, int64_t b) { return a + b; }},
                        {'-', [](int64_t a, int64_t b) { return a - b; }}
                }
            );
        }

    } // namespace Util

    Parsec::Parser<int64_t> roman_calc() {
        return Util::roman_expr();
    }

} // namespace CalcParser
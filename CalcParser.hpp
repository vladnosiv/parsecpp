#pragma once

#include "Parsec/Parsec.hpp"

namespace CalcParser {

    namespace Util {

        using namespace Parsec;

        Parser<int64_t> roman_numeral() {
            Parser<int64_t> maybe_num = maybe_parser<int64_t>(lazyParser<int64_t>(roman_numeral), 0);
            return mapParser(charParser('M')                    >> maybe_num, [](int64_t a) { return a + 1000; }) // M
                 | mapParser(charParser('C') >> charParser('M') >> maybe_num, [](int64_t a) { return a + 900; })  // CM
                 | mapParser(charParser('D')                    >> maybe_num, [](int64_t a) { return a + 500; })  // D
                 | mapParser(charParser('C') >> charParser('D') >> maybe_num, [](int64_t a) { return a + 400; })  // CD
                 | mapParser(charParser('C')                    >> maybe_num, [](int64_t a) { return a + 100; })  // C
                 | mapParser(charParser('X') >> charParser('C') >> maybe_num, [](int64_t a) { return a + 90; })   // XC
                 | mapParser(charParser('L')                    >> maybe_num, [](int64_t a) { return a + 50; })   // L
                 | mapParser(charParser('X') >> charParser('L') >> maybe_num, [](int64_t a) { return a + 40; })   // XL
                 | mapParser(charParser('X')                    >> maybe_num, [](int64_t a) { return a + 10; })   // X
                 | mapParser(charParser('I') >> charParser('X') >> maybe_num, [](int64_t a) { return a + 9; })    // IX
                 | mapParser(charParser('V')                    >> maybe_num, [](int64_t a) { return a + 5; })    // V
                 | mapParser(charParser('I') >> charParser('V') >> maybe_num, [](int64_t a) { return a + 4;})     // IV
                 | mapParser(charParser('I')                    >> maybe_num, [](int64_t a) { return a + 1; })    // I
                 | fmapParser<char, int64_t>(charParser('Z'), [](char) { return 0; });                            // Z
        }

        Parser<int64_t> roman_expr();
        Parser<int64_t> roman_atom();

        Parser<int64_t> roman_unary_minus_atom() {
            return mapParser(charParser('-') >> lazyParser<int64_t>(roman_atom), [](int64_t a) { return -a; });
        }

        Parser<int64_t> roman_brackets() {
            return brackets_parser(charParser('('), lazyParser<int64_t>(roman_expr), charParser(')'));
        }

        Parser<int64_t> roman_atom() {
            return roman_numeral() | roman_unary_minus_atom() | roman_brackets();
        }

        Parser<int64_t> roman_mlt_div() {
            return fold(
                seq_save(roman_atom(), charParser('*') | charParser('/')),
                {
                        {'*', [](int64_t a, int64_t b) { return a * b; }},
                        {'/', [](int64_t a, int64_t b) { return a / b; }}
                }
                // В условии явно не написано, но судя по строчке про вычисления в лонгах, деление целочисленное
            );
        }

        Parser<int64_t> roman_expr() {
            return fold(
                seq_save(roman_mlt_div(), charParser('+') | charParser('-')),
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
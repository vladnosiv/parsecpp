#pragma once

#include <cstdint>

#include "Parsec/Parsec.hpp"
#include "RomanNumeralsParser.hpp"

namespace CalcParser {

    namespace Internal {

        using namespace Parsec;

        Parser<int64_t> roman_expr();
        Parser<int64_t> roman_atom();

        Parser<int64_t> roman_numeral() {
            return RomanNumerals::roman_numeral();
        }

        Parser<int64_t> roman_unary_minus_atom() {
            return map_parser(char_parser('-') >> lazy_parser<int64_t>(roman_atom), [](int64_t a) { return -a; });
        }

        Parser<int64_t> roman_brackets() {
            // На самом деле исходя из грамматики, здесь должен быть только один второй случай,
            // Но если сделать так, то слишком просто построить пример, на котором парсер работает медленно
            // А так начинают быстрее работать всякие ((((((I)))))) и похожие примеры
            return brackets_parser(char_parser('('), lazy_parser<int64_t>(roman_brackets), char_parser(')'))
                 | brackets_parser(char_parser('('), lazy_parser<int64_t>(roman_expr), char_parser(')'));
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

    } // namespace Internal

    Parsec::Parser<int64_t> roman_calc() {
        return Internal::roman_expr();
    }

} // namespace CalcParser
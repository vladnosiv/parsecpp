#pragma once

#include "Parser.hpp"

namespace CalcParser {

    /**
     * Grammar.
     *
     * E -> T {+-} T
     * T -> F {mlt div} F
     * F -> RomanNum
     * F -> -F
     * F -> (E)
     */

    namespace Util {

        struct VRomanNumeralParser : Parser::Util::VParser<long long> {
            std::optional<std::pair<long long, std::string>> eval(std::string str) override {
                long long result = 0;
                bool parsed = false, parsed_new_digit = true;
                std::size_t str_index = 0;
                while (parsed_new_digit) {
                    parsed_new_digit = false;
                    for (const auto& i : converting_table) {
                        std::string digit = i.first;
                        if (str_index + digit.size() <= str.size() && str[str_index] == digit[0]
                            && (digit.size() == 1 || str[str_index + 1] == digit[1])) {

                            parsed = parsed_new_digit = true;
                            result += i.second;
                            str_index += digit.size();
                        }
                    }
                }
                if (!parsed) {
                    return std::nullopt;
                }
                return std::make_pair(result, str.substr(str_index));
            }

            const std::vector<std::pair<std::string, int>> converting_table = {
                    {"M", 1000},
                    {"CM", 900},
                    {"D", 500},
                    {"CD", 400},
                    {"C", 100},
                    {"XC", 90},
                    {"L", 50},
                    {"XL", 40},
                    {"X", 10},
                    {"IX", 9},
                    {"V", 5},
                    {"IV", 4},
                    {"I", 1},
                    {"Z", 0}
            };
        };

        Parser::Parser<long long> romanNumeral() {
            return Parser::Parser<long long>(std::make_shared<VRomanNumeralParser>());
        }

        Parser::Parser<long long> romanExpr();
        Parser::Parser<long long> romanAtom();

        Parser::Parser<long long> romanUnaryMinusAtom() {
            struct VUnaryMinusParser : Parser::Util::VParser<long long> {
                std::optional<std::pair<long long, std::string>> eval(std::string s) override {
                    auto res_minus = Parser::charParser('-').eval(s);
                    if (!res_minus.has_value()) {
                        return std::nullopt;
                    }
                    auto res = romanAtom().eval(res_minus.value().second);
                    if (!res.has_value()) {
                        return std::nullopt;
                    }
                    return std::make_pair(-res.value().first, res.value().second);
                }
            };
            return Parser::Parser<long long>(std::make_shared<VUnaryMinusParser>());
        }

        Parser::Parser<long long> romanBrackets() {
            struct RomanBrParser : Parser::Util::VParser<long long> {
                std::optional<std::pair<long long, std::string>> eval(std::string str) override {
                    auto left_result = Parser::charParser('(').eval(str);
                    if (!left_result.has_value()) {
                        return std::nullopt;
                    }
                    str = left_result.value().second;
                    auto elem_result = romanExpr().eval(str);
                    if (!elem_result.has_value()) {
                        return std::nullopt;
                    }
                    long long result = elem_result.value().first;
                    str = elem_result.value().second;
                    auto right_result = Parser::charParser(')').eval(str);
                    if (!right_result.has_value()) {
                        return std::nullopt;
                    }
                    return std::make_pair(result, right_result.value().second);
                }
            };
            return Parser::Parser<long long>(std::make_shared<RomanBrParser>());
        }

        Parser::Parser<long long> romanAtom() {
            return romanNumeral() | romanUnaryMinusAtom() | romanBrackets();
        }

        Parser::Parser<long long> romanMltDiv() {
            return Parser::fold(
                Parser::seq_save(romanAtom(), Parser::charParser('*') | Parser::charParser('/')),
                {
                        {'*', [](long long a, long long b) { return a * b; }},
                        {'/', [](long long a, long long b) { return a / b; }}
                }
                // В условии явно не написано, но судя по строчке про вычисления в лонгах, деление целочисленное
            );
        }

        Parser::Parser<long long> romanExpr() {
            return Parser::fold(
                Parser::seq_save(romanMltDiv(), Parser::charParser('+') | Parser::charParser('-')),
                {
                        {'+', [](long long a, long long b) -> long long { return a + b; }},
                        {'-', [](long long a, long long b) -> long long { return a - b; }}
                }
            );
        }

    } // namespace Util

    Parser::Parser<long long> romanCalc() {
        return Util::romanExpr();
    }

} // namespace CalcParser
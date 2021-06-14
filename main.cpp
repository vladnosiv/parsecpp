#include "CalcParser.hpp"

#include <string>
#include <iostream>

int main() {
    auto parser = CalcParser::roman_calc();

    std::string str;
    while (std::getline(std::cin, str)) {
        str = CalcParser::remove_all_spaces(str);
        try {
            auto result = parser.parse(str);
            if (result && result.rest().empty()) {
                std::cout << CalcParser::arabic_numeral_to_roman(result.value()).str();
            } else if (result) {
                std::cout << "error: Parsing failed. Part from position " << str.size() - result.rest().size() + 1 << " not parsed." << '\n';
            } else {
                std::cout << "error: Parsing failed. Message: " << result.get_message() << '\n';
            }
        } catch (const std::overflow_error&) {
            std::cout << "error: Overflow int64 error.\n";
        }
    }
}
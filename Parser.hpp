#pragma once

#include <optional>
#include <string>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>

namespace Parser {

    template<typename T>
    struct Parser;

    namespace Util {
        template<typename T>
        struct VParser {
            virtual std::optional<std::pair<T, std::string>> eval(std::string) = 0;
        };

        template<typename T>
        struct VAlternativeParser : VParser<T> {
            VAlternativeParser(Parser<T> fst_, Parser<T> snd_)
                    : fst(std::move(fst_)), snd(std::move(snd_)) {}

            std::optional<std::pair<T, std::string>> eval(std::string s) override {
                auto fst_result = fst.eval(s);
                if (fst_result.has_value()) {
                    return fst_result;
                }
                return snd.eval(s);
            }

        private:
            Parser<T> fst, snd;
        };

        struct VCharParser : VParser<char> {
            explicit VCharParser(char target_) : target(target_) {}

            std::optional<std::pair<char, std::string>> eval(std::string str) override {
                if (str.empty() || str[0] != target) {
                    return std::nullopt;
                }
                return std::make_pair(target, str.substr(1));
            }

        private:
            char target = 0;
        };

        struct VCharsParser : VParser<char> {
            explicit VCharsParser(std::vector<char> chars) : targets(std::move(chars)) {}

            std::optional<std::pair<char, std::string>> eval(std::string str) override {
                if (str.empty()) {
                    return std::nullopt;
                }
                for (char c : targets) {
                    if (str[0] == c) {
                        return std::make_pair(c, str.substr(1));
                    }
                }
                return std::nullopt;
            }

        private:
            std::vector<char> targets;
        };

        template<typename T>
        struct VManyParser : VParser<T> {
            explicit VManyParser(Parser<T> p) : parser(std::move(p)) {}

            std::optional<std::pair<T, std::string>> eval(std::string str) override {
                std::optional<std::pair<T, std::string>> result;
                while (true) {
                    auto current_result = parser.eval(str);
                    if (!current_result.has_value()) {
                        return result;
                    }
                    result = current_result;
                    str = current_result.value().second;
                }
            }
        private:
            Parser<T> parser;
        };

        template<typename U, typename T>
        struct VSkipParser : VParser<T> {
            explicit VSkipParser(Parser<U> skip_parser_, Parser<T> parser_)
                : skip_parser(skip_parser_), parser(parser_) {}

            std::optional<std::pair<T, std::string>> eval(std::string str) override {
                auto res_skip = skip_parser.eval(str);
                if (res_skip.has_value()) {
                    str = res_skip.value().second;
                }
                return parser.eval(str);
            }
        private:
            Parser<U> skip_parser;
            Parser<T> parser;
        };

    } // namespace Util


    template<typename T>
    struct Parser {
        explicit Parser(std::shared_ptr<Util::VParser<T>> ptr) {
            parser = ptr;
        }

        std::optional<std::pair<T, std::string>> eval(std::string s) {
            return parser->eval(std::move(s));
        }

    private:
        std::shared_ptr<Util::VParser<T>> parser;
    };

    template<typename T>
    Parser<T> operator|(Parser<T> alt1, Parser<T> alt2) {
        return Parser<T>(std::make_shared<Util::VAlternativeParser<T>>(alt1, alt2));
    }

    template<typename U, typename T>
    Parser<T> operator>>(Parser<U> skip_parser, Parser<T> parser) {
        return Parser<T>(std::make_shared<Util::VSkipParser<U, T>>(skip_parser, parser));
    }

    Parser<char> charParser(char c) {
        return Parser<char>(std::make_shared<Util::VCharParser>(c));
    }

    Parser<char> charsParser(std::vector<char> chars) {
        return Parser<char>(std::make_shared<Util::VCharsParser>(std::move(chars)));
    }

    template<typename T>
    Parser<T> many(Parser<T> parser) {
        return Parser<T>(std::make_shared<Util::VManyParser<T>>(std::move(parser)));
    }

    Parser<char> space() {
        //TODO: maybe more special symbols
        return charsParser({' ', '\t'});
    }

    Parser<char> spaces() {
        return many(space());
    }

    Parser<char> alpha() {
        std::vector<char> alpha;
        for (char c = 'a'; c <= 'z'; ++c) {
            alpha.push_back(c);
        }
        return charsParser(alpha);
    }

    Parser<char> num() {
        std::vector<char> digits;
        for (char c = '0'; c <= '9'; ++c) {
            digits.push_back(c);
        }
        return charsParser(digits);
    }

    Parser<char> alphaNum() {
        return alpha() | num();
    }

} // namespace Parser
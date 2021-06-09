#pragma once

#include <optional>
#include <string>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>
#include <functional>

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
        struct VManyParser : VParser<std::vector<T>> {
            explicit VManyParser(Parser<T> p) : parser(std::move(p)) {}

            std::optional<std::pair<std::vector<T>, std::string>> eval(std::string str) override {
                std::vector<T> results;
                while (true) {
                    auto current = parser.eval(str);
                    if (!current.has_value()) {
                        break;
                    }
                    results.push_back(current.value().first);
                    str = current.value().second;
                }
                return std::make_pair(results, str);
            }
        private:
            Parser<T> parser;
        };

        // like VManyParser but ignores the second occurrence and beyond
        template<typename T>
        struct VManyIgnoreParser : VParser<T> {
            explicit VManyIgnoreParser(Parser<T> p) : parser(std::move(p)) {}

            std::optional<std::pair<T, std::string>> eval(std::string str) override {
                auto many = Parser<std::vector<T>>(std::make_shared<Util::VManyParser<T>>(parser));
                auto many_result = many.eval(str);
                if (many_result.value().first.empty()) {
                    return std::make_pair(0, many_result.value().second); //TODO: убрать костыль с чаром 0
                }
                return std::make_pair(many_result.value().first[0], many_result.value().second);
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

        template<typename T, typename U>
        struct VSeqParser : VParser<std::vector<T>> {
            explicit VSeqParser(Parser<T> elem_parser_, Parser<U> sep_parser_)
                    : elem_parser(elem_parser_), sep_parser(sep_parser_) {}

            std::optional<std::pair<std::vector<T>, std::string>> eval(std::string str) override {
                auto head = elem_parser.eval(str);
                if (!head.has_value()) {
                    return std::nullopt;
                }
                std::vector<T> results = {head.value().first};
                str = head.value().second;
                auto tail_parser = many(sep_parser >> elem_parser);
                auto tail = tail_parser.eval(str);
                for (T t : tail.value().first) {
                    results.push_back(t);
                }
                return std::make_pair(results, tail.value().second);
            }
        private:
            Parser<T> elem_parser;
            Parser<U> sep_parser;
        };

        template<typename T, typename BL, typename BR>
        struct VBrParser : VParser<T> {
            explicit VBrParser(Parser<T> elem_parser_,
                               Parser<BL> left_parser_,
                               Parser<BR> right_parser_)
                : elem_parser(elem_parser_), left_parser(left_parser_), right_parser(right_parser_) {}

            std::optional<std::pair<T, std::string>> eval(std::string str) override {
                std::cerr << "IN BR PARSER" << std::endl;
                auto left_result = left_parser.eval(str);
                if (!left_result.has_value()) {
                    return std::nullopt;
                }
                str = left_result.value().second;
                auto elem_result = elem_parser.eval(str);
                if (!elem_result.has_value()) {
                    return std::nullopt;
                }
                T result = elem_result.value().first;
                str = elem_result.value().second;
                auto right_result = right_parser.eval(str);
                if (!right_result.has_value()) {
                    return std::nullopt;
                }
                return std::make_pair(result, right_result.value().second);
            }
        private:
            Parser<T> elem_parser;
            Parser<BL> left_parser;
            Parser<BR> right_parser;
        };

        // like VSeqParser but save separators too
        template<typename T, typename U>
        struct VSeqSaverParser : VParser<std::pair<std::vector<T>, std::vector<U>>> {
            explicit VSeqSaverParser(Parser<T> elem_parser_, Parser<U> sep_parser_)
                : elem_parser(elem_parser_), sep_parser(sep_parser_) {}

            std::optional<std::pair<std::pair<std::vector<T>, std::vector<U>>, std::string>> eval(std::string str) override {
                auto head = elem_parser.eval(str);
                if (!head.has_value()) {
                    return std::nullopt;
                }
                std::vector<T> results = {head.value().first};
                std::vector<U> seps;
                str = head.value().second;
                while (true) {
                    auto sep_result = sep_parser.eval(str);
                    if (!sep_result.has_value()) {
                        break;
                    }
                    auto elem_result = elem_parser.eval(sep_result.value().second);
                    if (!elem_result.has_value()) {
                        break;
                    }
                    str = elem_result.value().second;
                    results.push_back(elem_result.value().first);
                    seps.push_back(sep_result.value().first);
                }
                return std::make_pair(std::make_pair(results, seps), str);
            }
        private:
            Parser<T> elem_parser;
            Parser<U> sep_parser;
        };

        template<typename T, typename U>
        struct VFoldParser : VParser<T> {
            explicit VFoldParser(Parser<std::pair<std::vector<T>, std::vector<U>>> parser_, std::vector<std::pair<U, std::function<T(T, T)>>> operators_)
                : parser(parser_), operators(operators_) {}

            std::optional<std::pair<T, std::string>> eval(std::string str) override {
                auto result = parser.eval(str);
                if (!result.has_value()) {
                    return std::nullopt;
                }
                auto [elements, seps] = result.value().first;
                T t_result = elements[0];
                for (std::size_t i = 1; i < elements.size(); ++i) {
                    for (const auto& [op, func] : operators) {
                        if (op == seps[i - 1]) {
                            t_result = func(t_result, elements[i]);
                            break;
                        }
                    }
                }
                return std::make_pair(t_result, result.value().second);
            }
        private:
            Parser<std::pair<std::vector<T>, std::vector<U>>> parser;
            std::vector<std::pair<U, std::function<T(T, T)>>> operators;
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
    Parser<std::vector<T>> many(Parser<T> parser) {
        return Parser<std::vector<T>>(std::make_shared<Util::VManyParser<T>>(std::move(parser)));
    }

    Parser<char> space() {
        //TODO: maybe more special symbols
        return charsParser({' ', '\t'});
    }

    Parser<char> spaces() {
        return Parser<char>(std::make_shared<Util::VManyIgnoreParser<char>>(space()));
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

    template<typename T, typename U>
    Parser<std::vector<T>> seq(Parser<T> elem_parser, Parser<U> sep_parser) {
        return Parser<std::vector<T>>(
                std::make_shared<Util::VSeqParser<T, U>>(
                        std::move(elem_parser), std::move(sep_parser)
                )
        );
    }

    template<typename T, typename U>
    Parser<std::pair<std::vector<T>, std::vector<U>>> seq_save(Parser<T> elem_parser, Parser<U> sep_parser) {
        return Parser<std::pair<std::vector<T>, std::vector<U>>>(
                std::make_shared<Util::VSeqSaverParser<T, U>>(
                        std::move(elem_parser), std::move(sep_parser)
                )
        );
    }

    template<typename T, typename BL, typename BR>
    Parser<T> brackets_parser(Parser<T> elem_parser, Parser<BL> left_parser, Parser<BR> right_parser) {
        return Parser<T>(std::make_shared<Util::VBrParser<T, BL, BR>>(std::move(elem_parser), std::move(left_parser), std::move(right_parser)));
    }

    template<typename T, typename U>
    Parser<T> fold(Parser<std::pair<std::vector<T>, std::vector<U>>> vec_parser, std::vector<std::pair<U, std::function<T(T, T)>>> operators) {
        return Parser<T>(std::make_shared<Util::VFoldParser<T, U>>(std::move(vec_parser), std::move(operators)));
    }

} // namespace Parser
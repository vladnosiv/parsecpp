#pragma once

namespace Parsec {

    template<typename T>
    struct Parser;

    namespace Util {

        template<typename T>
        struct Result {
            explicit Result() = default;
            explicit Result(T t, std::string s)
                : tvalue(std::move(t)), srest(std::move(s)), has_value(true) {}

            explicit operator bool() { return has_value; }
            T value() { return tvalue; }
            std::string rest() { return srest; }

        private:
            T tvalue;
            std::string srest;
            bool has_value = false;
        };

        template<typename T>
        Result<T> nullres() { return Result<T>(); }

        template<typename T>
        struct VParser {
            virtual Result<T> eval(std::string) = 0;
        };

        template<typename T>
        struct VAlternativeParser : VParser<T> {
            VAlternativeParser(Parser<T> fst_, Parser<T> snd_)
                    : fst(std::move(fst_)), snd(std::move(snd_)) {}

            Result<T> eval(std::string s) override {
                auto fst_result = fst.eval(s);
                if (fst_result) {
                    return fst_result;
                }
                return snd.eval(s);
            }

        private:
            Parser<T> fst, snd;
        };

        struct VCharParser : VParser<char> {
            explicit VCharParser(char target_) : target(target_) {}

            Result<char> eval(std::string str) override {
                if (str.empty() || str[0] != target) {
                    return nullres<char>();
                }
                std::string rest = str.substr(1);
                return Result<char>{target, std::move(rest)};
            }

        private:
            char target = 0;
        };

        struct VCharsParser : VParser<char> {
            explicit VCharsParser(std::vector<char> chars) : targets(std::move(chars)) {}

            Result<char> eval(std::string str) override {
                if (str.empty()) {
                    return nullres<char>();
                }
                for (char c : targets) {
                    if (str[0] == c) {
                        std::string rest = str.substr(1);
                        return Result<char>{c, std::move(rest)};
                    }
                }
                return nullres<char>();
            }

        private:
            std::vector<char> targets;
        };

        template<typename T>
        struct VManyParser : VParser<std::vector<T>> {
            explicit VManyParser(Parser<T> p) : parser(std::move(p)) {}

            Result<std::vector<T>> eval(std::string str) override {
                std::vector<T> results;
                while (true) {
                    auto current_res = parser.eval(str);
                    if (!current_res) {
                        break;
                    }
                    results.push_back(current_res.value());
                    str = current_res.rest();
                }
                return Result<std::vector<T>>{results, str};
            }
        private:
            Parser<T> parser;
        };

        // like VManyParser but ignores the second occurrence and beyond
        template<typename T>
        struct VManyIgnoreParser : VParser<T> {
            explicit VManyIgnoreParser(Parser<T> p) : parser(std::move(p)) {}

            Result<T> eval(std::string str) override {
                auto many = Parser<std::vector<T>>(std::make_shared<Util::VManyParser<T>>(parser));
                auto many_result = many.eval(str);
                if (many_result.value().empty()) {
                    return Result<T>{0, many_result.rest()}; //TODO: убрать костыль с чаром 0
                }
                return Result<T>{many_result.value()[0], many_result.rest()};
            }
        private:
            Parser<T> parser;
        };

        template<typename U, typename T>
        struct VSkipParser : VParser<T> {
            explicit VSkipParser(Parser<U> skip_parser_, Parser<T> parser_)
                    : skip_parser(skip_parser_), parser(parser_) {}

            Result<T> eval(std::string str) override {
                auto res_skip = skip_parser.eval(str);
                if (!res_skip) {
                    return nullres<T>();
                }
                str = res_skip.rest();
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

            Result<std::vector<T>> eval(std::string str) override {
                auto head = elem_parser.eval(str);
                if (!head) {
                    return nullres<std::vector<T>>();
                }
                std::vector<T> results = {head.value()};
                str = head.rest();
                auto tail_parser = many(sep_parser >> elem_parser);
                auto tail = tail_parser.eval(str);
                for (T t : tail.value()) {
                    results.push_back(t);
                }
                return Result<std::vector<T>>{results, tail.rest()};
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

            Result<T> eval(std::string str) override {
                auto left_result = left_parser.eval(str);
                if (!left_result) {
                    return nullres<T>();
                }
                str = left_result.rest();
                auto elem_result = elem_parser.eval(str);
                if (!elem_result) {
                    return nullres<T>();
                }
                T result = elem_result.value();
                str = elem_result.rest();
                auto right_result = right_parser.eval(str);
                if (!right_result) {
                    return nullres<T>();
                }
                return Result<T>{result, right_result.rest()};
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

            Result<std::pair<std::vector<T>, std::vector<U>>> eval(std::string str) override {
                auto head = elem_parser.eval(str);
                if (!head) {
                    return nullres<std::pair<std::vector<T>, std::vector<U>>>();
                }
                std::vector<T> results = {head.value()};
                std::vector<U> seps;
                str = head.rest();
                while (true) {
                    auto sep_result = sep_parser.eval(str);
                    if (!sep_result) {
                        break;
                    }
                    auto elem_result = elem_parser.eval(sep_result.rest());
                    if (!elem_result) {
                        break;
                    }
                    str = elem_result.rest();
                    results.push_back(elem_result.value());
                    seps.push_back(sep_result.value());
                }
                return Result<std::pair<std::vector<T>, std::vector<U>>>{std::make_pair(results, seps), str};
            }
        private:
            Parser<T> elem_parser;
            Parser<U> sep_parser;
        };

        template<typename T, typename U>
        struct VFoldParser : VParser<T> {
            explicit VFoldParser(Parser<std::pair<std::vector<T>, std::vector<U>>> parser_, std::vector<std::pair<U, std::function<T(T, T)>>> operators_)
            : parser(parser_), operators(operators_) {}

            Result<T> eval(std::string str) override {
                auto result = parser.eval(str);
                if (!result) {
                    return nullres<T>();
                }
                auto [elements, seps] = result.value();
                T t_result = elements[0];
                for (std::size_t i = 1; i < elements.size(); ++i) {
                    for (const auto& [op, func] : operators) {
                        if (op == seps[i - 1]) {
                            t_result = func(t_result, elements[i]);
                            break;
                        }
                    }
                }
                return Result<T>{t_result, result.rest()};
            }
        private:
            Parser<std::pair<std::vector<T>, std::vector<U>>> parser;
            std::vector<std::pair<U, std::function<T(T, T)>>> operators;
        };

        template<typename T, typename R, typename Func>
        struct VFMapParser : VParser<R> {
            explicit VFMapParser(Parser<T> parser_, Func f_)
                    : parser(parser_), f(f_) {}

            Result<R> eval(std::string str) override {
                auto result = parser.eval(str);
                if (!result) {
                    return nullres<R>();
                }
                return Result<R>{f(result.value()), result.rest()};
            }
        private:
            Parser<T> parser;
            Func f;
        };

        template<typename T>
        struct VLazyParser : VParser<T> {
            explicit VLazyParser(std::function<Parser<T>()> get_parser_)
                    : get_parser(std::move(get_parser_)) {}

            Result<T> eval(std::string str) override {
                return get_parser().eval(std::move(str));
            }
        private:
            std::function<Parser<T>()> get_parser;
        };

        template<typename T>
        struct VMaybeParser : VParser<T> {
            explicit VMaybeParser(Parser<T> parser_, T default_value_)
                    : parser(parser_), default_value(default_value_) {}

            Result<T> eval(std::string str) override {
                auto result = parser.eval(str);
                if (!result) {
                    return Result<T>{default_value, str};
                }
                return result;
            }
        private:
            Parser<T> parser;
            T default_value;
        };

    } // namespace Util

} // namespace Parsec
#pragma once

namespace Parsec {

    template<typename T>
    struct Parser;

    namespace Internal {

        template<typename T>
        struct Result {
            explicit Result() = default;
            explicit Result(T t, std::string_view s)
                : tvalue(std::move(t)), srest(s), has_value(true) {}

            explicit operator bool() { return has_value; }
            T value() { return tvalue; }
            std::string_view rest() { return srest; }

            void set_error(std::string msg) {
                has_value = false;
                error_message = std::move(msg);
            }
            std::string get_message() { return error_message; }

        private:
            T tvalue;
            std::string_view srest;
            bool has_value = false;
            std::string error_message;
        };

        template<typename T>
        Result<T> nullres(std::string message) {
            Result<T> res;
            res.set_error(std::move(message));
            return res;
        }

        template<typename T>
        struct IParser {
            virtual Result<T> parse(std::string_view) = 0;
            virtual ~IParser() = default;
        };

        template<typename T>
        struct IAlternativeParser : IParser<T> {
            IAlternativeParser(Parser<T> fst_, Parser<T> snd_)
                    : fst(std::move(fst_)), snd(std::move(snd_)) {}

            Result<T> parse(std::string_view s) override {
                auto fst_result = fst.parse(s);
                if (fst_result) {
                    return fst_result;
                }
                return snd.parse(s);
            }

        private:
            Parser<T> fst, snd;
        };

        struct ICharParser : IParser<char> {
            explicit ICharParser(char target_) : target(target_) {}

            Result<char> parse(std::string_view str) override {
                if (str.empty() || str[0] != target) {
                    std::string msg = "Expected ";
                    msg.push_back(target);
                    msg.append(". But received ");
                    msg.push_back(str[0]);
                    return nullres<char>(std::move(msg));
                }
                std::string_view rest = str.substr(1);
                return Result<char>{target, rest};
            }

        private:
            char target = 0;
        };

        struct ICharsParser : IParser<char> {
            explicit ICharsParser(std::vector<char> chars) : targets(std::move(chars)) {}

            Result<char> parse(std::string_view str) override {
                if (str.empty()) {
                    return nullres<char>("Expected chars but string is empty.");
                }
                for (char c : targets) {
                    if (str[0] == c) {
                        std::string_view rest = str.substr(1);
                        return Result<char>{c, rest};
                    }
                }
                return nullres<char>("Expected chars but not matched");
            }

        private:
            std::vector<char> targets;
        };

        struct IPrefixParser : IParser<std::string_view> {
            explicit IPrefixParser(std::string_view target_)
                : target(target_) {}

            Result<std::string_view> parse(std::string_view str) override {
                bool starts_with_target = true;
                for (std::size_t i = 0; i < target.size(); ++i) {
                    starts_with_target &= target[i] == str[i];
                }
                if (!starts_with_target) {
                    std::string msg = "Expected prefix ";
                    msg += target;
                    return nullres<std::string_view>(std::move(msg));
                }
                return Result<std::string_view>(target, str.substr(target.size()));
            }
        private:
            std::string_view target;
        };

        template<typename T>
        struct IManyParser : IParser<std::vector<T>> {
            explicit IManyParser(Parser<T> p) : parser(std::move(p)) {}

            Result<std::vector<T>> parse(std::string_view str) override {
                std::vector<T> results;
                while (true) {
                    auto current_res = parser.parse(str);
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

        // like IManyParser but ignores the second occurrence and beyond
        template<typename T>
        struct IManyIgnoreParser : IParser<T> {
            explicit IManyIgnoreParser(Parser<T> p) : parser(std::move(p)) {}

            Result<T> parse(std::string_view str) override {
                auto many = Parser<std::vector<T>>(std::make_shared<Internal::IManyParser<T>>(parser));
                auto many_result = many.parse(str);
                if (many_result.value().empty()) {
                    return Result<T>{0, many_result.rest()};
                }
                return Result<T>{many_result.value()[0], many_result.rest()};
            }
        private:
            Parser<T> parser;
        };

        template<typename T, typename U, typename R, typename Func>
        struct IMergeParser : IParser<R> {
            explicit IMergeParser(Parser<T> p1_, Parser<U> p2_, Func f_)
                : p1(std::move(p1_)), p2(std::move(p2_)), f(std::move(f_)) {}

            Result<R> parse(std::string_view str) override {
                auto res1 = p1.parse(str);
                if (!res1) {
                    return nullres<R>(res1.get_message());
                }
                auto res2 = p2.parse(res1.rest());
                if (!res2) {
                    return nullres<R>(res2.get_message());
                }
                return Result<R>(f(res1.value(), res2.value()), res2.rest());
            }
        private:
            Parser<T> p1;
            Parser<U> p2;
            Func f;
        };

        template<typename T>
        struct IEmptyParser : IParser<T> {
            explicit IEmptyParser(T t) : target(std::move(t)) {}

            Result<T> parse(std::string_view str) override {
                if (!str.empty()) {
                    return nullres<T>("Expected empty string.");
                }
                return Result<T>(target, str);
            }
        private:
            T target;
        };

        template<typename T>
        struct INotEmptyParser : IParser<T> {
            explicit INotEmptyParser(Parser<T> parser_)
                : parser(std::move(parser_)) {}

            Result<T> parse(std::string_view str) override {
                if (str.empty()) {
                    return nullres<T>("Expected not empty string.");
                }
                return parser.parse(str);
            }
        private:
            Parser<T> parser;
        };

        template<typename U, typename T>
        struct ISkipParser : IParser<T> {
            explicit ISkipParser(Parser<U> skip_parser_, Parser<T> parser_)
                    : skip_parser(skip_parser_), parser(parser_) {}

            Result<T> parse(std::string_view str) override {
                auto res_skip = skip_parser.parse(str);
                if (!res_skip) {
                    return nullres<T>(res_skip.get_message());
                }
                str = res_skip.rest();
                return parser.parse(str);
            }
        private:
            Parser<U> skip_parser;
            Parser<T> parser;
        };

        template<typename T, typename U>
        struct ISeqParser : IParser<std::vector<T>> {
            explicit ISeqParser(Parser<T> elem_parser_, Parser<U> sep_parser_)
                    : elem_parser(elem_parser_), sep_parser(sep_parser_) {}

            Result<std::vector<T>> parse(std::string_view str) override {
                auto head = elem_parser.parse(str);
                if (!head) {
                    return nullres<std::vector<T>>(head.get_message());
                }
                std::vector<T> results = {head.value()};
                str = head.rest();
                auto tail_parser = many(sep_parser >> elem_parser);
                auto tail = tail_parser.parse(str);
                for (T t : tail.value()) {
                    results.push_back(t);
                }
                return Result<std::vector<T>>{results, tail.rest()};
            }
        private:
            Parser<T> elem_parser;
            Parser<U> sep_parser;
        };

        template<typename T>
        struct IBanParser : IParser<T> {
            explicit IBanParser(Parser<T> parser_, T val)
                : parser(std::move(parser_)), ban_value(std::move(val)) {}

            Result<T> parse(std::string_view str) override {
                auto res = parser.parse(str);
                if (!res) {
                    return nullres<T>(res.get_message());
                }
                if (res.value() == ban_value) {
                    return nullres<T>("Expected any value except banned value.");
                }
                return res;
            }
        private:
            Parser<T> parser;
            T ban_value;
        };

        template<typename T, typename BL, typename BR>
        struct IBrParser : IParser<T> {
            explicit IBrParser(Parser<T> elem_parser_,
                               Parser<BL> left_parser_,
                               Parser<BR> right_parser_)
                    : elem_parser(elem_parser_), left_parser(left_parser_), right_parser(right_parser_) {}

            Result<T> parse(std::string_view str) override {
                auto left_result = left_parser.parse(str);
                if (!left_result) {
                    return nullres<T>(left_result.get_message());
                }
                str = left_result.rest();
                auto elem_result = elem_parser.parse(str);
                if (!elem_result) {
                    return nullres<T>(elem_result.get_message());
                }
                T result = elem_result.value();
                str = elem_result.rest();
                auto right_result = right_parser.parse(str);
                if (!right_result) {
                    return nullres<T>(right_result.get_message());
                }
                return Result<T>{result, right_result.rest()};
            }
        private:
            Parser<T> elem_parser;
            Parser<BL> left_parser;
            Parser<BR> right_parser;
        };

        // wrap pair<vector, vector> for SeqSaver
        template<typename T, typename U>
        struct SeqWithSeps {
            SeqWithSeps() = default;
            SeqWithSeps(std::vector<T> elems_, std::vector<U> seps_)
                : ielems(std::move(elems_)), iseps(std::move(seps_)) {}

            std::vector<T> elems() { return ielems; }
            std::vector<U> seps() { return iseps; }
        private:
            std::vector<T> ielems;
            std::vector<U> iseps;
        };

        // like ISeqParser but save separators too
        template<typename T, typename U>
        struct ISeqSaverParser : IParser<SeqWithSeps<T, U>> {
            explicit ISeqSaverParser(Parser<T> elem_parser_, Parser<U> sep_parser_)
                    : elem_parser(elem_parser_), sep_parser(sep_parser_) {}

            Result<SeqWithSeps<T, U>> parse(std::string_view str) override {
                auto head = elem_parser.parse(str);
                if (!head) {
                    return nullres<SeqWithSeps<T, U>>(head.get_message());
                }
                std::vector<T> results = {head.value()};
                std::vector<U> seps;
                str = head.rest();
                while (true) {
                    auto sep_result = sep_parser.parse(str);
                    if (!sep_result) {
                        break;
                    }
                    auto elem_result = elem_parser.parse(sep_result.rest());
                    if (!elem_result) {
                        break;
                    }
                    str = elem_result.rest();
                    results.push_back(elem_result.value());
                    seps.push_back(sep_result.value());
                }
                while (seps.size() + 1 > results.size()) {
                    seps.pop_back();
                }
                return Result<SeqWithSeps<T, U>>{SeqWithSeps(results, seps), str};
            }
        private:
            Parser<T> elem_parser;
            Parser<U> sep_parser;
        };

        template<typename T, typename U>
        struct IFoldParser : IParser<T> {
            explicit IFoldParser(Parser<SeqWithSeps<T, U>> parser_, std::vector<std::pair<U, std::function<T(T, T)>>> operators_)
            : parser(parser_), operators(operators_) {}

            Result<T> parse(std::string_view str) override {
                auto result = parser.parse(str);
                if (!result) {
                    return nullres<T>(result.get_message());
                }
                auto elements = result.value().elems();
                auto seps = result.value().seps();
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
            Parser<SeqWithSeps<T, U>> parser;
            std::vector<std::pair<U, std::function<T(T, T)>>> operators;
        };

        template<typename T>
        struct IIdParser : IParser<T> {
            explicit IIdParser(T val_) : val(val_) {}

            Result<T> parse(std::string_view str) override {
                return Result<T>(val, str);
            }
        private:
            T val;
        };

        template<typename T, typename R, typename Func>
        struct IFMapParser : IParser<R> {
            explicit IFMapParser(Parser<T> parser_, Func f_)
                    : parser(parser_), f(f_) {}

            Result<R> parse(std::string_view str) override {
                auto result = parser.parse(str);
                if (!result) {
                    return nullres<R>(result.get_message());
                }
                return Result<R>{f(result.value()), result.rest()};
            }
        private:
            Parser<T> parser;
            Func f;
        };

        template<typename T>
        struct ILazyParser : IParser<T> {
            explicit ILazyParser(std::function<Parser<T>()> get_parser_)
                    : get_parser(std::move(get_parser_)) {}

            Result<T> parse(std::string_view str) override {
                return get_parser().parse(str);
            }
        private:
            std::function<Parser<T>()> get_parser;
        };

        template<typename T>
        struct IMaybeParser : IParser<T> {
            explicit IMaybeParser(Parser<T> parser_, T default_value_)
                    : parser(parser_), default_value(default_value_) {}

            Result<T> parse(std::string_view str) override {
                auto result = parser.parse(str);
                if (!result) {
                    return Result<T>{default_value, str};
                }
                return result;
            }
        private:
            Parser<T> parser;
            T default_value;
        };

    } // namespace Internal

} // namespace Parsec

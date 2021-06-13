#pragma once

#include <optional>
#include <string>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>
#include <functional>

#include "ParsecInternal.hpp"

namespace Parsec {

    template<typename T>
    struct Parser {
        explicit Parser(std::shared_ptr<Internal::IParser<T>> ptr) {
            parser = ptr;
        }

        Internal::Result<T> parse(std::string_view s) {
            return parser->parse(s);
        }

    private:
        std::shared_ptr<Internal::IParser<T>> parser;
    };

    template<typename T, typename R, typename... Args>
    Parser<T> make_parser(Args&&... args) {
        return Parser<T>(std::make_shared<R>(std::forward<Args>(args)...));
    }

    template<typename T>
    Parser<T> operator|(Parser<T> alt1, Parser<T> alt2) {
        return make_parser<T, Internal::IAlternativeParser<T>>(alt1, alt2);
    }

    template<typename U, typename T>
    Parser<T> operator>>(Parser<U> skip_parser, Parser<T> parser) {
        return make_parser<T, Internal::ISkipParser<U, T>>(skip_parser, parser);
    }

    Parser<char> char_parser(char c) {
        return make_parser<char, Internal::ICharParser>(c);
    }

    Parser<char> chars_alt_parser(std::vector<char> chars) {
        return make_parser<char, Internal::ICharsParser>(std::move(chars));
    }

    Parser<std::string_view> prefix_parser(std::string_view str) {
        return make_parser<std::string_view, Internal::IPrefixParser>(str);
    }

    template<typename T>
    Parser<T> id_parser(T id_value) {
        return make_parser<T, Internal::IIdParser<T>>(std::move(id_value));
    }

    template<typename T, typename U, typename R, typename Func>
    Parser<R> merge_parser(Parser<T> p1, Parser<U> p2, Func f) {
        return make_parser<R, Internal::IMergeParser<T, U, R, Func>>(std::move(p1), std::move(p2), std::move(f));
    }

    template<typename T>
    Parser<T> if_equal_not_parsed(Parser<T> parser, T ban_value) {
        return make_parser<T, Internal::IBanParser<T>>(std::move(parser), std::move(ban_value));
    }

    // if string is empty return default_value
    template<typename T>
    Parser<T> empty_parser(T default_value) {
        return make_parser<T, Internal::IEmptyParser<T>>(std::move(default_value));
    }

    // if string is empty return nullres even if parse is success
    template<typename T>
    Parser<T> not_empty_str(Parser<T> parser) {
        return make_parser<T, Internal::INotEmptyParser<T>>(std::move(parser));
    }

    template<typename T>
    Parser<std::vector<T>> many(Parser<T> parser) {
        return make_parser<std::vector<T>, Internal::IManyParser<T>>(std::move(parser));
    }

    Parser<char> space() {
        //TODO: maybe more special symbols
        return chars_alt_parser({' ', '\t'});
    }

    Parser<char> spaces() {
        return make_parser<char, Internal::IManyIgnoreParser<char>>(space());
    }

    Parser<char> alpha() {
        std::vector<char> alpha;
        for (char c = 'a'; c <= 'z'; ++c) {
            alpha.push_back(c);
        }
        return chars_alt_parser(alpha);
    }

    Parser<char> maybe_num() {
        std::vector<char> digits;
        for (char c = '0'; c <= '9'; ++c) {
            digits.push_back(c);
        }
        return chars_alt_parser(digits);
    }

    Parser<char> alpha_num() {
        return alpha() | maybe_num();
    }

    template<typename T, typename Func>
    Parser<T> map_parser(Parser<T> parser, Func f) {
        return make_parser<T, Internal::IFMapParser<T, T, Func>>(std::move(parser), std::move(f));
    }

    template<typename T, typename R, typename Func>
    Parser<R> fmap_parser(Parser<T> parser, Func f) {
        return make_parser<R, Internal::IFMapParser<T, R, Func>>(std::move(parser), std::move(f));
    }

    template<typename T>
    Parser<T> maybe_parser(Parser<T> parser, T default_value) {
        return make_parser<T, Internal::IMaybeParser<T>>(std::move(parser), std::move(default_value));
    }

    template<typename T, typename U>
    Parser<std::vector<T>> seq(Parser<T> elem_parser, Parser<U> sep_parser) {
        return make_parser<std::vector<T>, Internal::ISeqParser<T, U>>
                (std::move(elem_parser), std::move(sep_parser));
    }

    template<typename T, typename U>
    Parser<Internal::SeqWithSeps<T, U>> seq_save(Parser<T> elem_parser, Parser<U> sep_parser) {
        return make_parser<Internal::SeqWithSeps<T, U>, Internal::ISeqSaverParser<T, U>>(std::move(elem_parser), std::move(sep_parser));
    }

    template<typename T, typename BL, typename BR>
    Parser<T> brackets_parser(Parser<BL> left_parser, Parser<T> elem_parser, Parser<BR> right_parser) {
        return make_parser<T, Internal::IBrParser<T, BL, BR>>(std::move(elem_parser), std::move(left_parser), std::move(right_parser));
    }

    template<typename T, typename U>
    Parser<T> fold(Parser<Internal::SeqWithSeps<T, U>> vec_parser, std::vector<std::pair<U, std::function<T(T, T)>>> operators) {
        return make_parser<T, Internal::IFoldParser<T, U>>(std::move(vec_parser), std::move(operators));
    }

    template<typename T>
    Parser<T> lazy_parser(std::function<Parser<T>()> get_parser) {
        return make_parser<T, Internal::ILazyParser<T>>(std::move(get_parser));
    }

} // namespace Parser
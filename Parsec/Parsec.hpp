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
        explicit Parser(std::shared_ptr<Util::VParser<T>> ptr) {
            parser = ptr;
        }

        Util::Result<T> eval(std::string s) {
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

    Parser<char> maybe_num() {
        std::vector<char> digits;
        for (char c = '0'; c <= '9'; ++c) {
            digits.push_back(c);
        }
        return charsParser(digits);
    }

    Parser<char> alphaNum() {
        return alpha() | maybe_num();
    }

    template<typename T, typename Func>
    Parser<T> mapParser(Parser<T> parser, Func f) {
        return Parser<T>(std::make_shared<Util::VFMapParser<T, T, Func>>(std::move(parser), std::move(f)));
    }

    template<typename T, typename R, typename Func>
    Parser<R> fmapParser(Parser<T> parser, Func f) {
        return Parser<R>(std::make_shared<Util::VFMapParser<T, R, Func>>(std::move(parser), std::move(f)));
    }

    template<typename T>
    Parser<T> maybe_parser(Parser<T> parser, T default_value) {
        return Parser<T>(std::make_shared<Util::VMaybeParser<T>>(std::move(parser), std::move(default_value)));
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
    Parser<T> brackets_parser(Parser<BL> left_parser, Parser<T> elem_parser, Parser<BR> right_parser) {
        return Parser<T>(std::make_shared<Util::VBrParser<T, BL, BR>>(std::move(elem_parser), std::move(left_parser), std::move(right_parser)));
    }

    template<typename T, typename U>
    Parser<T> fold(Parser<std::pair<std::vector<T>, std::vector<U>>> vec_parser, std::vector<std::pair<U, std::function<T(T, T)>>> operators) {
        return Parser<T>(std::make_shared<Util::VFoldParser<T, U>>(std::move(vec_parser), std::move(operators)));
    }

    template<typename T>
    Parser<T> lazyParser(std::function<Parser<T>()> get_parser) {
        return Parser<T>(std::make_shared<Util::VLazyParser<T>>(std::move(get_parser)));
    }

} // namespace Parser
#pragma once

#include "parse.hpp"

#ifndef __FLEX_LEXER_H
#include <FlexLexer.h>
#endif

#undef YY_DECL
#define YY_DECL \
    psql_parse::parser::symbol_type psql_parse::scanner::lex()

#define yyterminate() return psql_parse::parser::make_END(loc);

namespace psql_parse {

    class driver;

    class scanner : public yyFlexLexer {
        psql_parse::location loc;

    public:
        explicit scanner(std::istream *in = nullptr, std::ostream *out = nullptr);

        virtual psql_parse::parser::symbol_type lex();

    };

}
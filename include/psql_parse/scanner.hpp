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

		/* STATE while lexing string literals */
		std::string string_buffer;
		std::string ident_buffer;
		StringLiteralType string_type;

		void start_string(StringLiteralType type);
		void start_ident();

    public:
        explicit scanner(std::istream *in = nullptr, std::ostream *out = nullptr);

        virtual psql_parse::parser::symbol_type lex();

        void reset();
    };

}
%skeleton "lalr1.cc"
%require "3.2"

%code top {

// ----------
// PSQL PARSE
// ----------

}

%defines
%define api.parser.class {parser}
%define api.namespace    { psql_parse }
%define api.value.type   variant
%define api.token.raw
%define api.token.constructor
%define api.token.prefix {TOK_}
%define parse.assert
%define parse.trace
%define parse.error verbose

%locations

%parse-param { psql_parse::driver& driver }

%code requires {

#include <string>
#include <vector>

namespace psql_parse {
class driver;
}

}

%code {

#include "psql_parse/driver.hpp"

#undef yylex
#define yylex driver.scanner_->lex

}

%token END 0 "end of file"

%token <int>      INTEGER_VALUE    "integer_value"

%type <std::vector<int>> numbers
%%

%start st;

st:
    numbers	 		{ driver.result_ = $1; }

numbers:
    numbers INTEGER_VALUE	{ $1.push_back($2); std::swap($1, $$); }
 |  %empty			{ $$ = std::vector<int>(); }

%%

void psql_parse::parser::error(const location_type& l, const std::string& m)
{
    driver.error(l, m);
}

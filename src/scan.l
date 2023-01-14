
%option noinput
%option nounput
%option interactive
%option c++
%option warn
%option noyywrap
%option debug
%option yyclass="psql_parse::scanner"

%top{

// ----------
// PSQL PARSE
// ----------

}

%{

#include "psql_parse/scanner.hpp"

#define YY_USER_ACTION loc.columns(yyleng);

%}

/***********************************************
 * Seen Postgres Scanner for reasoning behind  *
 * these rules.                                *
 ***********************************************/

space			[ \t\r\f]
decdigit        [0-9]
decinteger		{decdigit}+

%%

%{
    loc.step();
%}
"\n"    {
            loc.lines(yyleng);
        }

{space}	{
			/* ignore */
		}

{decinteger}	{
				    return psql_parse::parser::make_INTEGER_VALUE(std::stoul(yytext), loc);
			    }

.               {
                    throw psql_parse::parser::syntax_error
                        (loc, "invalid character: " + std::string(yytext));
                }

%%
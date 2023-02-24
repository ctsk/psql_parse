
%option noinput
%option nounput
%option interactive
%option c++
%option warn
%option noyywrap
%option debug
%option caseless
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

numeric         ({decinteger}\.{decinteger}?)|(\.{decinteger})

ident_start     [a-zA-Z]
ident_cont      [a-zA-Z0-9]
identifier      {ident_start}{ident_cont}*


/***********************************
 * Quoted Character String Literal *
 ***********************************/
quote           '

%x xquoted
xqdouble        {quote}{quote}
xqinside        [^']+


xcharstart      {quote}
xbitstart       [bB]{quote}
xnationalstart  [nN]{quote}
xhexstart       [xX]{quote}

%%

%{
    loc.step();
%}

"\n"    {
            loc.lines(yyleng); loc.step();
        }

{space}	{
			/* ignore */
		}

{xbitstart}       { BEGIN(xquoted); start_string(StringLiteralType::BIT); }
{xnationalstart}  { BEGIN(xquoted); start_string(StringLiteralType::NATIONAL); }
{xhexstart}       { BEGIN(xquoted); start_string(StringLiteralType::HEX); }
{xcharstart}      { BEGIN(xquoted); start_string(StringLiteralType::CHAR); }

<xquoted>{xqdouble}  { string_buffer.append("'"); }
<xquoted>{quote}     { BEGIN(INITIAL);
                       switch (string_type) {
                       case StringLiteralType::BIT:
                        return psql_parse::parser::make_BIT_VALUE(string_buffer, loc);
                       case StringLiteralType::CHAR:
                        return psql_parse::parser::make_STRING_VALUE(string_buffer, loc);
                       case StringLiteralType::HEX:
                        return psql_parse::parser::make_HEX_VALUE(string_buffer, loc);
                       case StringLiteralType::NATIONAL:
                        return psql_parse::parser::make_NATIONAL_VALUE(string_buffer, loc);
                       }
                      }
<xquoted>{xqinside}  { string_buffer.append(yytext); }


"("             { return psql_parse::parser::make_LP(loc); }
")"             { return psql_parse::parser::make_RP(loc); }
"."             { return psql_parse::parser::make_DOT(loc); }
","             { return psql_parse::parser::make_COMMA(loc); }
";"             { return psql_parse::parser::make_SEMICOLON(loc); }
"+"             { return psql_parse::parser::make_PLUS(loc); }
"-"             { return psql_parse::parser::make_MINUS(loc); }
"/"             { return psql_parse::parser::make_SLASH(loc); }
"*"             { return psql_parse::parser::make_STAR(loc); }

action          { return psql_parse::parser::make_ACTION(loc); }
all             { return psql_parse::parser::make_ALL(loc); }
and             { return psql_parse::parser::make_AND(loc); }
as              { return psql_parse::parser::make_AS(loc); }
bit             { return psql_parse::parser::make_BIT(loc); }
cascade         { return psql_parse::parser::make_CASCADE(loc); }
character       { return psql_parse::parser::make_CHARACTER(loc); }
char            { return psql_parse::parser::make_CHARACTER(loc); }
collate         { return psql_parse::parser::make_COLLATE(loc); }
commit          { return psql_parse::parser::make_COMMIT(loc); }
constraint      { return psql_parse::parser::make_CONSTRAINT(loc); }
create          { return psql_parse::parser::make_CREATE(loc); }
cross           { return psql_parse::parser::make_CROSS(loc); }
current_user    { return psql_parse::parser::make_CURRENT_USER(loc); }
date            { return psql_parse::parser::make_DATE(loc); }
decimal         { return psql_parse::parser::make_DECIMAL(loc); }
dec             { return psql_parse::parser::make_DECIMAL(loc); }
default         { return psql_parse::parser::make_DEFAULT(loc); }
delete          { return psql_parse::parser::make_DELETE(loc); }
distinct        { return psql_parse::parser::make_DISTINCT(loc); }
double          { return psql_parse::parser::make_DOUBLE(loc); }
float           { return psql_parse::parser::make_FLOAT(loc); }
foreign         { return psql_parse::parser::make_FOREIGN(loc); }
from            { return psql_parse::parser::make_FROM(loc); }
full            { return psql_parse::parser::make_FULL(loc); }
global          { return psql_parse::parser::make_GLOBAL(loc); }
inner           { return psql_parse::parser::make_INNER(loc); }
integer         { return psql_parse::parser::make_INTEGER(loc); }
int             { return psql_parse::parser::make_INTEGER(loc); }
join            { return psql_parse::parser::make_JOIN(loc); }
key             { return psql_parse::parser::make_KEY(loc); }
left            { return psql_parse::parser::make_LEFT(loc); }
local           { return psql_parse::parser::make_LOCAL(loc); }
match           { return psql_parse::parser::make_MATCH(loc); }
national        { return psql_parse::parser::make_NATIONAL(loc); }
natural         { return psql_parse::parser::make_NATURAL(loc); }
nchar           { return psql_parse::parser::make_NCHAR(loc); }
not             { return psql_parse::parser::make_NOT(loc); }
no              { return psql_parse::parser::make_NO(loc); }
null            { return psql_parse::parser::make_NULL(loc); }
numeric         { return psql_parse::parser::make_NUMERIC(loc); }
on              { return psql_parse::parser::make_ON(loc); }
or              { return psql_parse::parser::make_ON(loc); }
outer           { return psql_parse::parser::make_OUTER(loc); }
partial         { return psql_parse::parser::make_PARTIAL(loc); }
precision       { return psql_parse::parser::make_PRECISION(loc); }
preserve        { return psql_parse::parser::make_PRESERVE(loc); }
primary         { return psql_parse::parser::make_PRIMARY(loc); }
real            { return psql_parse::parser::make_REAL(loc); }
references      { return psql_parse::parser::make_REFERENCES(loc); }
right           { return psql_parse::parser::make_RIGHT(loc); }
rows            { return psql_parse::parser::make_ROWS(loc); }
select          { return psql_parse::parser::make_SELECT(loc); }
session_user    { return psql_parse::parser::make_SESSION_USER(loc); }
set             { return psql_parse::parser::make_SET(loc); }
system_user     { return psql_parse::parser::make_SYSTEM_USER(loc); }
table           { return psql_parse::parser::make_TABLE(loc); }
temporary       { return psql_parse::parser::make_TEMPORARY(loc); }
timestamp       { return psql_parse::parser::make_TIMESTAMP(loc); }
time            { return psql_parse::parser::make_TIME(loc); }
unique          { return psql_parse::parser::make_UNIQUE(loc); }
update          { return psql_parse::parser::make_UPDATE(loc); }
user            { return psql_parse::parser::make_USER(loc); }
using           { return psql_parse::parser::make_USING(loc); }
varchar         { return psql_parse::parser::make_VARCHAR(loc); }
varying         { return psql_parse::parser::make_VARYING(loc); }
with            { return psql_parse::parser::make_WITH(loc); }
zone            { return psql_parse::parser::make_ZONE(loc); }


{identifier}    {
                    return psql_parse::parser::make_IDENTIFIER(yytext, loc);
                }

{decinteger}	{
				    return psql_parse::parser::make_INTEGER_VALUE(std::stoul(yytext), loc);
			    }

{numeric}       {
                    return psql_parse::parser::make_FLOAT_VALUE(std::stod(yytext), loc);
                }


.               {
                    throw psql_parse::parser::syntax_error
                        (loc, "invalid character: " + std::string(yytext));
                }

%%
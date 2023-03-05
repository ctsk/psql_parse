
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

%x xident
dquote          \"
xidentstart     {dquote}
xidentstop      {dquote}
xidentinside    [^"]+

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

{xidentstart}           { BEGIN(xident); start_ident(); }
<xident>{xidentinside}  { ident_buffer.append(yytext); }
<xident>{xidentstop}    {
                            BEGIN(INITIAL);
                            return psql_parse::parser::make_IDENTIFIER(ident_buffer, loc);
                        }

"("             { return psql_parse::parser::make_LP(loc); }
")"             { return psql_parse::parser::make_RP(loc); }
"["             { return psql_parse::parser::make_LB(loc); }
"]"             { return psql_parse::parser::make_RB(loc); }
"."             { return psql_parse::parser::make_DOT(loc); }
","             { return psql_parse::parser::make_COMMA(loc); }
";"             { return psql_parse::parser::make_SEMICOLON(loc); }
"+"             { return psql_parse::parser::make_PLUS(loc); }
"-"             { return psql_parse::parser::make_MINUS(loc); }
"/"             { return psql_parse::parser::make_SLASH(loc); }
"*"             { return psql_parse::parser::make_STAR(loc); }
"="             { return psql_parse::parser::make_EQUAL(loc); }
"<>"            { return psql_parse::parser::make_NOT_EQUAL(loc); }
"<="            { return psql_parse::parser::make_LESS_EQUAL(loc); }
"<"             { return psql_parse::parser::make_LESS(loc); }
">="            { return psql_parse::parser::make_GREATER_EQUAL(loc); }
">"             { return psql_parse::parser::make_GREATER(loc); }
"||"            { return psql_parse::parser::make_CONCAT(loc); }

action          { return psql_parse::parser::make_ACTION(loc); }
all             { return psql_parse::parser::make_ALL(loc); }
and             { return psql_parse::parser::make_AND(loc); }
array           { return psql_parse::parser::make_ARRAY(loc); }
asymmetric      { return psql_parse::parser::make_ASYMMETRIC(loc); }
asc             { return psql_parse::parser::make_ASC(loc); }
as              { return psql_parse::parser::make_AS(loc); }
between         { return psql_parse::parser::make_BETWEEN(loc); }
bigint          { return psql_parse::parser::make_BIGINT(loc); }
binary          { return psql_parse::parser::make_BINARY(loc); }
bit             { return psql_parse::parser::make_BIT(loc); }
blob            { return psql_parse::parser::make_BLOB(loc); }
boolean         { return psql_parse::parser::make_BOOLEAN(loc); }
by              { return psql_parse::parser::make_BY(loc); }
cascade         { return psql_parse::parser::make_CASCADE(loc); }
characters      { return psql_parse::parser::make_CHARACTERS(loc); }
character       { return psql_parse::parser::make_CHARACTER(loc); }
char            { return psql_parse::parser::make_CHARACTER(loc); }
clob            { return psql_parse::parser::make_CLOB(loc); }
collate         { return psql_parse::parser::make_COLLATE(loc); }
commit          { return psql_parse::parser::make_COMMIT(loc); }
constraint      { return psql_parse::parser::make_CONSTRAINT(loc); }
create          { return psql_parse::parser::make_CREATE(loc); }
cross           { return psql_parse::parser::make_CROSS(loc); }
cube            { return psql_parse::parser::make_CUBE(loc); }
current_user    { return psql_parse::parser::make_CURRENT_USER(loc); }
current         { return psql_parse::parser::make_CURRENT(loc); }
date            { return psql_parse::parser::make_DATE(loc); }
decimal         { return psql_parse::parser::make_DECIMAL(loc); }
dec             { return psql_parse::parser::make_DECIMAL(loc); }
default         { return psql_parse::parser::make_DEFAULT(loc); }
delete          { return psql_parse::parser::make_DELETE(loc); }
desc            { return psql_parse::parser::make_DESC(loc); }
distinct        { return psql_parse::parser::make_DISTINCT(loc); }
double          { return psql_parse::parser::make_DOUBLE(loc); }
except          { return psql_parse::parser::make_EXCEPT(loc); }
exclude         { return psql_parse::parser::make_EXCLUDE(loc); }
false           { return psql_parse::parser::make_FALSE(loc); }
fetch           { return psql_parse::parser::make_FETCH(loc); }
first           { return psql_parse::parser::make_FIRST(loc); }
float           { return psql_parse::parser::make_FLOAT(loc); }
following       { return psql_parse::parser::make_FOLLOWING(loc); }
foreign         { return psql_parse::parser::make_FOREIGN(loc); }
from            { return psql_parse::parser::make_FROM(loc); }
full            { return psql_parse::parser::make_FULL(loc); }
global          { return psql_parse::parser::make_GLOBAL(loc); }
grouping        { return psql_parse::parser::make_GROUPING(loc); }
groups          { return psql_parse::parser::make_GROUPS(loc); }
group           { return psql_parse::parser::make_GROUP(loc); }
having          { return psql_parse::parser::make_HAVING(loc); }
inner           { return psql_parse::parser::make_INNER(loc); }
integer         { return psql_parse::parser::make_INTEGER(loc); }
intersect       { return psql_parse::parser::make_INTERSECT(loc); }
int             { return psql_parse::parser::make_INTEGER(loc); }
in              { return psql_parse::parser::make_IN(loc); }
join            { return psql_parse::parser::make_JOIN(loc); }
key             { return psql_parse::parser::make_KEY(loc); }
large           { return psql_parse::parser::make_LARGE(loc); }
last            { return psql_parse::parser::make_LAST(loc); }
left            { return psql_parse::parser::make_LEFT(loc); }
local           { return psql_parse::parser::make_LOCAL(loc); }
match           { return psql_parse::parser::make_MATCH(loc); }
multiset        { return psql_parse::parser::make_MULTISET(loc); }
national        { return psql_parse::parser::make_NATIONAL(loc); }
natural         { return psql_parse::parser::make_NATURAL(loc); }
nchar           { return psql_parse::parser::make_NCHAR(loc); }
nclob           { return psql_parse::parser::make_NCLOB(loc); }
next            { return psql_parse::parser::make_NEXT(loc); }
not             { return psql_parse::parser::make_NOT(loc); }
no              { return psql_parse::parser::make_NO(loc); }
nulls           { return psql_parse::parser::make_NULLS(loc); }
null            { return psql_parse::parser::make_NULL(loc); }
numeric         { return psql_parse::parser::make_NUMERIC(loc); }
object          { return psql_parse::parser::make_OBJECT(loc); }
octets          { return psql_parse::parser::make_OCTETS(loc); }
offset          { return psql_parse::parser::make_OFFSET(loc); }
only            { return psql_parse::parser::make_ONLY(loc); }
on              { return psql_parse::parser::make_ON(loc); }
order           { return psql_parse::parser::make_ORDER(loc); }
or              { return psql_parse::parser::make_ON(loc); }
others          { return psql_parse::parser::make_OTHERS(loc); }
outer           { return psql_parse::parser::make_OUTER(loc); }
partial         { return psql_parse::parser::make_PARTIAL(loc); }
partition       { return psql_parse::parser::make_PARTITION(loc); }
percent         { return psql_parse::parser::make_PERCENT(loc); }
preceding       { return psql_parse::parser::make_PRECEDING(loc); }
precision       { return psql_parse::parser::make_PRECISION(loc); }
preserve        { return psql_parse::parser::make_PRESERVE(loc); }
primary         { return psql_parse::parser::make_PRIMARY(loc); }
range           { return psql_parse::parser::make_RANGE(loc); }
real            { return psql_parse::parser::make_REAL(loc); }
references      { return psql_parse::parser::make_REFERENCES(loc); }
ref             { return psql_parse::parser::make_REF(loc); }
right           { return psql_parse::parser::make_RIGHT(loc); }
rollup          { return psql_parse::parser::make_ROLLUP(loc); }
rows            { return psql_parse::parser::make_ROWS(loc); }
row             { return psql_parse::parser::make_ROW(loc); }
scope           { return psql_parse::parser::make_SCOPE(loc); }
select          { return psql_parse::parser::make_SELECT(loc); }
session_user    { return psql_parse::parser::make_SESSION_USER(loc); }
sets            { return psql_parse::parser::make_SETS(loc); }
set             { return psql_parse::parser::make_SET(loc); }
symmetric       { return psql_parse::parser::make_SYMMETRIC(loc); }
system_user     { return psql_parse::parser::make_SYSTEM_USER(loc); }
table           { return psql_parse::parser::make_TABLE(loc); }
temporary       { return psql_parse::parser::make_TEMPORARY(loc); }
ties            { return psql_parse::parser::make_TIES(loc); }
timestamp       { return psql_parse::parser::make_TIMESTAMP(loc); }
time            { return psql_parse::parser::make_TIME(loc); }
true            { return psql_parse::parser::make_TRUE(loc); }
unbounded       { return psql_parse::parser::make_UNBOUNDED(loc); }
union           { return psql_parse::parser::make_UNION(loc); }
unique          { return psql_parse::parser::make_UNIQUE(loc); }
unknown         { return psql_parse::parser::make_UNKNOWN(loc); }
update          { return psql_parse::parser::make_UPDATE(loc); }
user            { return psql_parse::parser::make_USER(loc); }
using           { return psql_parse::parser::make_USING(loc); }
values          { return psql_parse::parser::make_VALUES(loc); }
varbinary       { return psql_parse::parser::make_VARBINARY(loc); }
varchar         { return psql_parse::parser::make_VARCHAR(loc); }
varying         { return psql_parse::parser::make_VARYING(loc); }
where           { return psql_parse::parser::make_WHERE(loc); }
window          { return psql_parse::parser::make_WINDOW(loc); }
without         { return psql_parse::parser::make_WITHOUT(loc); }
with            { return psql_parse::parser::make_WITH(loc); }
zone            { return psql_parse::parser::make_ZONE(loc); }


{identifier}    {
                    std::string ident(yytext);
                    std::transform(ident.begin(), ident.end(), ident.begin(),
                        [](unsigned char c){ return std::tolower(c); });
                    return psql_parse::parser::make_IDENTIFIER(std::move(ident), loc);
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

#include "psql_parse/scanner.hpp"

namespace psql_parse {
    scanner::scanner(std::istream *in, std::ostream *out)
            : yyFlexLexer(in, out) { }
}
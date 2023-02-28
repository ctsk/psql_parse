#include "psql_parse/scanner.hpp"

namespace psql_parse {
    scanner::scanner(std::istream *in, std::ostream *out)
            : yyFlexLexer(in, out), string_buffer() { }

	void scanner::start_string(StringLiteralType type) {
		string_type = type;
		string_buffer.clear();
	}

	void scanner::start_ident() {
		ident_buffer.clear();
	}
}
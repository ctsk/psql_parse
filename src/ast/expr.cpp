#include "psql_parse/ast/expr.h"


psql_parse::Expression::Expression(psql_parse::location loc)
: Node { loc } { }

psql_parse::NumberLiteral::NumberLiteral(psql_parse::location loc)
: Expression(loc) { }

psql_parse::IntegerLiteral::IntegerLiteral(location loc, int64_t value)
: NumberLiteral(loc), value(value) { }

void psql_parse::IntegerLiteral::negate() {
	this->value *= -1;
}

psql_parse::FloatLiteral::FloatLiteral(location loc, double value)
: NumberLiteral(loc), value(value) { }

void psql_parse::FloatLiteral::negate() {
	this->value *= -1;
}

psql_parse::StringLiteral::StringLiteral(location loc, std::string&& value, StringLiteralType type)
: Expression(loc), value(value), type(type) { }


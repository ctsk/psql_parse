#include "psql_parse/ast/expr.hpp"

namespace psql_parse {
	Expression::Expression(location loc)
	: Node{loc} {}

	bool operator==(const Expression &, const Expression &) {
		return false;
	}

	NumberLiteral::NumberLiteral(location loc)
	: Expression(loc) {}

	IntegerLiteral::IntegerLiteral(location loc, int64_t value)
	: NumberLiteral(loc), value(value) {}

	void IntegerLiteral::negate() {
		this->value *= -1;
	}

	FloatLiteral::FloatLiteral(location loc, double value)
	: NumberLiteral(loc), value(value) {}

	void FloatLiteral::negate() {
		this->value *= -1;
	}

	StringLiteral::StringLiteral(location loc, std::string &&value, StringLiteralType type)
	: Expression(loc), value(value), type(type) {}
}

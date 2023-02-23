#include "psql_parse/ast/expr.hpp"

namespace psql_parse {
	Expression::Expression(location loc)
	: Node{loc} {}

	bool operator==(const Expression &, const Expression &) {
		return false;
	}

	ValueExpression::ValueExpression(location loc)
	: Expression(loc) {}

	SetExpression::SetExpression(location loc)
	: Expression(loc) {}

	NumberLiteral::NumberLiteral(location loc)
	: ValueExpression(loc) {}

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
	: ValueExpression(loc), value(value), type(type) {}

}

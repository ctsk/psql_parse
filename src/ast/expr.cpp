#include "psql_parse/ast/expr.hpp"

namespace psql_parse {
	Expression::Expression(location loc)
	: Node{loc} {}

	bool operator==(const Expression &, const Expression &) {
		return false;
	}

	ValExpr::ValExpr(location loc)
	: Expression(loc) {}

	SetExpression::SetExpression(location loc)
	: Expression(loc) {}

	IntegerLiteral::IntegerLiteral(location loc, int64_t value)
	: ValExpr(loc), value(value) {}

	FloatLiteral::FloatLiteral(location loc, double value)
	: ValExpr(loc), value(value) {}

	StringLiteral::StringLiteral(location loc, std::string &&value, StringLiteralType type)
	: ValExpr(loc), value(value), type(type) {}

	UnaryOp::UnaryOp(location loc, UnaryOp::Op op, std::unique_ptr<ValExpr> inner)
	: ValExpr(loc), op(op), inner(std::move(inner)) {}

	BinaryOp::BinaryOp(location loc, std::unique_ptr<ValExpr> left, BinaryOp::Op op, std::unique_ptr<ValExpr> right)
	: ValExpr(loc), op(op), left(std::move(left)), right(std::move(right)) {}

	IsExpr::IsExpr(location loc, std::unique_ptr<ValExpr> expr, BoolLiteral lit)
	: ValExpr(loc), expr(std::move(expr)), lit(lit) {}

}

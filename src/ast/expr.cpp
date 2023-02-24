#include "psql_parse/ast/expr.hpp"

namespace psql_parse {
	Expression::Expression(location loc)
	: Node{loc} {}

	bool operator==(const Expression &, const Expression &) {
		return false;
	}

	ValExpr::ValExpr(location loc)
	: Expression(loc) {}

	RelExpr::RelExpr(location loc)
	: Expression(loc) {}

	IntegerLiteral::IntegerLiteral(location loc, int64_t value)
	: ValExpr(loc), value(value) {}

	FloatLiteral::FloatLiteral(location loc, double value)
	: ValExpr(loc), value(value) {}

	StringLiteral::StringLiteral(location loc, std::string &&value, StringLiteralType type)
	: ValExpr(loc), value(value), type(type) {}

	UnaryOp::UnaryOp(location loc, UnaryOp::Op op, ValExpr *inner)
	: ValExpr(loc), op(op), inner(inner) {}

	BinaryOp::BinaryOp(location loc, ValExpr *left, BinaryOp::Op op, ValExpr *right)
	: ValExpr(loc), op(op), left(left), right(right) {}

	IsExpr::IsExpr(location loc, std::unique_ptr<ValExpr> expr, BoolLiteral lit)
	: ValExpr(loc), expr(std::move(expr)), lit(lit) {}

	AliasExpr::AliasExpr(location loc, std::string name, ValExpr *expr)
	: ValExpr(loc), name(std::move(name)), expr(expr) {}

	JoinExpr::JoinExpr(location loc, JoinExpr::Kind kind)
	: RelExpr(loc), kind(kind), natural(false) {}

	void JoinExpr::setNatural() {
		natural = true;
	}

	TableName::TableName(location loc, QualifiedName name)
	: RelExpr(loc), name(std::move(name)) {}
}

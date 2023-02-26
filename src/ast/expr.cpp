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

	AliasExpr::AliasExpr(location loc, std::string name, ValExpr *expr)
	: ValExpr(loc), name(std::move(name)), expr(expr) {}

	JoinExpr::JoinExpr(location loc, RelExpr *first, JoinExpr::Kind kind, RelExpr *second)
	: RelExpr(loc), kind(kind), natural(false), first(first), second(second) {}

	void JoinExpr::setNatural() {
		natural = true;
	}

	void JoinExpr::setQualifier(ValExpr *expr) {
		qualifier = std::unique_ptr<ValExpr>(expr);
	}

	TableName::TableName(location loc, QualifiedName name)
	: RelExpr(loc), name(std::move(name)) {}

	QueryExpr::QueryExpr(location loc)
	: RelExpr(loc) {}

	SetOp::SetOp(location loc, RelExpr *left, SetOp::Op op, RelExpr *right)
	: RelExpr(loc), op(op), left(left), right(right) {}

	RowSubquery::RowSubquery(location loc, RelExpr *expr)
	: ValExpr(loc), subquery(expr) { }

	Var::Var(location loc, std::string name)
	: ValExpr(loc), name(std::move(name)) {}

	IsExpr::IsExpr(location loc, ValExpr *inner, BoolLiteral truth_value)
	: ValExpr(loc), inner(inner), truth_value(truth_value) {}

	BetweenPred::BetweenPred(location loc, ValExpr *val, ValExpr *low, ValExpr *high)
	: ValExpr(loc), val(val), low(low), high(high), symmetric(false) {}
}

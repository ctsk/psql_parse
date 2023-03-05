#include "psql_parse/ast/expr.hpp"

namespace psql_parse {

	IntegerLiteral::IntegerLiteral(uint64_t value)
	: value(value) {}

	FloatLiteral::FloatLiteral(double value)
	: value(value) {}

	StringLiteral::StringLiteral(std::string &&value, StringLiteralType type)
	: value(value), type(type) {}

	UnaryOp::UnaryOp(UnaryOp::Op op, Expression inner)
	: op(op), inner(std::move(inner)) {}

	BinaryOp::BinaryOp(Expression left, BinaryOp::Op op, Expression right)
	: op(op), left(std::move(left)), right(std::move(right)) {}

	AliasExpr::AliasExpr(std::string name, Expression expr)
	: name(std::move(name)), expr(std::move(expr)) {}

	JoinExpr::JoinExpr(RelExpression first, JoinExpr::Kind kind, RelExpression second)
	: kind(kind), natural(false), first(std::move(first)), second(std::move(second)) {}

	void JoinExpr::setNatural() {
		natural = true;
	}

	void JoinExpr::setQualifier(Expression expr) {
		qualifier = std::move(expr);
	}

	TableName::TableName(box<QualifiedName> name)
	: name(std::move(name)) {}

    TableAlias::TableAlias(Name name)
    : name(std::move(name)) {}

	SelectExpr::SelectExpr() = default;

	SetOp::SetOp(RelExpression left, SetOp::Op op, RelExpression right)
	: op(op), left(std::move(left)), right(std::move(right)) {}

	RowSubquery::RowSubquery(RelExpression expr)
	: subquery(std::move(expr)) { }

	Var::Var(std::string name)
	: name(std::move(name)) {}

	IsExpr::IsExpr(Expression inner, BoolLiteral truth_value)
	: inner(std::move(inner)), truth_value(truth_value) {}

	Collate::Collate(Expression var, box<QualifiedName> collation)
	: var(std::move(var)), collation(std::move(collation)) {}

	BetweenPred::BetweenPred(Expression val, Expression low, Expression high)
	: val(std::move(val)), low(std::move(low)), high(std::move(high)), symmetric(false) {}

	ValuesExpr::ValuesExpr(std::vector<Expression> rows)
	: rows(std::move(rows)) {}

	InPred::InPred(Expression val, RelExpression rows)
	: val(std::move(val)), rows(std::move(rows)), symmetric(false) {}

	LikePred::LikePred(Expression val, Expression pattern)
	: val(std::move(val)), pattern(std::move(pattern)), escape(std::nullopt) {}

	SortSpec::SortSpec(Expression expr)
	: expr(std::move(expr)) {}

	SortSpec::SortSpec() = default;

	Query::Query(RelExpression expr)
	: expr(std::move(expr)), order (), offset (std::nullopt), fetch (std::nullopt) {}

	RowExpr::RowExpr() = default;

	RowExpr::RowExpr(std::vector<Expression> exprs)
	: exprs(std::move(exprs)) {}

	GroupingSet::GroupingSet() = default;
	GroupingSets::GroupingSets() = default;
	Rollup::Rollup() = default;
	Cube::Cube() = default;
}

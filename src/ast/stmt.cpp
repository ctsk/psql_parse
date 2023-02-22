#include "psql_parse/ast/stmt.hpp"

namespace psql_parse {

	Statement::Statement(location loc)
	: Node { loc } { }

	ExprStatement::ExprStatement(location loc, std::unique_ptr<Expression> expr)
	: Statement(loc), expr(std::move(expr)) {}

	const Expression* ExprStatement::getExpr() const {
		return expr.get();
	}

}
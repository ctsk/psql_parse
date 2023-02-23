#include "psql_parse/ast/stmt.hpp"

namespace psql_parse {

	Statement::Statement(location loc)
	: Node { loc } { }

	ExprStatement::ExprStatement(location loc, Expression *expr)
	: Statement(loc), expr(expr) {}

	const Expression* ExprStatement::getExpr() const {
		return expr.get();
	}

}
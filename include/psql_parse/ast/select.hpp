#pragma once

#include "stmt.hpp"
#include "expr.hpp"

namespace psql_parse {
	struct SelectStatement: public Statement {
		std::unique_ptr<RelExpr> rel_expr;
		explicit SelectStatement(RelExpr *queryExpr);
	};
}
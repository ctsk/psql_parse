#pragma once

#include "stmt.hpp"
#include "expr.hpp"

namespace psql_parse {
	struct SelectStatement: public Statement {
		std::unique_ptr<QueryExpr> query_expr;
		explicit SelectStatement(QueryExpr *queryExpr);
	};
}
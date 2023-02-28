#pragma once

#include "expr.hpp"

namespace psql_parse {
	struct SelectStatement {
		DEFAULT_EQ(SelectStatement);

		RelExpression rel_expr;

		explicit SelectStatement(RelExpression queryExpr);
	};
}
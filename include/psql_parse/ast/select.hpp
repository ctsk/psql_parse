#pragma once

#include "expr.hpp"

namespace psql_parse {
	struct SelectStatement {
		DEFAULT_EQ(SelectStatement);

		box<Query> rel_expr;

		explicit SelectStatement(box<Query> queryExpr);
	};
}
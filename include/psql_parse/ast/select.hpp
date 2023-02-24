#pragma once

#include "stmt.hpp"
#include "expr.hpp"

namespace psql_parse {
	struct SelectStatement: public Statement {
		std::vector<std::unique_ptr<ValExpr>> target_list;

		SelectStatement(location loc);
	};
}
#pragma once

#include "stmt.hpp"
#include "expr.hpp"

namespace psql_parse {
	enum class SetQuantifier {
		ALL,
		DISTINCT
	};

	struct SelectStatement: public Statement {
		std::vector<std::unique_ptr<ValExpr>> target_list;
		std::optional<SetQuantifier> set_quantifier;

		SelectStatement(location loc);
	};
}
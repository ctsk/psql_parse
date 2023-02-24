#pragma once

#include "stmt.hpp"
#include "expr.hpp"

namespace psql_parse {
	enum class SetQuantifier {
		ALL,
		DISTINCT
	};

	struct SelectStatement: public Statement {
		/*
		 * NOTE: nullptr = ASTERISK
		 */
		std::vector<std::unique_ptr<ValExpr>> target_list;
		std::vector<std::unique_ptr<RelExpr>> from_clause;
		std::optional<SetQuantifier> set_quantifier;

		explicit SelectStatement(location loc);
	};
}
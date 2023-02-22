#pragma once

#include <memory>

#include "expr.hpp"

namespace psql_parse {

	class Statement: public Node {
	protected:
		explicit Statement(location loc);

	public:
		virtual ~Statement() = default;
	};

	class ExprStatement: public Statement {
		std::unique_ptr<Expression> expr;

	public:
		ExprStatement(location loc, std::unique_ptr<Expression> expr);

		[[nodiscard]] const Expression* getExpr() const;
	};
}

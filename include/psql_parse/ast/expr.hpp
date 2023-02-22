#pragma once

#include <cstdint>

#include "common.hpp"

namespace psql_parse {
    struct Expression: public Node {
	protected:
		explicit Expression(location loc);
	public:
		virtual	~Expression() = default;
	};

	struct NumberLiteral: public Expression {
		explicit NumberLiteral(location loc);
		virtual void negate() = 0;
	};

    struct IntegerLiteral: public NumberLiteral {
		std::int64_t value;
		IntegerLiteral(location loc, std::int64_t value);
		void negate() override;
	};

    struct FloatLiteral: public NumberLiteral {
        double value;
        FloatLiteral(location loc, double value);
		void negate() override;
	};

	struct StringLiteral: public Expression {
		std::string value;
		StringLiteralType type;

		StringLiteral(location loc, std::string&& value, StringLiteralType type);
	};

}
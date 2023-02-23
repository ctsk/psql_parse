#pragma once

#include <cstdint>

#include "common.hpp"

namespace psql_parse {

    struct Expression: public Node {
		virtual	~Expression() = default;
		friend bool operator==(const Expression&, const Expression&);

	protected:
		explicit Expression(location loc);
	};


	/// ValueExpression: Expression kinds, whose result is a value;
	struct ValueExpression: public Expression {
	protected:
		explicit ValueExpression(location loc);
	};

	/// SetExpression: Expression kinds, whose result is a Bag of Tuples
	struct SetExpression: public Expression {
	protected:
		explicit SetExpression(location loc);
	};

	struct NumberLiteral: public ValueExpression {
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

	struct StringLiteral: public ValueExpression {
		std::string value;
		StringLiteralType type;
		StringLiteral(location loc, std::string&& value, StringLiteralType type);
	};

	struct UnaryOp: public ValueExpression {
		enum class Op {
			NOT
		};

		box<Expression> inner;
	};

	struct BinaryOp: public ValueExpression {
		enum class Op {
			OR,
			AND
		};

		box<Expression> left;
		box<Expression> right;
	};
}
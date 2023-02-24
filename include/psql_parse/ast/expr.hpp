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


	/// ValExpr: Expression kinds, whose result is a value;
	struct ValExpr: public Expression {
	protected:
		explicit ValExpr(location loc);
	};

	/// <expr> AS <name>
	struct AliasExpr: public ValExpr {
		Name name;
		std::unique_ptr<ValExpr> expr;
		AliasExpr(location loc, std::string name, ValExpr *expr);
	};

	/// SetExpression: Expression kinds, whose result is a Bag of Tuples
	struct SetExpression: public Expression {
	protected:
		explicit SetExpression(location loc);
	};

    struct IntegerLiteral: public ValExpr {
		std::int64_t value;
		IntegerLiteral(location loc, std::int64_t value);
	};

    struct FloatLiteral: public ValExpr {
        double value;
        FloatLiteral(location loc, double value);
	};

	struct StringLiteral: public ValExpr {
		std::string value;
		StringLiteralType type;
		StringLiteral(location loc, std::string&& value, StringLiteralType type);
	};

	enum class BoolLiteral {
		TRUE,
		FALSE,
		UNKNOWN
	};

	struct IsExpr: public ValExpr {
		std::unique_ptr<ValExpr> expr;
		BoolLiteral lit;

		IsExpr(location loc, std::unique_ptr<ValExpr> expr, BoolLiteral lit);
	};

	struct UnaryOp: public ValExpr {
		enum class Op {
			NOT,
			NEG
		};

		Op op;
		std::unique_ptr<ValExpr> inner;

		UnaryOp(location loc, Op op, ValExpr *inner);
	};



	struct BinaryOp: public ValExpr {
		enum class Op {
			OR, AND,
			ADD, SUB, MULT, DIV
		};

		Op op;
		std::unique_ptr<ValExpr> left;
		std::unique_ptr<ValExpr> right;

		BinaryOp(location loc, ValExpr *left, Op op, ValExpr *right);
	};
}
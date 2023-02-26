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

	/// RelExpr: Expression kinds, whose result is a Bag of Tuples
	struct RelExpr: public Expression {
	protected:
		explicit RelExpr(location loc);
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

	struct Var: public ValExpr {
		Name name;

		Var(location loc, std::string);
	};

	struct IsExpr: public ValExpr {
		std::unique_ptr<ValExpr> inner;
		BoolLiteral truth_value;

		IsExpr(location loc, ValExpr *inner, BoolLiteral truth_value);
	};

	struct UnaryOp: public ValExpr {
		enum class Op {
			NOT,
			NEG
		};

		Op op;
		std::unique_ptr<ValExpr> inner;

		UnaryOp(location loc, Op op, ValExpr *inner);

		static UnaryOp* Not(ValExpr *expr) {
			return new UnaryOp(expr->loc, UnaryOp::Op::NOT, expr);
		}
	};

	struct BinaryOp: public ValExpr {
		enum class Op {
			OR, AND,
			ADD, SUB, MULT, DIV,
			LESS, LESS_EQUAL, GREATER, GREATER_EQUAL, EQUAL, NOT_EQUAL
		};

		Op op;
		std::unique_ptr<ValExpr> left;
		std::unique_ptr<ValExpr> right;

		BinaryOp(location loc, ValExpr *left, Op op, ValExpr *right);
	};

	struct TableName: public RelExpr {
		QualifiedName name;
		TableName(location loc, QualifiedName name);
	};

	struct JoinExpr: public RelExpr {
		enum class Kind {
			FULL,
			LEFT,
			RIGHT,
			INNER
		};

		Kind kind;
		bool natural;
		std::unique_ptr<ValExpr> qualifier;
		std::vector<Name> columns;

		std::unique_ptr<RelExpr> first;
		std::unique_ptr<RelExpr> second;

		JoinExpr(location loc, RelExpr *first, Kind kind, RelExpr *second);

		void setNatural();
		void setQualifier(ValExpr *expr);
	};

	enum class SetQuantifier {
		ALL,
		DISTINCT
	};

	struct QueryExpr: public RelExpr {
		/*
 		 * NOTE: nullptr = ASTERISK
 		*/
		const ValExpr* ASTERISK = nullptr;
		std::vector<std::unique_ptr<ValExpr>> target_list;

		std::vector<std::unique_ptr<RelExpr>> from_clause;
		std::optional<SetQuantifier> set_quantifier;

		explicit QueryExpr(location loc);
	};

	struct SetOp: public RelExpr {
		enum class Op {
			UNION,
			INTERSECT,
			EXCEPT
		};

		Op op;
		std::unique_ptr<RelExpr> left;
		std::unique_ptr<RelExpr> right;

		std::optional<SetQuantifier> quantifier;

		SetOp(location loc, RelExpr *left, Op op, RelExpr *right);
	};

	struct RowSubquery: public ValExpr {
		std::unique_ptr<RelExpr> subquery;

		RowSubquery(location loc, RelExpr *expr);
	};

	enum class Builtin { };

	template<Builtin name, typename... Args>
	requires ((std::is_same_v<Args, ValExpr> || std::is_same_v<Args, RelExpr>) || ...)
	struct BuiltinFunc: public ValExpr {
		std::vector<std::unique_ptr<Expression>> args;

		BuiltinFunc(location loc, Args*... exprs)
		: ValExpr(loc), args() {
			(args.emplace_back(exprs), ...);
		}
	};

	struct BetweenPred: public ValExpr {
		std::unique_ptr<ValExpr> val;
		std::unique_ptr<ValExpr> low;
		std::unique_ptr<ValExpr> high;

		bool symmetric;

		BetweenPred(location loc, ValExpr *val, ValExpr *low, ValExpr *high);
	};

	struct InPred: public ValExpr {
		std::unique_ptr<ValExpr> val;
		std::unique_ptr<RelExpr> rows;

		bool symmetric;

		InPred(location loc, ValExpr *val, RelExpr *rows);
	};
}
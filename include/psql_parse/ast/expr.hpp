#pragma once

#include <cstdint>

#include "psql_parse/ast/common.hpp"
#include "psql_parse/ast/nodes.hpp"

namespace psql_parse {

    struct Expression: public Node {
		virtual ExprPtrs asVariant() = 0;
	protected:
		explicit Expression(location loc);
	};

	/// ValExpr: Expression kinds, whose result is a value;
	struct ValExpr: public Expression {
	protected:
		explicit ValExpr(location loc);
	};

	/// RelExpr: Expression kinds, whose result is a Bag of Tuples
	struct RelExpr: public Expression {
	protected:
		explicit RelExpr(location loc);
	};

	/// <expr> AS <name>
	struct AliasExpr: public ValExpr {
		Name name;
		std::unique_ptr<ValExpr> expr;
		AliasExpr(location loc, std::string name, ValExpr *expr);

		ADD_AS_VARIANT()
	};

    struct IntegerLiteral: public ValExpr {
		std::int64_t value;
		IntegerLiteral(location loc, std::int64_t value);
		ADD_AS_VARIANT()
	};

    struct FloatLiteral: public ValExpr {
        double value;
        FloatLiteral(location loc, double value);
		ADD_AS_VARIANT()
	};

	struct StringLiteral: public ValExpr {
		std::string value;
		StringLiteralType type;
		StringLiteral(location loc, std::string&& value, StringLiteralType type);
		ADD_AS_VARIANT()
	};

	enum class BoolLiteral {
		TRUE,
		FALSE,
		UNKNOWN
	};

	struct Var: public ValExpr {
		Name name;

		Var(location loc, std::string);
		ADD_AS_VARIANT()
	};

	struct IsExpr: public ValExpr {
		std::unique_ptr<ValExpr> inner;
		BoolLiteral truth_value;

		IsExpr(location loc, ValExpr *inner, BoolLiteral truth_value);
		ADD_AS_VARIANT()
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

		ADD_AS_VARIANT()
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
		ADD_AS_VARIANT()
	};

	struct TableName: public RelExpr {
		QualifiedName name;
		TableName(location loc, QualifiedName name);
		ADD_AS_VARIANT()
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

		ADD_AS_VARIANT()
	};

	enum class SetQuantifier {
		ALL,
		DISTINCT
	};

	struct SortSpec {
		enum class Order {
			ASC,
			DESC
		};

		enum class NullOrder {
			DEFAULT,
			FIRST,
			LAST,
		};

		location loc;
		std::unique_ptr<ValExpr> expr;
		Order order = Order::ASC;
		NullOrder null_order = NullOrder::DEFAULT;

		SortSpec();
		SortSpec(location loc, ValExpr *expr);
	};

	struct QueryExpr: public RelExpr {
		/*
 		 * NOTE: nullptr = ASTERISK
 		*/
		const ValExpr* ASTERISK = nullptr;
		std::vector<std::unique_ptr<ValExpr>> target_list;
		std::vector<std::unique_ptr<RelExpr>> from_clause;
		std::unique_ptr<ValExpr> where_clause;
		std::optional<SetQuantifier> set_quantifier;

		explicit QueryExpr(location loc);
		ADD_AS_VARIANT()
	};

	struct OrderOp: public RelExpr {
		std::unique_ptr<RelExpr> expr;
		std::vector<SortSpec> fields;

		explicit OrderOp(location loc, RelExpr *expr, std::vector<SortSpec> fields);
		ADD_AS_VARIANT()
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
		ADD_AS_VARIANT()
	};

	struct ValuesExpr: public RelExpr {
		std::vector<std::unique_ptr<ValExpr>> rows;

		explicit ValuesExpr(location loc);
		ValuesExpr(location loc, std::vector<std::unique_ptr<ValExpr>> rows);
		ADD_AS_VARIANT()
	};

	struct RowSubquery: public ValExpr {
		std::unique_ptr<RelExpr> subquery;

		RowSubquery(location loc, RelExpr *expr);
		ADD_AS_VARIANT()
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
		ADD_AS_VARIANT()
	};

	struct InPred: public ValExpr {
		std::unique_ptr<ValExpr> val;
		std::unique_ptr<RelExpr> rows;

		bool symmetric;

		InPred(location loc, ValExpr *val, RelExpr *rows);
		ADD_AS_VARIANT()
	};
}
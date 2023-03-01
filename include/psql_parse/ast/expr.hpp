#pragma once

#include <cstdint>

#include "common.hpp"

namespace psql_parse {

	struct AliasExpr;
	struct IntegerLiteral;
	struct FloatLiteral;
	struct StringLiteral;
	struct UnaryOp;
	struct BinaryOp;
	struct RowSubquery;
	struct Var;
	struct Collate;
	struct IsExpr;
	struct BetweenPred;
	struct InPred;
	struct SortSpec;
	struct RowExpr;

	struct GroupingSet;
	struct GroupingSets;
	struct Rollup;
	struct Cube;

	struct JoinExpr;
	struct TableName;
	struct QueryExpr;
	struct ValuesExpr;
	struct SetOp;
	struct OrderOp;

	using Expression = std::variant<
			box<IntegerLiteral>,
			box<FloatLiteral>,
			box<StringLiteral>,
			box<UnaryOp>,
			box<BinaryOp>,
			box<AliasExpr>,
			box<RowExpr>,
			box<RowSubquery>,
			box<Var>,
			box<Collate>,
			box<IsExpr>,
			box<BetweenPred>,
			box<InPred>,
			box<SortSpec>,
			box<GroupingSet>,
			box<GroupingSets>,
			box<Rollup>,
			box<Cube>>;

	using Grouping = std::variant<
			box<GroupingSet>,
			box<GroupingSets>,
			box<Rollup>,
			box<Cube>>;

	using RelExpression = std::variant<
			box<JoinExpr>,
			box<TableName>,
			box<QueryExpr>,
			box<ValuesExpr>,
			box<SetOp>,
			box<OrderOp>>;
	
	/// <expr> AS <name>
	struct AliasExpr {
		DEFAULT_EQ(AliasExpr);

		Name name;
		Expression expr;

		AliasExpr(std::string name, Expression expr);
	};

    struct IntegerLiteral {
		DEFAULT_EQ(IntegerLiteral);

		std::int64_t value;

		explicit IntegerLiteral(std::int64_t value);
	};

    struct FloatLiteral {
		DEFAULT_EQ(FloatLiteral);

        double value;

        explicit FloatLiteral(double value);
	};

	struct StringLiteral {
		DEFAULT_EQ(StringLiteral);

		std::string value;
		StringLiteralType type;

		StringLiteral(std::string&& value, StringLiteralType type);
	};

	enum class BoolLiteral {
		TRUE,
		FALSE,
		UNKNOWN
	};

	struct Var {
		DEFAULT_EQ(Var);

		Name name;

		explicit Var(std::string);
	};

	struct Collate {
		DEFAULT_EQ(Collate);

		box<Var> var;
		QualifiedName collation;

		Collate(Var* var, QualifiedName collation);
	};

	struct IsExpr {
		DEFAULT_EQ(IsExpr);

		Expression inner;
		BoolLiteral truth_value;

		IsExpr(Expression inner, BoolLiteral truth_value);
	};

	struct UnaryOp {
		DEFAULT_EQ(UnaryOp);

		enum class Op {
			NOT,
			NEG
		};

		Op op;
		Expression inner;

		UnaryOp(Op op, Expression inner);

		static UnaryOp* Not(Expression expr) {
			return new UnaryOp(UnaryOp::Op::NOT, std::move(expr));
		}
	};

	struct BinaryOp {
		DEFAULT_EQ(BinaryOp);

		enum class Op {
			OR, AND,
			ADD, SUB, MULT, DIV,
			LESS, LESS_EQUAL,
			GREATER, GREATER_EQUAL,
			EQUAL, NOT_EQUAL
		};

		Op op;
		Expression left;
		Expression right;

		BinaryOp(Expression left, Op op, Expression right);
	};

	struct TableName {
		DEFAULT_EQ(TableName);

		QualifiedName name;

		explicit TableName(QualifiedName name);
	};

	struct JoinExpr {
		DEFAULT_EQ(JoinExpr);

		enum class Kind {
			FULL,
			LEFT,
			RIGHT,
			INNER
		};

		Kind kind;
		bool natural;
		Expression qualifier;
		std::vector<Name> columns;

		RelExpression first;
		RelExpression second;

		JoinExpr(RelExpression first, Kind kind, RelExpression second);

		void setNatural();
		void setQualifier(Expression expr);
	};

	enum class SetQuantifier {
		ALL,
		DISTINCT
	};

	struct SortSpec {
		DEFAULT_EQ(SortSpec);

		enum class Order {
			ASC,
			DESC
		};

		enum class NullOrder {
			DEFAULT,
			FIRST,
			LAST,
		};

		Expression expr;
		Order order = Order::ASC;
		NullOrder null_order = NullOrder::DEFAULT;

		SortSpec();
		explicit SortSpec(Expression expr);
	};

	struct GroupClause {
		DEFAULT_EQ(GroupClause);

		std::optional<SetQuantifier> group_quantifier;
		std::vector<Grouping> group_clause;
	};

	struct QueryExpr {
		DEFAULT_EQ(QueryExpr);

		/*
 		 * NOTE: nullptr = ASTERISK
 		*/
		std::vector<Expression> target_list;
		std::vector<RelExpression> from_clause;
		Expression where_clause;
		std::optional<GroupClause> group_clause;
		std::optional<SetQuantifier> set_quantifier;

		QueryExpr();
	};

	struct OrderOp {
		DEFAULT_EQ(OrderOp);

		RelExpression expr;
		std::vector<box<SortSpec>> fields;

		explicit OrderOp(RelExpression expr, std::vector<box<SortSpec>> fields);
	};

	struct SetOp {
		DEFAULT_EQ(SetOp);

		enum class Op {
			UNION,
			INTERSECT,
			EXCEPT
		};

		Op op;
		RelExpression left;
		RelExpression right;
		std::optional<SetQuantifier> quantifier;

		SetOp(RelExpression left, Op op, RelExpression right);
	};

	struct ValuesExpr {
		DEFAULT_EQ(ValuesExpr);

		std::vector<Expression> rows;

		explicit ValuesExpr(std::vector<Expression> rows);
	};

	struct RowExpr {
		DEFAULT_EQ(RowExpr);

		std::vector<Expression> exprs;

		RowExpr();
		explicit RowExpr(std::vector<Expression> exprs);
	};

	struct GroupingSet {
		DEFAULT_EQ(GroupingSet);

		std::vector<Expression> columns;

		GroupingSet();
	};

	struct GroupingSets {
		DEFAULT_EQ(GroupingSets);

		std::vector<Grouping> sets;

		GroupingSets();
	};

	struct Rollup {
		DEFAULT_EQ(Rollup);

		std::vector<box<GroupingSet>> sets;

		Rollup();
	};

	struct Cube {
		DEFAULT_EQ(Cube);

		std::vector<box<GroupingSet>> sets;

		Cube();
	};

	struct RowSubquery {
		DEFAULT_EQ(RowSubquery);

		RelExpression subquery;

		explicit RowSubquery(RelExpression expr);
	};

	struct BetweenPred {
		DEFAULT_EQ(BetweenPred);

		Expression val;
		Expression low;
		Expression high;
		bool symmetric;

		BetweenPred(Expression val, Expression low, Expression high);
	};

	struct InPred {
		DEFAULT_EQ(InPred);

		Expression val;
		RelExpression rows;
		bool symmetric;

		InPred(Expression val, RelExpression rows);
	};
}

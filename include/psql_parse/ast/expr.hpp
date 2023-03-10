#pragma once

#include <cstdint>

#include "common.hpp"

namespace psql_parse {

	struct AliasExpr;
    struct Asterisk;
	struct IntegerLiteral;
	struct FloatLiteral;
	struct StringLiteral;
    struct BooleanLiteral;
	struct UnaryOp;
	struct BinaryOp;
	struct RowSubquery;
	struct Var;
	struct Collate;
	struct IsExpr;
	struct BetweenPred;
	struct InPred;
    struct LikePred;
	struct ExistsPred;
	struct UniquePred;
	struct SortSpec;
	struct RowExpr;
    struct AggregateExpr;

	struct GroupingSet;
	struct GroupingSets;
	struct Rollup;
	struct Cube;


	struct JoinExpr;
	struct TableName;
    struct TableAlias;
	struct SelectExpr;
	struct ValuesExpr;
	struct SetOp;
	struct Query;

	using Expression = std::variant<
            box<Asterisk>,
			box<IntegerLiteral>,
			box<FloatLiteral>,
			box<StringLiteral>,
            box<BooleanLiteral>,
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
            box<LikePred>,
		    box<ExistsPred>,
		    box<UniquePred>,
			box<SortSpec>,
			box<GroupingSet>,
			box<GroupingSets>,
			box<Rollup>,
			box<Cube>,
            box<AggregateExpr>>;

	using Grouping = std::variant<
			box<GroupingSet>,
			box<GroupingSets>,
			box<Rollup>,
			box<Cube>>;

	using RelExpression = std::variant<
			box<JoinExpr>,
			box<TableName>,
            box<TableAlias>,
			box<SelectExpr>,
			box<ValuesExpr>,
			box<SetOp>,
			box<Query>>;
	
	/// <expr> AS <name>
	struct AliasExpr {
		DEFAULT_EQ(AliasExpr);

		Name name;
		Expression expr;

		AliasExpr(std::string name, Expression expr);
	};

    struct Asterisk {
        DEFAULT_EQ(Asterisk);
    };

    struct IntegerLiteral {
		DEFAULT_EQ(IntegerLiteral);

		std::uint64_t value;

		explicit IntegerLiteral(std::uint64_t value);
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

    struct BooleanLiteral {
        DEFAULT_EQ(BooleanLiteral);

        enum class Val {
            TRUE,
            FALSE,
            UNKNOWN
        };

        Val value;

        explicit BooleanLiteral(Val value);
    };


	struct Var {
		DEFAULT_EQ(Var);

		Name name;

		explicit Var(std::string);
	};

	struct Collate {
		DEFAULT_EQ(Collate);

		Expression var;
		box<QualifiedName> collation;

		Collate(Expression var, box<QualifiedName> collation);
	};

	struct IsExpr {
		DEFAULT_EQ(IsExpr);

		Expression inner;
		box<BooleanLiteral> truth_value;

		IsExpr(Expression inner, box<BooleanLiteral> truth_value);
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
			EQUAL, NOT_EQUAL,
			CONCAT
		};

		Op op;
		Expression left;
		Expression right;

		BinaryOp(Expression left, Op op, Expression right);
	};

	struct TableName {
		DEFAULT_EQ(TableName);

		box<QualifiedName> name;

		explicit TableName(box<QualifiedName> name);
	};

    struct TableAlias {
        DEFAULT_EQ(TableAlias);

        Name name;
        std::vector<Name> columns;
        RelExpression expression;

        explicit TableAlias(Name name);
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
		std::optional<Expression> qualifier;
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

    struct WithSpec {
        DEFAULT_EQ(WithSpec);

        Name name;
        std::optional<std::vector<Name>> columns;
        box<Query> query;

        explicit WithSpec(std::string name, box<Query> query);
    };

    struct WithClause {
        DEFAULT_EQ(WithClause);

        bool recursive;
        std::vector<box<WithSpec>> elements;

        WithClause();
    };

	struct Window {
		DEFAULT_EQ(Window);

		struct Frame {
			DEFAULT_EQ(Frame);

			enum class Unit {
				ROWS,
				RANGE,
				GROUPS
			};

			enum class BoundKind {
				CURRENT_ROW,
				PRECEDING,
				FOLLOWING
			};

			enum class Exclusion {
				CURRENT_ROW,
				GROUP,
				TIES,
				NO_OTHERS
			};

			/*
			 * This is loose from a typing perspective,
			 * somewhat to avoid nested std::variants
			 */
			using Bound = std::pair<BoundKind, Expression>;

			Unit unit;
			Bound start;
			std::optional<Bound> end;
			std::optional<Exclusion> exclude;

		};

		Name window_name;
		std::optional<Name> existing_window;
		/*
		 * Empty vector = no partition clause
		 */
		std::vector<Expression> partition;
		std::vector<box<SortSpec>> sort;
		std::optional<Frame> frame;
	};

	struct SelectExpr {
		DEFAULT_EQ(SelectExpr);

		std::vector<Expression> target_list;
		std::vector<RelExpression> from_clause;
		std::optional<Expression> where_clause;
		std::optional<GroupClause> group_clause;
		std::optional<Expression> having_clause;
		std::vector<box<Window>> window_clause;
		std::optional<SetQuantifier> set_quantifier;

		SelectExpr();
	};

	struct Fetch {
		DEFAULT_EQ(Fetch);

		enum class Kind {
			FIRST,
			NEXT
		};


		Kind kind;
		bool with_ties;
		bool percent;
		std::optional<box<IntegerLiteral>> value;
	};

	struct Query {
		DEFAULT_EQ(Query);

		RelExpression expr;
        std::optional<box<WithClause>> with;
		std::vector<box<SortSpec>> order;
		std::optional<box<IntegerLiteral>> offset;
		std::optional<Fetch> fetch;

		explicit Query(RelExpression expr);
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

		InPred(Expression val, RelExpression rows);
	};

	struct LikePred {
		DEFAULT_EQ(LikePred);

		Expression val;
		Expression pattern;
		std::optional<Expression> escape;

		LikePred(Expression val, Expression pattern);
	};

	struct ExistsPred {
		DEFAULT_EQ(ExistsPred);

		box<Query> subquery;

		explicit ExistsPred(box<Query> subquery);
	};

	struct UniquePred {
		DEFAULT_EQ(UniquePred);

		box<Query> subquery;

		explicit UniquePred(box<Query> subquery);
	};

    struct AggregateExpr {
        DEFAULT_EQ(AggregateExpr);

        enum class Op {
            AVG, MAX, MIN, SUM, COUNT,
            EVERY, ANY, SOME, STDDEV_POP, STDDEV_SAMP,
            VAR_POP, VAR_SAMP,
            COLLECT, FUSION, INTERSECTION
        };

        // Semantic check required:
        // Only COUNT may use ASTERISK
        // when ASTERISK is used: Quantifier is meaningless
        Op op;
        Expression argument;
        std::optional<SetQuantifier> quantifier;
        std::optional<Expression> filter;

        AggregateExpr(Op op, Expression argument);
    };
}

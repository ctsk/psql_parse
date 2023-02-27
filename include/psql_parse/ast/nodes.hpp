#pragma once

#include <variant>

#define ADD_AS_VARIANT()	\
	ExprPtrs asVariant() override { 	\
		return this;       	\
	}

namespace psql_parse {
	using ExprPtrs = std::variant<
			struct AliasExpr *,
			struct IntegerLiteral *,
			struct FloatLiteral *,
			struct StringLiteral *,
			struct Var*,
			struct IsExpr*,
			struct UnaryOp*,
			struct BinaryOp*,
			struct TableName*,
			struct JoinExpr*,
			struct QueryExpr*,
			struct OrderOp*,
			struct SetOp*,
			struct ValuesExpr*,
			struct RowSubquery*,
			struct BetweenPred*,
			struct InPred*>;


	struct Node {
		location loc;
		virtual ~Node() = default;
		Node(location loc): loc(loc) {};
	};
}

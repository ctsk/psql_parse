#pragma once

#include <variant>

#include "psql_parse/ast/create.hpp"
#include "psql_parse/ast/select.hpp"
#include "psql_parse/ast/stmt.hpp"
#include "ast/expr.hpp"

namespace psql_parse {

    struct printer {
        std::ostream& out;
        struct rel_expr_visitor {
            printer& context;
            void operator()(box<JoinExpr>&);
            void operator()(box<TableName>&);
            void operator()(box<TableAlias>&);
            void operator()(box<SelectExpr>&);
            void operator()(box<ValuesExpr>&);
            void operator()(box<SetOp>&);
            void operator()(box<Query>&);
        };

        struct expr_visitor {
            printer& context;
            void operator()(box<AliasExpr> &expr);
            void operator()(box<Asterisk> &expr);
            void operator()(box<IntegerLiteral> &expr);
            void operator()(box<FloatLiteral> &expr);
            void operator()(box<StringLiteral> &expr);
            void operator()(box<BooleanLiteral> &expr);
            void operator()(box<UnaryOp> &expr);
            void operator()(box<BinaryOp> &expr);
            void operator()(box<RowExpr> &expr);
            void operator()(box<RowSubquery> &expr);
            void operator()(box<Var> &expr);
            void operator()(box<Collate> &expr);
            void operator()(box<IsExpr> &expr);
            void operator()(box<BetweenPred> &expr);
            void operator()(box<InPred> &expr);
            void operator()(box<LikePred> &expr);
            void operator()(box<ExistsPred> &expr);
            void operator()(box<UniquePred> &expr);
            void operator()(box<SortSpec> &expr);
            void operator()(box<GroupingSet> &expr);
            void operator()(box<GroupingSets> &expr);
            void operator()(box<Rollup> &expr);
            void operator()(box<Cube> &expr);
            void operator()(box<AggregateExpr> &expr);
        };

        struct stmt_visitor {
            printer& context;
            void operator()(box<CreateStatement> &stmt);
            void operator()(box<SelectStatement> &stmt);
        };

        rel_expr_visitor rev;
        expr_visitor ev;
        stmt_visitor sv;

        explicit printer(std::ostream &out)
        : out(out), rev {*this}, ev {*this}, sv{*this} {};

        void print(Expression& expr);

        void print(RelExpression& expr);

        void print(Statement& stmt);

        void print(SetQuantifier q);

        void print(JoinExpr::Kind q);

        void print(QualifiedName& q);

        void print(BooleanLiteral::Val v);
    };

}
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


    template <typename T>
    struct default_visit {
        struct rel_expr_visitor {
            default_visit& context;
            void operator()(box<JoinExpr>& expr) {
                context.visit(expr->first);
                context.visit(expr->second);

                if (expr->qualifier.has_value())
                    context.visit(expr->qualifier.value());

                for (auto &col : expr->columns)
                    context.visit(col);
            }

            void operator()(box<TableName>&) {}

            void operator()(box<TableAlias>&expr) {
                context.visit(expr->expression);
            }

            void operator()(box<SelectExpr>& expr) {
                for (auto &target : expr->target_list)
                    context.visit(target);

                for (auto &from : expr->from_clause)
                    context.visit(from);

                if (expr->where_clause.has_value())
                    context.visit(expr->where_clause.value());

                if (expr->group_clause.has_value())
                    for (auto &group : expr->group_clause->group_clause)
                        context.visit(group);

                if (expr->having_clause.has_value())
                    context.visit(expr->having_clause.value());
            }

            void operator()(box<ValuesExpr> &expr) {
                for (auto &row : expr->rows)
                    context.visit(row);
            }

            void operator()(box<SetOp>& expr) {
                context.visit(expr->left);
                context.visit(expr->right);
            }

            void operator()(box<Query>& expr) {
                context.visit(expr->expr);

                for (auto &sort : expr->order)
                    context.visit(sort);

                if (expr->offset.has_value())
                    context.visit(expr->offset.value());

                if (expr->fetch.has_value())
                    context.visit(expr->fetch.value());
            }
        };

        struct expr_visitor {
            default_visit& context;
            void operator()(box<AliasExpr> &expr) {
                context.visit(expr->expr);
            };
            void operator()(box<Asterisk> &) {};
            void operator()(box<IntegerLiteral> &) {};
            void operator()(box<FloatLiteral> &) {};
            void operator()(box<StringLiteral> &) {};
            void operator()(box<BooleanLiteral> &) {};
            void operator()(box<UnaryOp> &expr) {
                context.visit(expr->inner);
            };
            void operator()(box<BinaryOp> &expr) {
                context.visit(expr->left);
                context.visit(expr->right);
            };
            void operator()(box<RowExpr> &expr) {
                for (auto &ex : expr->exprs)
                    context.visit(ex);
            };
            void operator()(box<RowSubquery> &expr) {
                context.visit(expr->subquery);
            };
            void operator()(box<Var> &expr) {

            };
            void operator()(box<Collate> &expr) {
                context.visit(expr->var);
            };
            void operator()(box<IsExpr> &expr) {
                context.visit(expr->inner);
            };
            void operator()(box<BetweenPred> &expr) {
                context.visit(expr->val);
                context.visit(expr->low);
                context.visit(expr->high);
            };
            void operator()(box<InPred> &expr) {
                context.visit(expr->val);
                context.visit(expr->rows);
            };
            void operator()(box<LikePred> &expr) {
                context.visit(expr->val);
                context.visit(expr->pattern);
                context.visit(expr->escape);
            };
            void operator()(box<ExistsPred> &expr) {
                context.visit(expr->subquery);
            };
            void operator()(box<UniquePred> &expr) {
                context.visit(expr->subquery);
            };
            void operator()(box<SortSpec> &expr) {
                context.visit(expr->expr);
            };
            void operator()(box<GroupingSet> &expr) {
                for (auto &ex : expr->columns)
                    context.visit(ex);
            };
            void operator()(box<GroupingSets> &expr) {
                for (auto &ex : expr->sets)
                    context.visit(ex);
            };
            void operator()(box<Rollup> &expr) {
                for (auto &ex : expr->sets)
                    context.visit(ex);
            };
            void operator()(box<Cube> &expr) {
                for (auto &ex : expr->sets)
                    context.visit(ex);
            };
            void operator()(box<AggregateExpr> &expr) {
                context.visit(expr->argument);

                if (expr->filter.has_value())
                    context.visit(expr->filter.value());
            };
        };

        struct stmt_visitor {
            default_visit& context;
            void operator()(box<CreateStatement> &stmt){};
            void operator()(box<SelectStatement> &stmt){};
        };

        T& derived;

        void visit(RelExpression& expr) {
            derived.visit(expr);
        }
        void visit(Expression& expr) {
            derived.visit(expr);
        }
        void visit(Statement & stmt) {
            derived.visit(stmt);
        }
    };
}
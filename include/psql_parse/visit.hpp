#pragma once

#include <variant>

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

        printer(std::ostream &out)
        : out(out), rev {*this}, ev {*this}, sv{*this} {};

        void print(Expression& expr) {
            std::visit(ev, expr);
        }

        void print(RelExpression& expr) {
            std::visit(rev, expr);
        }

        void print(Statement& stmt) {
            std::visit(sv, stmt);
        }
    };

    void printer::stmt_visitor::operator()(box<CreateStatement> &stmt) {
        context.out << "CREATE: ";
    }

    void printer::stmt_visitor::operator()(box<SelectStatement> &stmt) {
        context.out << "SELECT: ";
        context.rev(stmt->rel_expr);
    }

    void printer::rel_expr_visitor::operator()(box<JoinExpr> &expr) {
        std::string kind;
        switch (expr->kind) {
            case JoinExpr::Kind::FULL:
                kind = "FULL OUTER";
                break;
            case JoinExpr::Kind::LEFT:
                kind = "LEFT OUTER";
                break;
            case JoinExpr::Kind::RIGHT:
                kind = "RIGHT OUTER";
                break;
            case JoinExpr::Kind::INNER:
                kind = "INNER";
                break;
        }

        context.out << "(join " << "[" << kind << "]";
        if (expr->natural) {
            context.out << " NATURAL ";
        }

        std::visit(context.ev, expr->qualifier);
        if (!expr->columns.empty()) {
            context.out << " (";
            context.out << expr->columns.at(0);
            for (unsigned i = 1; i < expr->columns.size(); i++) {
                context.out << ", " << expr->columns.at(i);
            }
            context.out << ")";
        }

        std::visit(*this, expr->first);
        context.out << " ";
        std::visit(*this, expr->second);
    }

    void printer::rel_expr_visitor::operator()(box<TableName> &expr) {
        for (auto &i : expr->name->qualifier) {
            context.out << i << ".";
        }
        context.out << expr->name->name;
    }

    void printer::rel_expr_visitor::operator()(box<TableAlias> &expr) {
        context.out << "(as ";
        std::visit(*this, expr->expression);
        context.out << " " << expr->name;
        if (!expr->columns.empty()) {
            context.out << "(" << expr->columns.at(0);
            for (unsigned i = 1; i < expr->columns.size(); i++) {
                context.out << ", " << expr->columns.at(i);
            }
            context.out << ")";
        }
        context.out << ")";
    }

    void printer::rel_expr_visitor::operator()(box<SelectExpr> &expr) {
        context.out << "(select";

        if (expr->set_quantifier.has_value()) {
            switch (expr->set_quantifier.value()) {
                case SetQuantifier::ALL:
                    context.out << " ALL";
                    break;
                case SetQuantifier::DISTINCT:
                    context.out << " DISTINCT";
                    break;
            }
        }

        if (!expr->target_list.empty()) {
            context.out << " (";
            std::visit(context.ev, expr->target_list.at(0));
            for (unsigned i = 1; i < expr->target_list.size(); i++) {
                context.out << " ";
                std::visit(context.ev, expr->target_list.at(i));
            }
            context.out << ")";
        }

        if (!expr->from_clause.empty()) {
            context.out << " (";
            std::visit(*this, expr->from_clause.at(0));
            for (unsigned i = 1; i < expr->from_clause.size(); i++) {
                context.out << " ";
                std::visit(*this, expr->from_clause.at(i));
            }
            context.out << ")";
        }

        if (expr->where_clause.has_value()) {
            context.out << " (where ";
            std::visit(context.ev, expr->where_clause.value());
            context.out << ")";
        }

        if (expr->group_clause.has_value()) {
            context.out << " (group";
            if (expr->group_clause->group_quantifier.has_value()) {
                switch (expr->group_clause->group_quantifier.value()) {
                    case SetQuantifier::ALL:
                        context.out << " ALL";
                        break;
                    case SetQuantifier::DISTINCT:
                        context.out << " DISTINCT";
                        break;
                }
            }

            if (!expr->group_clause->group_clause.empty()) {
                std::visit(context.ev, expr->group_clause->group_clause.at(0));
                for (unsigned i = 1; i < expr->group_clause->group_clause.size(); i++) {
                    context.out << " ";
                    std::visit(context.ev, expr->group_clause->group_clause.at(i));
                }
                context.out << ")";
            }
        }

        if (expr->having_clause.has_value()) {
            context.out << " (having ";
            std::visit(context.ev, expr->having_clause.value());
            context.out << ")";
        }

        context.out << ")";

        // todo: handle window
    }

    // Print a value expression
    void printer::rel_expr_visitor::operator()(box<ValuesExpr> &expr) {
        context.out << "(values ";
        if (!expr->rows.empty()) {
            std::visit(context.ev, expr->rows.at(0));
            for (unsigned i = 1; i < expr->rows.size(); i++) {
                context.out << " ";
                std::visit(context.ev, expr->rows.at(i));
            }
        }
        context.out << ")";
    }

    void printer::rel_expr_visitor::operator()(box<SetOp> &expr) {
        std::string op;
        switch(expr->op) {
            case SetOp::Op::UNION:
                op = "UNION";
                break;
            case SetOp::Op::INTERSECT:
                op = "INTERSECT";
                break;
            case SetOp::Op::EXCEPT:
                op = "EXCEPT";
                break;
        }
        if (expr->quantifier.has_value()) {
            switch (expr->quantifier.value()) {
                case SetQuantifier::ALL:
                    op += " ALL";
                    break;
                case SetQuantifier::DISTINCT:
                    op += " DISTINCT";
                    break;
            }
        }

        context.out << "(" << op << " ";
        std::visit(*this, expr->left);
        context.out << " ";
        std::visit(*this, expr->right);
        context.out << ")";
    }

    void printer::rel_expr_visitor::operator()(box<Query> &expr) {
        context.out << "(query ";
        std::visit(*this, expr->expr);
        if (!expr->order.empty()) {
            context.out << " (";
            for (auto &a : expr->order) {
                context.ev(a);
            }
            context.out << ")";
        }
        if (expr->offset.has_value()) {
            context.out << " (offset ";
            context.ev(expr->offset.value());
            context.out << ")";
        }
        if (expr->fetch.has_value()) {
            context.out << " (fetch ";
            auto &fetch = expr->fetch.value();
            switch(fetch.kind) {
                case Fetch::Kind::FIRST:
                    context.out << "FIRST ";
                    break;
                case Fetch::Kind::NEXT:
                    context.out << "NEXT ";
                    break;
            }
            if (fetch.with_ties) {
                context.out << "WITH TIES ";
            }
            if (fetch.percent) {
                context.out << "PERCENT ";
            }
            if (fetch.value.has_value()) {
                context.ev(fetch.value.value());
            }
            context.out << ")";
        }

    }

    void printer::expr_visitor::operator()(box<AliasExpr> &expr) {
        context.out << "(" << "AS" << " " << expr->name << " ";
        std::visit(*this, expr->expr);
        context.out << ")";
    }

    void printer::expr_visitor::operator()(box<IntegerLiteral> &expr) {
        context.out << expr->value;
    }

    void printer::expr_visitor::operator()(box<FloatLiteral> &expr) {
        context.out << expr->value;
    }

    void printer::expr_visitor::operator()(box<StringLiteral> &expr) {
        context.out << expr->value;
    }

    void printer::expr_visitor::operator()(box<BooleanLiteral> &expr) {
        using enum BooleanLiteral::Val;
        switch (expr->value) {
            case TRUE:
                context.out << "TRUE";
                break;
            case FALSE:
                context.out << "FALSE";
                break;
            case UNKNOWN:
                context.out << "UNKNOWN";
                break;
        }
    }

    void printer::expr_visitor::operator()(box<UnaryOp> &expr) {
        std::string op;
        switch (expr->op) {

            case UnaryOp::Op::NOT:
                op = "NOT";
                break;
            case UnaryOp::Op::NEG:
                op = "-";
                break;
        }
        context.out << "(" << op;
        std::visit(*this, expr->inner);
        context.out << ")";
    }

    void printer::expr_visitor::operator()(box<BinaryOp> &expr) {
        std::string op;
        switch (expr->op) {
            case BinaryOp::Op::OR:
                op = "OR";
                break;
            case BinaryOp::Op::AND:
                op = "AND";
                break;
            case BinaryOp::Op::ADD:
                op = "+";
                break;
            case BinaryOp::Op::SUB:
                op = "-";
                break;
            case BinaryOp::Op::MULT:
                op = "*";
                break;
            case BinaryOp::Op::DIV:
                op = "/";
                break;
            case BinaryOp::Op::LESS:
                op = "<";
                break;
            case BinaryOp::Op::LESS_EQUAL:
                op = "<=";
                break;
            case BinaryOp::Op::GREATER:
                op = ">";
                break;
            case BinaryOp::Op::GREATER_EQUAL:
                op = ">=";
                break;
            case BinaryOp::Op::EQUAL:
                op = "=";
                break;
            case BinaryOp::Op::NOT_EQUAL:
                op = "<>";
                break;
            case BinaryOp::Op::CONCAT:
                op = "||";
                break;
        }
        context.out << "(" << op << " ";
        std::visit(*this, expr->left);
        context.out << " ";
        std::visit(*this, expr->right);
        context.out << ")";
    }

    void printer::expr_visitor::operator()(box<RowExpr> &expr) {
        context.out << "(row";
        for (auto &e : expr->exprs) {
            context.out << " ";
            std::visit(*this, e);
        }
    }

    void printer::expr_visitor::operator()(box<RowSubquery>& expr) {
        context.out << "(row ";
        std::visit(context.rev, expr->subquery);
    }

//    void printer::expr_visitor::operator()(auto &expr) {
//        context.out << "[Print not implement]" << std::endl;
//    }

    void printer::expr_visitor::operator()(box<Var> &expr) {
        context.out << expr->name << std::endl;
    }

    void printer::expr_visitor::operator()(box<Collate> &expr) {
        context.out << "(collate ";
        std::visit(*this, expr->var);
        context.out << " ";
        for (const auto& q: expr->collation->qualifier) {
            context.out << q << ".";
        }
        context.out << expr->collation->name << ")";
    }

    void printer::expr_visitor::operator()(box<IsExpr> &expr) {
        context.out << "(is ";
        std::visit(*this, expr->inner);
        context.out << " ";
        (*this)(expr->truth_value);
        context.out << ")";
    }

    void printer::expr_visitor::operator()(box<BetweenPred> &expr) {
        context.out << "(between ";
        std::visit(*this, expr->val);
        context.out << " ";
        std::visit(*this, expr->low);
        context.out << " ";
        std::visit(*this, expr->high);
        context.out << ")";
    }

    void printer::expr_visitor::operator()(box<InPred> &expr) {
        context.out << "(in ";
        if (expr->symmetric) {
            context.out << "SYM ";
        } else {
            context.out << "ASYM ";
        }
        std::visit(*this, expr->val);
        context.out << " ";
        std::visit(context.rev, expr->rows);
        context.out << ")";
    }

    void printer::expr_visitor::operator()(box<LikePred> &expr) {
        context.out << "(like ";
        std::visit(*this, expr->val);
        context.out << " ";
        std::visit(*this, expr->pattern);
        if (expr->escape.has_value()) {
            context.out << " ";
            std::visit(*this, expr->escape.value());
        }
        context.out << ")";
    }

    void printer::expr_visitor::operator()(box<ExistsPred> &expr) {
        context.out << "(exists ";
        context.rev(expr->subquery);
        context.out << ")";
    }

    void printer::expr_visitor::operator()(box<UniquePred> &expr) {
        context.out << "(unique ";
        context.rev(expr->subquery);
        context.out << ")";
    }

    void printer::expr_visitor::operator()(box<SortSpec> &expr) {
        std::string order;
        switch(expr->order) {
            case SortSpec::Order::ASC:
                order = "ASC";
                break;
            case SortSpec::Order::DESC:
                order = "DESC";
                break;
        }

        std::string nulls;
        switch (expr->null_order) {
            case SortSpec::NullOrder::DEFAULT:
                nulls = "DEFAULT";
                break;
            case SortSpec::NullOrder::FIRST:
                nulls = "FIRST";
                break;
            case SortSpec::NullOrder::LAST:
                nulls = "LAST";
                break;
        }

        context.out << "(sort-by ";
        std::visit(*this, expr->expr);
        context.out << " " << order << " nulls:" << nulls << ")";
    }

    void printer::expr_visitor::operator()(box<GroupingSet> &expr) {
        context.out << "(grouping ";
        for (auto &g : expr->columns) {
            std::visit(*this, g);
            context.out << " ";
        }
        context.out << ")";
    }

    void printer::expr_visitor::operator()(box<GroupingSets> &expr) {
        context.out << "(grouping-sets ";
        for (auto &g : expr->sets) {
            std::visit(*this, g);
            context.out << " ";
        }
        context.out << ")";
    }

    void printer::expr_visitor::operator()(box<Rollup> &expr) {
        context.out << "(rollup ";
        for (auto &g : expr->sets) {
            (*this)(g);
            context.out << " ";
        }
        context.out << ")";
    }

    void printer::expr_visitor::operator()(box<Cube> &expr) {
        context.out << "(cube ";
        for (auto &g : expr->sets) {
            (*this)(g);
            context.out << " ";
        }
        context.out << ")";
    }

    void printer::expr_visitor::operator()(box<Asterisk> &expr) {
        context.out << "*";
    }

    void printer::expr_visitor::operator()(box<AggregateExpr> &expr) {
        context.out << "(aggregate ";
        using
        enum AggregateExpr::Op;
        switch (expr->op) {
            case AVG:
                context.out << "avg";
                break;
            case MAX:
                context.out << "max";
                break;
            case MIN:
                context.out << "min";
                break;
            case SUM:
                context.out << "sum";
                break;
            case EVERY:
                context.out << "every";
                break;
            case ANY:
                context.out << "any";
                break;
            case SOME:
                context.out << "some";
                break;
            case COUNT:
                context.out << "count";
                break;
            case STDDEV_POP:
                context.out << "stddev_pop";
                break;
            case STDDEV_SAMP:
                context.out << "stddev_samp";
                break;
            case VAR_POP:
                context.out << "var_pop";
                break;
            case VAR_SAMP:
                context.out << "var_samp";
                break;
            case COLLECT:
                context.out << "collect";
                break;
            case FUSION:
                context.out << "fusion";
                break;
            case INTERSECTION:
                context.out << "intersection";
                break;
        }
        context.out << " ";
        if (expr->quantifier.has_value()) {
            switch (expr->quantifier.value()) {
                case SetQuantifier::DISTINCT:
                    context.out << "distinct ";
                    break;
                case SetQuantifier::ALL:
                    context.out << "all ";
                    break;
            }
        }
        std::visit(*this, expr->argument);
        if (expr->filter.has_value()) {
            context.out << " ";
            std::visit(*this, expr->filter.value());
        }
        context.out << ")";
    }
}
#pragma clang diagnostic push
#pragma ide diagnostic ignored "misc-no-recursion"
#include "psql_parse/visit.hpp"

namespace psql_parse {

    void printer::stmt_visitor::operator()(box<CreateStatement> &) {
        context.out << "CREATE\n";
    }

    void printer::stmt_visitor::operator()(box<SelectStatement> &stmt) {
        context.out << "SELECT: ";
        context.rev(stmt->rel_expr);
    }

    void printer::rel_expr_visitor::operator()(box<JoinExpr> &expr) {
        context.out << "(";

        std::visit(*this, expr->first);

        context.out << " ";
        context.print(expr->kind);
        if (expr->natural) {
            context.out << " NATURAL";
        }
        context.out << " JOIN ";

        std::visit(*this, expr->second);

        if (expr->qualifier.has_value()) {
            context.out << " ON ";
            std::visit(context.ev, expr->qualifier.value());
        } else if (!expr->columns.empty()) {
            context.out << " USING (";
            context.out << expr->columns.at(0);
            for (unsigned i = 1; i < expr->columns.size(); i++) {
                context.out << ", " << expr->columns.at(i);
            }
            context.out << ")";
        }

        context.out << ")";
    }

    void printer::rel_expr_visitor::operator()(box<TableName> &expr) {
        context.print(*(expr->name));
    }

    void printer::rel_expr_visitor::operator()(box<TableAlias> &expr) {
        context.out << "(";
        std::visit(*this, expr->expression);
        context.out << " AS " << expr->name;
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
        context.out << "(SELECT";

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
            std::visit(context.ev, expr->target_list.at(0));
            for (unsigned i = 1; i < expr->target_list.size(); i++) {
                context.out << ", ";
                std::visit(context.ev, expr->target_list.at(i));
            }
        }

        if (!expr->from_clause.empty()) {
            context.out << " FROM ";
            std::visit(*this, expr->from_clause.at(0));
            for (unsigned i = 1; i < expr->from_clause.size(); i++) {
                context.out << ", ";
                std::visit(*this, expr->from_clause.at(i));
            }
        }


        if (expr->where_clause.has_value()) {
            context.out << " WHERE ";
            std::visit(context.ev, expr->where_clause.value());
        }

        if (expr->group_clause.has_value()) {
            context.out << " GROUP BY ";
            if (expr->group_clause->group_quantifier.has_value()) {
                context.print(expr->group_clause->group_quantifier.value());
            }

            if (!expr->group_clause->group_clause.empty()) {
                context.out << " ";
                std::visit(context.ev, expr->group_clause->group_clause.at(0));
                for (unsigned i = 1; i < expr->group_clause->group_clause.size(); i++) {
                    context.out << ", ";
                    std::visit(context.ev, expr->group_clause->group_clause.at(i));
                }
            }
        }

        if (expr->having_clause.has_value()) {
            context.out << " HAVING ";
            std::visit(context.ev, expr->having_clause.value());
        }

        context.out << ")";

        // todo: handle window
    }

    // Print a value expression
    void printer::rel_expr_visitor::operator()(box<ValuesExpr> &expr) {
        context.out << "(VALUES ";
        if (!expr->rows.empty()) {
            std::visit(context.ev, expr->rows.at(0));
            for (unsigned i = 1; i < expr->rows.size(); i++) {
                context.out << ", ";
                std::visit(context.ev, expr->rows.at(i));
            }
        }
        context.out << ")";
    }

    void printer::rel_expr_visitor::operator()(box<SetOp> &expr) {
        context.out << "(";
        std::visit(*this, expr->left);

        switch(expr->op) {
            case SetOp::Op::UNION:
                context.out << " UNION";
                break;
            case SetOp::Op::INTERSECT:
                context.out << " INTERSECT";
                break;
            case SetOp::Op::EXCEPT:
                context.out << " EXCEPT";
                break;
        }

        if (expr->quantifier.has_value()) {
            context.out << " ";
            context.print(expr->quantifier.value());
            context.out << " ";
        }

        std::visit(*this, expr->right);

        context.out << ")";
    }

    void printer::rel_expr_visitor::operator()(box<Query> &expr) {
        context.out << "( ";

        std::visit(*this, expr->expr);

        if (!expr->order.empty()) {
            context.out << " ORDER BY ";
            context.ev(expr->order.at(0));
            for (unsigned i = 1; i < expr->order.size(); i++) {
                context.out << ", ";
                context.ev(expr->order.at(i));
            }
            context.out << "";
        }

        if (expr->offset.has_value()) {
            context.out << " OFFSET ";
            context.ev(expr->offset.value());
        }

        if (expr->fetch.has_value()) {
            context.out << " FETCH ";
            auto &fetch = expr->fetch.value();
            switch(fetch.kind) {
                case Fetch::Kind::FIRST:
                    context.out << "FIRST ";
                    break;
                case Fetch::Kind::NEXT:
                    context.out << "NEXT ";
                    break;
            }

            if (fetch.value.has_value()) {
                context.ev(fetch.value.value());

                if (fetch.percent) {
                    context.out << " PERCENT ";
                }
            }

            context.out << " ROWS ";

            if (fetch.with_ties) {
                context.out << " WITH TIES ";
            } else {
                context.out << " ONLY ";
            }

        }

        context.out << ")";
    }

    void printer::expr_visitor::operator()(box<AliasExpr> &expr) {
        context.out << "(";
        std::visit(*this, expr->expr);
        context.out << " AS " << expr->name << " )";
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
        context.print(expr->value);
    }

    void printer::expr_visitor::operator()(box<UnaryOp> &expr) {
        context.out << "(";
        switch (expr->op) {
            case UnaryOp::Op::NOT:
                context.out << "NOT ";
                break;
            case UnaryOp::Op::NEG:
                context.out << "- ";
                break;
        }

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
        context.out << "(";
        std::visit(*this, expr->left);
        context.out << " " << op << " ";
        std::visit(*this, expr->right);
        context.out << ")";
    }

    void printer::expr_visitor::operator()(box<RowExpr> &expr) {
        context.out << "(ROW";

        if (!expr->exprs.empty()) {
            context.out << " (";
            std::visit(*this, expr->exprs.at(0));
            for (unsigned i = 1; i < expr->exprs.size(); i++) {
                context.out << ", ";
                std::visit(*this, expr->exprs.at(i));
            }
            context.out << ")";
        }
        context.out << ")";
    }

    void printer::expr_visitor::operator()(box<RowSubquery>& expr) {
        std::visit(context.rev, expr->subquery);
    }

//    void printer::expr_visitor::operator()(auto &expr) {
//        context.out << "[Print not implement]" << std::endl;
//    }

    void printer::expr_visitor::operator()(box<Var> &expr) {
        context.out << expr->name;
    }

    void printer::expr_visitor::operator()(box<Collate> &expr) {
        context.out << "(";
        std::visit(*this, expr->var);
        context.out << " COLLATE ";
        context.print(*expr->collation);
        context.out << expr->collation->name << ")";
    }

    void printer::expr_visitor::operator()(box<IsExpr> &expr) {
        context.out << "(";
        std::visit(*this, expr->inner);
        context.out << " IS ";
        (*this)(expr->truth_value);
        context.out << ")";
    }

    void printer::expr_visitor::operator()(box<BetweenPred> &expr) {
        context.out << "(";
        std::visit(*this, expr->val);
        context.out << " BETWEEN ";
        if (expr->symmetric) {
            context.out << "SYMMETRIC ";
        }
        std::visit(*this, expr->low);
        context.out << " AND ";
        std::visit(*this, expr->high);
        context.out << ")";
    }

    void printer::expr_visitor::operator()(box<InPred> &expr) {
        context.out << "(";
        std::visit(*this, expr->val);
        context.out << " IN ";
        std::visit(context.rev, expr->rows);
        context.out << ")";
    }

    void printer::expr_visitor::operator()(box<LikePred> &expr) {
        context.out << "(";
        std::visit(*this, expr->val);
        context.out << " LIKE ";
        std::visit(*this, expr->pattern);
        if (expr->escape.has_value()) {
            context.out << " ESCAPE ";
            std::visit(*this, expr->escape.value());
        }
        context.out << ")";
    }

    void printer::expr_visitor::operator()(box<ExistsPred> &expr) {
        context.out << "(EXISTS ";
        context.rev(expr->subquery);
        context.out << ")";
    }

    void printer::expr_visitor::operator()(box<UniquePred> &expr) {
        context.out << "(UNIQUE ";
        context.rev(expr->subquery);
        context.out << ")";
    }

    void printer::expr_visitor::operator()(box<SortSpec> &expr) {
        std::visit(*this, expr->expr);

        switch(expr->order) {
            case SortSpec::Order::ASC:
                context.out << " ASC ";
                break;
            case SortSpec::Order::DESC:
                context.out << " DESC ";
                break;
        }

        if (expr->null_order != SortSpec::NullOrder::DEFAULT) {
            switch (expr->null_order) {
                case SortSpec::NullOrder::FIRST:
                    context.out << " NULLS FIRST ";
                    break;
                case SortSpec::NullOrder::LAST:
                    context.out << " NULLS LAST ";
                    break;
            }
        }
        context.out << ")";
    }

    void printer::expr_visitor::operator()(box<GroupingSet> &expr) {
        context.out << "(";
        if (!expr->columns.empty()) {
            std::visit(*this, expr->columns.at(0));
            for (unsigned i = 1; i < expr->columns.size(); i++) {
                context.out << ", ";
                std::visit(*this, expr->columns.at(i));
            }
        }
        context.out << ")";
    }

    void printer::expr_visitor::operator()(box<GroupingSets> &expr) {
        context.out << "GROUPING SETS (";
        if (!expr->sets.empty()) {
            std::visit(*this, expr->sets.at(0));
            for (unsigned i = 1; i < expr->sets.size(); i++) {
                context.out << ", ";
                std::visit(*this, expr->sets.at(i));
            }
        }
        context.out << ")";
    }

    void printer::expr_visitor::operator()(box<Rollup> &expr) {
        context.out << "ROLLUP (";
        if (!expr->sets.empty()) {
            (*this)(expr->sets.at(0));
            for (unsigned i = 1; i < expr->sets.size(); i++) {
                context.out << ", ";
                (*this)(expr->sets.at(i));
            }
        }
        context.out << ")";
    }

    void printer::expr_visitor::operator()(box<Cube> &expr) {
        context.out << "CUBE (";
        if (!expr->sets.empty()) {
            (*this)(expr->sets.at(0));
            for (unsigned i = 1; i < expr->sets.size(); i++) {
                context.out << ", ";
                (*this)(expr->sets.at(i));
            }
        }
        context.out << ")";
    }

    void printer::expr_visitor::operator()(box<Asterisk> &) {
        context.out << "*";
    }

    void printer::expr_visitor::operator()(box<AggregateExpr> &expr) {
        context.out << "(";
        using enum AggregateExpr::Op;
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
        context.out << " (";
        if (expr->quantifier.has_value()) {
            switch (expr->quantifier.value()) {
                case SetQuantifier::DISTINCT:
                    context.out << "DISTINCT ";
                    break;
                case SetQuantifier::ALL:
                    context.out << "ALL ";
                    break;
            }
        }
        std::visit(*this, expr->argument);
        if (expr->filter.has_value()) {
            context.out << " FILTER WHERE ";
            std::visit(*this, expr->filter.value());
        }
        context.out << ")";
    }

    void printer::print(Expression &expr) {
        std::visit(ev, expr);
    }

    void printer::print(RelExpression &expr) {
        std::visit(rev, expr);
    }

    void printer::print(Statement &stmt) {
        std::visit(sv, stmt);
    }

    void printer::print(SetQuantifier q) {
        switch (q) {
            case SetQuantifier::DISTINCT:
                out << "DISTINCT";
            case SetQuantifier::ALL:
                out << "ALL";
        }
    }

    void printer::print(JoinExpr::Kind q) {
        switch (q) {
            case JoinExpr::Kind::FULL:
                out <<  "FULL OUTER";
            case JoinExpr::Kind::LEFT:
                out <<  "LEFT OUTER";
            case JoinExpr::Kind::RIGHT:
                out <<  "RIGHT OUTER";
            case JoinExpr::Kind::INNER:
                out <<  "INNER";
        }
    }

    void printer::print(QualifiedName &q) {
        for (const auto& part : q.qualifier) {
            out << part << ".";
        }
        out << q.name;
    }

    void printer::print(BooleanLiteral::Val v) {
        switch (v) {
            case BooleanLiteral::Val::TRUE:
                out << "TRUE"; break;
            case BooleanLiteral::Val::FALSE:
                out << "FALSE"; break;
            case BooleanLiteral::Val::UNKNOWN:
                out << "UNKNOWN"; break;
        }
    }
}
#pragma clang diagnostic pop
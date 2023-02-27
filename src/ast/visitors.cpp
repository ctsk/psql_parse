#include <algorithm>
#include "psql_parse/ast/expr.hpp"

namespace {
using namespace psql_parse;

#define DEF_APPLY(ret_type, ptr_type) \
	ret_type operator()(ptr_type* l, ptr_type* r)

struct EqualVisitor
{
	DEF_APPLY(bool, AliasExpr) {
		return l->name == r->name
			&& std::visit(*this, l->expr->asVariant(), r->expr->asVariant());
	}
	DEF_APPLY(bool, IntegerLiteral) {
		return l->value == r->value;
	}
	DEF_APPLY(bool, FloatLiteral) {
		return l->value == r->value;
	}
	DEF_APPLY(bool, StringLiteral) {
		return l->value == r->value;
	}
	DEF_APPLY(bool, Var) {
		return l->name == r->name;
	}
	DEF_APPLY(bool, IsExpr) {
		return l->truth_value == r->truth_value
		    && std::visit(*this, l->inner->asVariant(), r->inner->asVariant());
	}
	DEF_APPLY(bool, UnaryOp) {
		return l->op == r->op
		    && std::visit(*this, l->inner->asVariant(), r->inner->asVariant());
	}
	DEF_APPLY(bool, BinaryOp) {
		return l->op == r->op
			&& std::visit(*this, l->left->asVariant(), r->left->asVariant())
			&& std::visit(*this, l->right->asVariant(), r->right->asVariant());
	}
	DEF_APPLY(bool, TableName) {
		return l->name == r->name;
	}
	DEF_APPLY(bool, JoinExpr) {
		return l->natural == r->natural
		    && l->kind == r->kind
			&& l->columns == r->columns
			&& std::visit(*this, l->qualifier->asVariant(), r->qualifier->asVariant())
			&& std::visit(*this, l->first->asVariant(), r->first->asVariant())
			&& std::visit(*this, l->second->asVariant(), r->second->asVariant());
	}
	DEF_APPLY(bool, QueryExpr) {
		if (l->target_list.size() != r->target_list.size()
		    || l->from_clause.size() != r->from_clause.size()
			|| std::visit(*this, l->where_clause->asVariant(), r->where_clause->asVariant())) {
			return false;
		}

		for (unsigned i = 0; i < l->target_list.size(); i++) {
			if (l->target_list[i].get() == nullptr) {
				if (r->target_list[i].get() == nullptr) {
					continue;
				} else {
					return false;
				}
			}
			if (!std::visit(*this, l->target_list[i]->asVariant(), r->target_list[i]->asVariant())) {
				return false;
			}
		}

		for (unsigned i = 0; i < l->from_clause.size(); i++) {
			if (!std::visit(*this, l->from_clause[i]->asVariant(), r->from_clause[i]->asVariant())) {
				return false;
			}
		}

		return true;
	}
	DEF_APPLY(bool, OrderOp) {
		if (l->fields.size() != r->fields.size()) {
			return false;
		}

		for (unsigned i = 0; i < l->fields.size(); i++) {
			auto &leftField = l->fields[i];
			auto &rightField = r->fields[i];
			if (leftField.order != rightField.order
			    || leftField.null_order != rightField.null_order
				|| std::visit(*this, leftField.expr->asVariant(), rightField.expr->asVariant())) {

				return false;
			}
		}

		if (!std::visit(*this, l->expr->asVariant(), r->expr->asVariant())) {
			return false;
		}

		return true;
	}
	DEF_APPLY(bool, SetOp) {
		return l->op == r->op
		    && l->quantifier == r->quantifier
			&& std::visit(*this, l->left->asVariant(), r->left->asVariant())
		    && std::visit(*this, l->right->asVariant(), r->right->asVariant());
	}
	DEF_APPLY(bool, ValuesExpr) {
		if (l->rows.size() != r->rows.size()) {
			return false;
		}

		for (unsigned i = 0; i < l->rows.size(); i++) {
			if (!std::visit(*this, l->rows[i]->asVariant(), r->rows[i]->asVariant())) {
				return false;
			}
		}

		return true;
	}
	DEF_APPLY(bool, RowSubquery) {
		return std::visit(*this, l->subquery->asVariant(), r->subquery->asVariant());
	}
	DEF_APPLY(bool, BetweenPred) {
		return l->symmetric == r->symmetric
			   && std::visit(*this, l->val->asVariant(), r->val->asVariant())
			   && std::visit(*this, l->low->asVariant(), r->low->asVariant())
			   && std::visit(*this, l->high->asVariant(), r->high->asVariant());
	}
	DEF_APPLY(bool, InPred) {
		return l->symmetric == r->symmetric
			&& std::visit(*this, l->val->asVariant(), r->val->asVariant())
			&& std::visit(*this, l->rows->asVariant(), r->rows->asVariant());
	}

	bool operator()(auto, auto) {
		return false;
	}
};

#undef DEF_APPLY

}

bool expr_equal(Expression *l, Expression* r) {
	return std::visit(EqualVisitor {}, l->asVariant(), r->asVariant());
}

#include "psql_parse/ast/select.hpp"

psql_parse::SelectStatement::SelectStatement(psql_parse::RelExpression relExpr)
: rel_expr(std::move(relExpr)) {}

#include "psql_parse/ast/select.hpp"

psql_parse::SelectStatement::SelectStatement(psql_parse::QueryExpr *queryExpr)
: Statement(queryExpr->loc), query_expr(queryExpr) {}

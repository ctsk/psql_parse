#include "psql_parse/ast/select.hpp"

psql_parse::SelectStatement::SelectStatement(psql_parse::box<psql_parse::Query> queryExpr)
: rel_expr(std::move(queryExpr)) {}

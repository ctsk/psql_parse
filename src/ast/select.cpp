#include "psql_parse/ast/select.hpp"

psql_parse::SelectStatement::SelectStatement(psql_parse::RelExpr *relExpr)
: Statement(relExpr->loc), rel_expr(relExpr) {}

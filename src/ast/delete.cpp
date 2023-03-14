#include "psql_parse/ast/delete.hpp"

namespace psql_parse {

    DeleteStatement::DeleteStatement(box<QualifiedName> tableName, bool only)
    : table_name(std::move(tableName)), only(only) {}
}
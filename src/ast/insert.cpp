#include "psql_parse/ast/insert.hpp"

namespace psql_parse {

    InsertStatement::InsertStatement(box<QualifiedName> tableName)
    : table_name(std::move(tableName)) {}

}


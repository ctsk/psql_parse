#pragma once

#include "expr.hpp"

namespace psql_parse {
    struct DeleteStatement {
        DEFAULT_EQ(DeleteStatement);

        box<QualifiedName> table_name;
        bool only;
        std::optional<Expression> where;

        explicit DeleteStatement(box<QualifiedName> tableName, bool only);
    };
}
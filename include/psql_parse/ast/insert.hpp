#pragma once

#include "expr.hpp"

namespace psql_parse {
    struct InsertStatement {

        enum class Override {
            USER_VALUE,
            SYSTEM_VALUE
        };

        struct Default {
            DEFAULT_EQ(Default);
        };

        DEFAULT_EQ(InsertStatement);

        box<QualifiedName> table_name;
        std::vector<Name> column_names;
        std::optional<Override> override;
        std::variant<Default, box<Query>> source;

        explicit InsertStatement(box<QualifiedName> tableName);
    };
}
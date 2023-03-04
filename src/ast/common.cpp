//
// Created by christian on 04.03.23.
//

#include "psql_parse/ast/common.hpp"

namespace psql_parse {
    QualifiedName::QualifiedName(Name name)
    : qualifier(), name(std::move(name)) {}
}
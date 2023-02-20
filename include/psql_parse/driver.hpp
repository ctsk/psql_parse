#pragma once

#include <memory>
#include <vector>
#include <optional>

#include "psql_parse/scanner.hpp"
#include "psql_parse/ast/expr.h"

namespace psql_parse {

    class driver {
        friend class parser;

        std::unique_ptr<scanner> scanner_;

        bool trace_scanning_;
        bool trace_parsing_;

        std::unique_ptr<Statement> result_;

        [[maybe_unused]] static void error(const psql_parse::location&, const std::string&);

    public:
        driver();

        bool parse(std::istream& in);

        std::unique_ptr<Statement> getResult();
    };
}
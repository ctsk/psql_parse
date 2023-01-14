#pragma once

#include <memory>
#include <vector>
#include <optional>

#include "psql_parse/scanner.hpp"

namespace psql_parse {

    class driver {
        friend class parser;

        std::unique_ptr<scanner> scanner_;

        bool trace_scanning_;
        bool trace_parsing_;


        [[maybe_unused]] void error(const psql_parse::location&, const std::string&);

    public:
        driver();


        std::vector<int> result_;

        bool parse(std::istream& in);
    };
}
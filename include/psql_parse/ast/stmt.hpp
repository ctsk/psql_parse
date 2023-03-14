#pragma once

#include <memory>

#include "create.hpp"
#include "insert.hpp"
#include "select.hpp"

namespace psql_parse {
	using Statement = std::variant<
	        box<CreateStatement>,
            box<InsertStatement>,
			box<SelectStatement>>;
}
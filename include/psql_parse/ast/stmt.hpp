#pragma once

#include <memory>

#include "create.hpp"
#include "delete.hpp"
#include "insert.hpp"
#include "select.hpp"

namespace psql_parse {
	using Statement = std::variant<
	        box<CreateStatement>,
            box<InsertStatement>,
            box<DeleteStatement>,
			box<SelectStatement>>;
}
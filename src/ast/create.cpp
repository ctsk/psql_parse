//
// Created by christian on 17.02.23.
//

#include <string>

#include "psql_parse/ast/create.h"

namespace psql_parse {

	CreateStatement::CreateStatement(location loc, QualifiedName relName, std::optional<Temporary> temp, std::optional<OnCommit> onCommit)
	: Statement(loc), rel_name(std::move(relName)), temp(temp), on_commit(onCommit) {
		if (temp.has_value() && !onCommit.has_value()) {
			on_commit = OnCommit::DELETE;
		}
	}

}

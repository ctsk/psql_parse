#include <string>

#include "psql_parse/ast/create.hpp"

namespace psql_parse {

	CreateStatement::CreateStatement(location loc, QualifiedName relName, std::optional<Temporary> temp,
									 std::optional<OnCommit> onCommit,
									 std::vector<std::variant<ColumnDef, TableConstraint>> elements)
	: Statement(loc), rel_name(std::move(relName)), temp(temp), on_commit(onCommit), column_defs(), table_constraints() {
		if (temp.has_value() && !onCommit.has_value()) {
			on_commit = OnCommit::DELETE;
		}

		for (auto &elem : elements) {
			if (std::holds_alternative<ColumnDef>(elem)) {
				column_defs.push_back(std::move(std::get<ColumnDef>(elem)));
			} else {
				table_constraints.push_back(std::move(std::get<TableConstraint>(elem)));
			}
		}
	}

	CreateStatement::CreateStatement(location loc, QualifiedName relName)
	: Statement(loc)
	, rel_name(std::move(relName))
	, temp(std::nullopt)
	, on_commit(std::nullopt)
	, column_defs()
	, table_constraints() { }

	bool CreateStatement::equals(const CreateStatement &other) const {
		return other.rel_name == rel_name
			   && other.temp == temp
			   && other.on_commit == on_commit
			   && column_defs == column_defs
			   && table_constraints == table_constraints;
	}


	ColumnDef::ColumnDef() = default;

	ColumnDef::ColumnDef(Name name, std::variant<DataType, DomainName> type)
	: name(std::move(name)), type(std::move(type)) {}
}

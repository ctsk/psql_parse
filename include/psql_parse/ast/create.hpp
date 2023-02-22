#pragma once

#include <optional>
#include <variant>
#include <vector>

#include "psql_parse/ast/stmt.hpp"

namespace psql_parse {

	enum class Temporary {
		GLOBAL,
		LOCAL
	};

	enum class OnCommit {
		DELETE,
		PRESERVE
	};

	enum class UserSpec {
		CURRENT_USER,
		SESSION_USER,
		SYSTEM_USER,
	};

	using ColumnDefault = std::variant<
			UserSpec,
			V_NULL,
			std::unique_ptr<Expression>>;

	enum class ReferentialAction {
		CASCADE,
		SET_NULL,
		SET_DEFAULT,
		NO_ACTION
	};

	struct ReferentialTriggeredAction {
		ReferentialAction on_delete, on_update;
	};

	enum class MatchOption {
		FULL,
		PARTIAL,
		NONE
	};

	enum class ConstraintType {
		NOT_NULL,
		UNIQUE,
		PRIMARY_KEY
	};

	struct References {
		QualifiedName rel_name;
		std::vector<Name> col_names;
		MatchOption match_type;
		ReferentialTriggeredAction action;
	};

	using ColumnConstraint = std::variant<
			ConstraintType,
			std::unique_ptr<References>>;

	struct NamedColumnConstraint {
		DEFAULT_SPACESHIP(NamedColumnConstraint);
		std::optional<QualifiedName> name;
		std::variant<
				ConstraintType,
				std::unique_ptr<References>> constraint;
	};

	struct ColumnDef {
		DEFAULT_SPACESHIP(ColumnDef);

		Name name;
		std::variant<DataType, DomainName> type;
		std::optional<ColumnDefault> col_default;
		std::vector<NamedColumnConstraint> col_constraint;
		std::optional<QualifiedName> collate;
	};

	struct TableUniqueConstraint {
		DEFAULT_SPACESHIP(TableUniqueConstraint);
		std::vector<Name> column_names;
	};
	struct TablePrimaryKeyConstraint {
		DEFAULT_SPACESHIP(TablePrimaryKeyConstraint);
		std::vector<Name> column_names;
	};
	struct TableForeignKeyConstraint {
		DEFAULT_SPACESHIP(TableForeignKeyConstraint);
		std::vector<Name> column_names; std::unique_ptr<References> references;
	};

	using TableConstraint = std::variant<
			TableUniqueConstraint,
			TablePrimaryKeyConstraint,
			TableForeignKeyConstraint>;

	struct CreateStatement: public Statement {
		QualifiedName rel_name;
		std::optional<Temporary> temp;
		std::optional<OnCommit> on_commit;
		std::vector<ColumnDef> column_defs;
		std::vector<TableConstraint> table_constraints;

		CreateStatement(location loc, QualifiedName relName);
		CreateStatement(location loc, QualifiedName relName, std::optional<Temporary> temp, std::optional<OnCommit> onCommit, std::vector<std::variant<ColumnDef, TableConstraint>> elements);

		bool equals(const CreateStatement& other) {
			return other.rel_name == rel_name
				   && other.temp == temp
				   && other.on_commit == on_commit
			       && column_defs == column_defs
			       && table_constraints == table_constraints;
		}
	};
}



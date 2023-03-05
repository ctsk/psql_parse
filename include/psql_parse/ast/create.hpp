#pragma once

#include <optional>
#include <variant>
#include <vector>

#include "psql_parse/ast/data_types.hpp"
#include "psql_parse/ast/expr.hpp"

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
			Expression>;

	enum class ReferentialAction {
		CASCADE,
		SET_NULL,
		SET_DEFAULT,
		NO_ACTION
	};

	struct ReferentialTriggeredAction {
		DEFAULT_EQ(ReferentialTriggeredAction);
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
		DEFAULT_EQ(References);

		box<QualifiedName> rel_name;
		std::vector<Name> col_names;
		MatchOption match_type;
		ReferentialTriggeredAction action;
	};

	using ColumnConstraint = std::variant<
			ConstraintType,
			box<References>>;

	struct NamedColumnConstraint {
		DEFAULT_EQ(NamedColumnConstraint);
		std::optional<box<QualifiedName>> name = std::nullopt;
		std::variant<
				ConstraintType,
				box<References>> constraint;
	};

	struct ColumnDef {
		DEFAULT_EQ(ColumnDef);

		Name name;
        DataType type;
		std::optional<ColumnDefault> col_default;
		std::vector<NamedColumnConstraint> col_constraint;
		std::optional<box<QualifiedName>> collate;

		// required by bison
		ColumnDef();

		ColumnDef(Name name, DataType type);
	};

	struct TableUniqueConstraint {
		DEFAULT_EQ(TableUniqueConstraint);
		std::vector<Name> column_names;
	};
	struct TablePrimaryKeyConstraint {
		DEFAULT_EQ(TablePrimaryKeyConstraint);
		std::vector<Name> column_names;
	};
	struct TableForeignKeyConstraint {
		DEFAULT_EQ(TableForeignKeyConstraint);
		std::vector<Name> column_names;
		box<References> references;
	};

	using TableConstraint = std::variant<
			TableUniqueConstraint,
			TablePrimaryKeyConstraint,
			TableForeignKeyConstraint>;

	struct CreateStatement {
		box<QualifiedName> rel_name;
		std::optional<Temporary> temp = std::nullopt;
		std::optional<OnCommit> on_commit = std::nullopt;
		std::vector<ColumnDef> column_defs;
		std::vector<TableConstraint> table_constraints;

		explicit CreateStatement(box<QualifiedName> relName);
		CreateStatement(box<QualifiedName> relName, std::optional<Temporary> temp, std::optional<OnCommit> onCommit, std::vector<std::variant<ColumnDef, TableConstraint>> elements);
	};
}



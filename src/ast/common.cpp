//
// Created by christian on 18.02.23.
//

#include "psql_parse/ast/common.h"

#include <string>

struct ColumnDef {
	std::string colname;

};

struct TableConstraint {

};

struct PrimaryKeyDef : public TableConstraint {

};

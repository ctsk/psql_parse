%skeleton "lalr1.cc"
%require "3.2"

%code top {

// ----------
// PSQL PARSE
// ----------

}

%defines
%define api.parser.class {parser}
%define api.namespace    { psql_parse }
%define api.value.type   variant
%define api.token.raw
%define api.token.constructor
%define api.token.prefix {TOK_}
%define parse.assert
%define parse.trace
%define parse.error verbose

%locations

%parse-param { psql_parse::driver& driver }

%code requires {

#include <memory>
#include <string>
#include <vector>

#include "psql_parse/ast/expr.h"
#include "psql_parse/ast/stmt.h"
#include "psql_parse/ast/common.h"
#include "psql_parse/ast/create.h"

namespace psql_parse {
class driver;
}

}

%code {

#include "psql_parse/driver.hpp"

#undef yylex
#define yylex driver.scanner_->lex

}

%token END 0 "end of file"

%token <int>		INTEGER_VALUE	"integer_value"
%token <double>		FLOAT_VALUE	"float_value"
%token <std::string>	IDENTIFIER	"identifier"
%token <std::string>	STRING_VALUE	"string"
%token <std::string>	BIT_VALUE	"bit_string"
%token <std::string>	HEX_VALUE	"hex_string"
%token <std::string>	NATIONAL_VALUE	"nat_string"
%token 			LP		"("
%token 			RP		")"
%token 			DOT		"."
%token 			COMMA		","
%token 			SEMICOLON	";"
%token 			PLUS		"+"
%token 			MINUS		"-"
%token 			QUOTE		"'"

%token 	ACTION BIT CASCADE CHARACTER COLLATE CONSTRAINT CREATE CURRENT_USER DECIMAL DEFAULT DELETE DOUBLE FLOAT
	FOREIGN FULL INTEGER KEY MATCH NATIONAL NO NOT NCHAR NULL NUMERIC ON PARTIAL PRECISION PRIMARY REAL
	REFERENCES ROWS SESSION_USER SET SMALLINT SYSTEM_USER TABLE UNIQUE UPDATE USER VARCHAR VARYING


%type <std::unique_ptr<psql_parse::Statement>>		pseudo_start
%type <std::unique_ptr<psql_parse::Expression>>		numeric_literal
%type <std::unique_ptr<psql_parse::NumberLiteral>>	signed_numeric_literal
%type <std::unique_ptr<psql_parse::NumberLiteral>>	unsigned_numeric_literal
%type <std::unique_ptr<psql_parse::Expression>>		general_literal
%type <std::unique_ptr<psql_parse::Statement>>		ExprStatement
%type <std::unique_ptr<psql_parse::Statement>>		CreateStatement


%type <DataType>					data_type
%type <std::optional<uint64_t>>				precision_spec
%type <std::pair<std::optional<uint64_t>, uint64_t>>	precision_scale_spec
%type <uint64_t>					opt_length_spec
%type <uint64_t>					length_spec

%type <std::vector<Name>>				identifier_list
%type <std::vector<Name>>				opt_identifier_list
%type <QualifiedName>					qualified_name

%type <ColumnDef>					column_def
%type <std::variant<DataType, DomainName>>		column_type
%type <std::optional<ColumnDefault>>			opt_default_clause
%type <ColumnDefault>					default_clause
%type <ColumnDefault>					default_option
%type <std::vector<NamedColumnConstraint>>		opt_column_constraint_def
%type <std::vector<NamedColumnConstraint>>		column_constraint_defs
%type <NamedColumnConstraint>				column_constraint_def
%type <std::optional<QualifiedName>>			opt_constraint_name
%type <ColumnConstraint>				column_constraint
%type <std::unique_ptr<References>>			references_spec
%type <std::pair<QualifiedName, std::vector<Name>>>	ref_table_cols
%type <MatchOption>					opt_match
%type <MatchOption>					match_type
%type <ReferentialTriggeredAction>			opt_referential_triggered_action
%type <ReferentialTriggeredAction>			referential_triggered_action
%type <ReferentialAction>				referential_action
%type <ReferentialAction>				update_rule
%type <ReferentialAction>				delete_rule
%type <std::optional<QualifiedName>>			opt_collate_clause
%type <TableConstraint>					table_constraint_def


%%

%start pseudo_start;

pseudo_start:
    ExprStatement	{ std::swap($ExprStatement, driver.result_); }
 |  CreateStatement	{ std::swap($CreateStatement, driver.result_); }
 ;

ExprStatement:
    numeric_literal	{ $$ = std::make_unique<ExprStatement>(@$, std::move($numeric_literal)); }
    ;

/*
 *   Basics building blocks
 */

literal:
    numeric_literal
 |  general_literal
 ;

numeric_literal:
    signed_numeric_literal
 |  unsigned_numeric_literal

signed_numeric_literal:
    PLUS unsigned_numeric_literal	{ std::swap($$, $unsigned_numeric_literal); }
 |  MINUS unsigned_numeric_literal	{ $unsigned_numeric_literal->negate(); std::swap($$, $unsigned_numeric_literal); }
 ;

unsigned_numeric_literal:
    INTEGER_VALUE	{ $$ = std::make_unique<IntegerLiteral>(@INTEGER_VALUE, $INTEGER_VALUE); }
 |  FLOAT_VALUE		{ $$ = std::make_unique<FloatLiteral>(@FLOAT_VALUE, $FLOAT_VALUE); }
 ;

general_literal:
    STRING_VALUE	{ $$ = std::make_unique<StringLiteral>(@STRING_VALUE, $STRING_VALUE, StringLiteralType::CHAR); }
 |  BIT_VALUE		{ $$ = std::make_unique<StringLiteral>(@BIT_VALUE, $BIT_VALUE, StringLiteralType::BIT); }
 |  HEX_VALUE		{ $$ = std::make_unique<StringLiteral>(@HEX_VALUE, $HEX_VALUE, StringLiteralType::HEX); }
 |  NATIONAL_VALUE	{ $$ = std::make_unique<StringLiteral>(@NATIONAL_VALUE, $NATIONAL_VALUE, StringLiteralType::NATIONAL); }

data_type:
    DECIMAL precision_scale_spec[spec]		{ $$ = DecimalType { $spec.first, $spec.second }; }
 |  FLOAT precision_spec[spec]			{ $$ = FloatType { $spec }; }
 |  INTEGER 					{ $$ = IntegerType { }; }
 |  NUMERIC precision_scale_spec[spec]		{ $$ = NumericType { $spec.first, $spec.second }; }
 |  SMALLINT 					{ $$ = SmallIntType { }; }
 |  REAL	 				{ $$ = RealType { }; }
 |  DOUBLE PRECISION				{ $$ = DoublePrecisionType { }; }
 |  CHARACTER opt_length_spec 			{ $$ = CharType { $opt_length_spec }; }
 |  CHARACTER VARYING length_spec		{ $$ = VarCharType { $length_spec }; }
 |  NATIONAL CHARACTER opt_length_spec		{ $$ = NationalCharType { $opt_length_spec }; }
 |  NATIONAL CHARACTER VARYING length_spec	{ $$ = NationalVarCharType { $length_spec }; }
 |  NCHAR opt_length_spec			{ $$ = NationalCharType { $opt_length_spec }; }
 |  NCHAR VARYING length_spec			{ $$ = NationalVarCharType { $length_spec }; }
 |  BIT opt_length_spec				{ $$ = Bit { $opt_length_spec }; }
 |  BIT VARYING length_spec			{ $$ = VarBit { $length_spec }; }
 ;

precision_scale_spec:
    LP INTEGER_VALUE[precision] COMMA INTEGER_VALUE[scale] RP 	{ $$ = std::make_pair($precision, $scale); }
 |  LP INTEGER_VALUE[precision] RP				{ $$ = std::make_pair($precision, 0); }
 |  %empty							{ $$ = std::make_pair(std::nullopt, 0); }
 ;

precision_spec:
    LP INTEGER_VALUE RP { $$ = $INTEGER_VALUE; }
 |  %empty		{ $$ = std::nullopt; }
 ;

opt_length_spec:
    length_spec		{ $$ = $length_spec; }
 |  %empty		{ $$ = 1; }
 ;

length_spec: LP INTEGER_VALUE RP { $$ = $INTEGER_VALUE; };

integer_list:
    INTEGER_VALUE COMMA integer_list
 |  INTEGER_VALUE
 ;

/*
 *   Handling Identifiers
 */

opt_identifier_list:
    identifier_list	{ std::swap($$, $identifier_list); }
 |  %empty		{ $$ = std::vector<Name>(); }
 ;

identifier_list:
    IDENTIFIER[elem] COMMA identifier_list[vec]	{ $vec.push_back(std::move($elem)); std::swap($$, $vec); }
 |  IDENTIFIER[elem]				{ $$ = std::vector<Name> { std::move($elem) }; }
 ;

qualified_name:
    IDENTIFIER[catalog] DOT IDENTIFIER[schema] DOT IDENTIFIER[name]	{ $$ = QualifiedName { $catalog, $schema, $name}; }
 |  IDENTIFIER[schema] DOT IDENTIFIER[name]				{ $$ = QualifiedName { std::nullopt, $schema, $name}; }
 |  IDENTIFIER[name]							{ $$ = QualifiedName { std::nullopt, std::nullopt, $name}; }
 ;

/*
 *   CREATE TABLE <name> ...
 */
CreateStatement:
    CREATE TABLE IDENTIFIER LP column_defs_and_constraints_or_empty RP { }
    //{ $$ = std::make_unique<CreateStatement>(@$, $IDENTIFIER); }
    ;

column_defs_and_constraints_or_empty:
    column_defs_and_constraints		{}
 |  %empty				{}
 ;

column_defs_and_constraints:
    column_def "," column_defs_and_constraints			{ }
 |  table_constraint_def "," column_defs_and_constraints	{ }
 |  column_def							{ }
 |  table_constraint_def					{ }
 ;

column_def:
    IDENTIFIER[column_name] column_type opt_default_clause opt_column_constraint_def opt_collate_clause
    { $$ = ColumnDef { $column_name, $column_type, $opt_default_clause, std::move($opt_column_constraint_def), $opt_collate_clause}; }
 ;

column_type:
    data_type				{ $$ = $data_type; }
 |  qualified_name[domain_name]		{ $$ = $domain_name; }
 ;

opt_default_clause:
    default_clause		{ $$ = $default_clause; }
 |  %empty			{ $$ = std::nullopt; }
 ;

default_clause:
    DEFAULT default_option	{ $$ = $default_option; }
 ;

default_option:
 /*  literal
 | datetime value function
 |*/  USER		{ $$ = ColumnDefault::CURRENT_USER; }
 |  CURRENT_USER	{ $$ = ColumnDefault::CURRENT_USER; }
 |  SESSION_USER	{ $$ = ColumnDefault::SESSION_USER; }
 |  SYSTEM_USER		{ $$ = ColumnDefault::SYSTEM_USER; }
 |  NULL		{ $$ = ColumnDefault::NUL; }
 ;

opt_column_constraint_def:
    column_constraint_defs	{ $$ = std::move($column_constraint_defs); }
 |  %empty			{ $$ = std::vector<NamedColumnConstraint>(); }
 ;

column_constraint_defs:
    column_constraint_def[elem] column_constraint_defs[vec]	{ $vec.push_back(std::move($elem)); std::swap($vec, $$); }
 |  column_constraint_def[elem]
   { $$ = std::vector<NamedColumnConstraint>(); $$.push_back(std::move($elem)); }
 ;

column_constraint_def:
    opt_constraint_name column_constraint /* opt_constraint_attributes */
    { $$ = NamedColumnConstraint { $opt_constraint_name, std::move($column_constraint) }; }
 ;

opt_constraint_name:
    CONSTRAINT qualified_name 	{ $$ = $qualified_name; }
 |  %empty			{ $$ = std::nullopt; }
 ;

column_constraint:
    NOT NULL		{ $$ = ConstraintType::NOT_NULL; }
 |  UNIQUE		{ $$ = ConstraintType::UNIQUE; }
 |  PRIMARY KEY		{ $$ = ConstraintType::PRIMARY_KEY; }
 |  references_spec	{ $$ = std::move($references_spec); }
/* |  check_constraint_def */
 ;

references_spec:
    REFERENCES ref_table_cols opt_match opt_referential_triggered_action
    { $$ = std::make_unique<References>($ref_table_cols.first, std::move($ref_table_cols.second), $opt_match, $opt_referential_triggered_action); }
 ;

ref_table_cols:
    qualified_name[table_name] LP identifier_list RP	{ $$ = std::make_pair($table_name, $identifier_list); }
 ;

opt_match:
    MATCH match_type	{ $$ = $match_type; }
 |  %empty		{ $$ = MatchOption::NONE; }
 ;

match_type:
    FULL	{ $$ = MatchOption::FULL; }
 |  PARTIAL	{ $$ = MatchOption::PARTIAL; }
 ;

opt_referential_triggered_action:
    referential_triggered_action	{ $$ = $referential_triggered_action; }
 |  %empty				{ $$ = ReferentialTriggeredAction { .on_delete = ReferentialAction::NO_ACTION, .on_update = ReferentialAction::NO_ACTION }; }
 ;

referential_triggered_action:
    update_rule delete_rule	{ $$ = ReferentialTriggeredAction { .on_delete = $delete_rule, .on_update = $update_rule }; }
 |  delete_rule update_rule	{ $$ = ReferentialTriggeredAction { .on_delete = $delete_rule, .on_update = $update_rule }; }
 |  update_rule			{ $$ = ReferentialTriggeredAction { .on_delete = ReferentialAction::NO_ACTION, .on_update = $update_rule }; }
 |  delete_rule			{ $$ = ReferentialTriggeredAction { .on_delete = $delete_rule, .on_update = ReferentialAction::NO_ACTION }; }
 ;

referential_action:
    CASCADE		{ $$ = ReferentialAction::CASCADE; }
 |  SET NULL		{ $$ = ReferentialAction::SET_NULL; }
 |  SET DEFAULT		{ $$ = ReferentialAction::SET_DEFAULT; }
 |  NO ACTION		{ $$ = ReferentialAction::NO_ACTION; }
 ;

update_rule:
    ON UPDATE referential_action { $$ = $referential_action; }
 ;

delete_rule:
    ON DELETE referential_action { $$ = $referential_action; }
 ;

opt_collate_clause:
    COLLATE qualified_name	{ $$ = std::move($qualified_name); }
 |  %empty			{ $$ = std::nullopt; }
 ;

table_constraint_def:
    UNIQUE LP identifier_list RP			{ $$ = TableUniqueConstraint($identifier_list); }
 |  PRIMARY KEY LP identifier_list RP			{ $$ = TablePrimaryKeyConstraint($identifier_list); }
 |  FOREIGN KEY LP identifier_list RP references_spec	{ $$ = TableForeignKeyConstraint($identifier_list, std::move($references_spec)); }
/* |  check_constraint_definition */
 ;

%%

void psql_parse::parser::error(const location_type& l, const std::string& m)
{
    driver.error(l, m);
}

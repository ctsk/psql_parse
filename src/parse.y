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
#include <algorithm>
#include <string>
#include <vector>

#include "psql_parse/ast/expr.hpp"
#include "psql_parse/ast/stmt.hpp"
#include "psql_parse/ast/common.hpp"
#include "psql_parse/ast/create.hpp"
#include "psql_parse/ast/select.hpp"

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
%token 			SLASH		"/"
%token 			STAR		"*"
%token 			MINUS		"-"
%token 			QUOTE		"'"

%token 	ACTION ALL AND BIT CASCADE CHARACTER COLLATE COMMIT CONSTRAINT
	CREATE CURRENT_USER DATE DECIMAL DEFAULT DELETE DISTINCT DOUBLE
	FLOAT FOREIGN FULL GLOBAL INTEGER KEY LOCAL MATCH
	NATIONAL NO NOT NCHAR NULL NUMERIC ON OR PARTIAL PRECISION
	PRESERVE PRIMARY REAL REFERENCES ROWS SESSION_USER SET
	SMALLINT SELECT SYSTEM_USER TABLE TEMPORARY TIMESTAMP TIME
	UNIQUE UPDATE USER VARCHAR VARYING WITH ZONE

%left PLUS MINUS AND OR

%type <Statement*>		pseudo_start
%type <ValExpr*>		numeric_literal
%type <ValExpr*>		signed_numeric_literal
%type <ValExpr*>		unsigned_numeric_literal
%type <StringLiteral*>		general_literal
%type <ValExpr*>		literal
%type <Statement*>		ExprStatement
%type <Statement*>		CreateStatement


%type <DataType>					data_type
%type <bool>						opt_with_time_zone
%type <uint64_t>					opt_time_precision
%type <uint64_t>					opt_timestamp_precision
%type <std::optional<uint64_t>>				precision_spec
%type <std::pair<std::optional<uint64_t>, uint64_t>>	precision_scale_spec
%type <uint64_t>					opt_length_spec
%type <uint64_t>					length_spec

%type <std::vector<Name>>				identifier_list
%type <QualifiedName>					qualified_name
%type <std::optional<Temporary>>			opt_temporary
%type <std::optional<OnCommit>>				opt_on_commit
%type <std::vector<std::variant<ColumnDef, TableConstraint>>> column_defs_and_constraints
%type <std::variant<ColumnDef, TableConstraint>>	column_def_and_constraint
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
%type <References*>					references_spec
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

%type <Expression*>					expr
%type <ValExpr*>					value_expr

%type <SelectStatement*>				SelectStatement
%type <SelectStatement*>				select_no_parens
%type <SelectStatement*>				select_with_parens
%type <SelectStatement*>				simple_select
%type <std::optional<SetQuantifier>>			opt_set_quantifier

%type <std::vector<std::unique_ptr<ValExpr>>>		target_list
%type <ValExpr*>					target_element


%%

%start pseudo_start;

pseudo_start:
    ExprStatement	{ driver.result_ = $ExprStatement; }
 |  CreateStatement	{ driver.result_ = $CreateStatement; }
 |  SelectStatement	{ driver.result_ = $SelectStatement; }
 ;

ExprStatement: expr	{ $$ = new ExprStatement(@$, $expr); } ;

/*
 *   Basics building blocks
 */

literal: numeric_literal | general_literal ;

numeric_literal: signed_numeric_literal	| unsigned_numeric_literal ;

signed_numeric_literal:
    PLUS unsigned_numeric_literal	{ $$ = $unsigned_numeric_literal; }
 |  MINUS unsigned_numeric_literal	{ $$ = new UnaryOp(@$, UnaryOp::Op::NEG, $unsigned_numeric_literal); }
 ;

unsigned_numeric_literal:
    INTEGER_VALUE	{ $$ = new IntegerLiteral(@INTEGER_VALUE, $INTEGER_VALUE); }
 |  FLOAT_VALUE		{ $$ = new FloatLiteral(@FLOAT_VALUE, $FLOAT_VALUE); }
 ;

general_literal:
    STRING_VALUE	{ $$ = new StringLiteral(@STRING_VALUE, std::move($STRING_VALUE), StringLiteralType::CHAR); }
 |  BIT_VALUE		{ $$ = new StringLiteral(@BIT_VALUE, std::move($BIT_VALUE), StringLiteralType::BIT); }
 |  HEX_VALUE		{ $$ = new StringLiteral(@HEX_VALUE, std::move($HEX_VALUE), StringLiteralType::HEX); }
 |  NATIONAL_VALUE	{ $$ = new StringLiteral(@NATIONAL_VALUE, std::move($NATIONAL_VALUE), StringLiteralType::NATIONAL); }

/* Missing: INTERVAL */
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
 |  DATE					{ $$ = DateType { }; }
 |  TIME opt_time_precision opt_with_time_zone			{ $$ = TimeType { $opt_time_precision, $opt_with_time_zone }; }
 |  TIMESTAMP opt_timestamp_precision opt_with_time_zone	{ $$ = TimeType { $opt_timestamp_precision, $opt_with_time_zone }; }
 ;

opt_with_time_zone:
    WITH TIME ZONE	{ $$ = true; }
 |  %empty		{ $$ = false; }
 ;

opt_time_precision:
    LP INTEGER_VALUE[time_precision] RP		{ $$ = $time_precision; }
 |  %empty					{ $$ = 0; }
 ;

opt_timestamp_precision:
    LP INTEGER_VALUE[timestamp_precision] RP	{ $$ = $timestamp_precision; }
 |  %empty					{ $$ = 6; }
 ;

precision_scale_spec:
    LP INTEGER_VALUE[precision] COMMA INTEGER_VALUE[scale] RP 	{ $$ = std::make_pair($precision, $scale); }
 |  LP INTEGER_VALUE[precision] RP				{ $$ = std::make_pair($precision, 0); }
 |  %empty							{ $$ = std::make_pair(std::nullopt, 0); }
 ;

precision_spec:
    LP INTEGER_VALUE RP
 |  %empty		{ $$ = std::nullopt; }
 ;

opt_length_spec:
    length_spec
 |  %empty		{ $$ = 1; }
 ;

length_spec: LP INTEGER_VALUE RP { $$ = $INTEGER_VALUE; };

/*
 *   Handling Identifiers
 */

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
    CREATE opt_temporary TABLE qualified_name[table_name] LP column_defs_and_constraints[table_elems] RP opt_on_commit
    { $$ = new CreateStatement(@$, $table_name, $opt_temporary, $opt_on_commit, std::move($table_elems)); }
    ;

opt_temporary:
    LOCAL TEMPORARY	{ $$ = Temporary::LOCAL; }
 |  GLOBAL TEMPORARY    { $$ = Temporary::GLOBAL; }
 |  %empty		{ $$ = std::nullopt; }
 ;

opt_on_commit:
    ON COMMIT DELETE ROWS   { $$ = OnCommit::DELETE; }
 |  ON COMMIT PRESERVE ROWS { $$ = OnCommit::PRESERVE; }
 |  %empty		    { $$ = std::nullopt; }
 ;

column_defs_and_constraints:
    column_def_and_constraint[elem] COMMA column_defs_and_constraints[vec]
    { $vec.push_back(std::move($elem)); std::swap($vec, $$); }
 |  column_def_and_constraint[elem]
    { $$ = std::vector<std::variant<ColumnDef, TableConstraint>>(); $$.push_back(std::move($elem)); }
 ;

column_def_and_constraint:
    column_def			{ $$ = std::move($column_def); }
 |  table_constraint_def	{ $$ = std::move($table_constraint_def); }
 ;

column_def:
    IDENTIFIER[column_name] column_type opt_default_clause opt_column_constraint_def opt_collate_clause
    { $$ = ColumnDef($column_name, $column_type);
      $$.col_default = std::move($opt_default_clause);
      $$.col_constraint = std::move($opt_column_constraint_def);
      $$.collate = $opt_collate_clause; }
 ;

column_type:
    data_type				{ $$ = $data_type; }
 |  qualified_name[domain_name]		{ $$ = $domain_name; }
 ;

opt_default_clause:
    default_clause		{ $$ = std::move($default_clause); }
 |  %empty			{ $$ = std::nullopt; }
 ;

default_clause:
    DEFAULT default_option	{ $$ = std::move($default_option); }
 ;

default_option:
   literal		{ $$ = std::unique_ptr<Expression>($literal); }
 /*| datetime value function */
 |  USER		{ $$ = UserSpec::CURRENT_USER; }
 |  CURRENT_USER	{ $$ = UserSpec::CURRENT_USER; }
 |  SESSION_USER	{ $$ = UserSpec::SESSION_USER; }
 |  SYSTEM_USER		{ $$ = UserSpec::SYSTEM_USER; }
 |  NULL		{ $$ = V_NULL { }; }
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
 |  references_spec	{ $$ = std::unique_ptr<References>($references_spec); }
/* |  check_constraint_def */
 ;

references_spec:
    REFERENCES ref_table_cols opt_match opt_referential_triggered_action
    { $$ = new References($ref_table_cols.first, std::move($ref_table_cols.second), $opt_match, $opt_referential_triggered_action); }
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
 |  FOREIGN KEY LP identifier_list RP references_spec	{ $$ = TableForeignKeyConstraint($identifier_list, std::unique_ptr<References>($references_spec)); }
/* |  check_constraint_definition */
 ;

/*
 *  Expressions
 */

expr:
     value_expr { $$ = $1; }
  ;

value_expr:
     literal
  |  value_expr[left] OR  value_expr[right]		{ $$ = new BinaryOp(@$, $left, BinaryOp::Op::OR, $right); }
  |  value_expr[left] AND  value_expr[right]		{ $$ = new BinaryOp(@$, $left, BinaryOp::Op::AND, $right); }
  |  value_expr[left] PLUS  value_expr[right]		{ $$ = new BinaryOp(@$, $left, BinaryOp::Op::ADD, $right); }
  |  value_expr[left] MINUS value_expr[right]		{ $$ = new BinaryOp(@$, $left, BinaryOp::Op::SUB, $right); }
  ;


/************
 *          *
 *  SELECT  *
 *          *
 ************/

SelectStatement:
    select_no_parens
 |  select_with_parens
 ;

select_with_parens:
    LP select_no_parens RP		{ $$ = $select_no_parens; }
 |  LP select_with_parens[inner] RP	{ $$ = $inner; }
 ;

select_no_parens:
    simple_select
 ;

simple_select:
    SELECT opt_set_quantifier target_list 		{ $$ = new SelectStatement(@$);
    							  std::reverse($target_list.begin(), $target_list.end());
    						  	  $$->target_list = std::move($target_list);
    						  	  $$->set_quantifier = $opt_set_quantifier; }
 ;

opt_set_quantifier:
    ALL		{ $$ = SetQuantifier::ALL; }
 |  DISTINCT	{ $$ = SetQuantifier::DISTINCT; }
 |  %empty	{ $$ = std::nullopt; }
 ;

target_list:
    target_element				{ $$ = std::vector<std::unique_ptr<ValExpr>>(); $$.emplace_back($target_element); }
 |  target_element COMMA target_list[list]	{ $list.emplace_back($target_element); std::swap($$, $list); }
 ;

target_element:
    value_expr
 |  STAR  				{ $$ = nullptr; }
 ;

%%

void psql_parse::parser::error(const location_type& l, const std::string& m)
{
    driver.error(l, m);
}

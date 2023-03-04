%skeleton "lalr1.cc"
%require "3.2"

%code top {

// ----------
// PSQL PARSE
// ----------

}

%header

%define api.parser.class {parser}
%define api.namespace    { psql_parse }
%define api.value.type   variant
%define api.value.automove
%define api.token.raw
%define api.token.constructor
%define api.token.prefix {TOK_}
%define parse.assert
%define parse.trace
%define parse.error verbose

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

#define mkNode driver.nf.node
#define mkNotNode driver.nf.notNode

namespace psql_parse {
    auto applyOptAlias(std::optional<box<TableAlias>> alias, RelExpression expr) -> RelExpression {
	if (alias.has_value()) {
	    alias.value()->expression = std::move(expr);
	    return std::move(alias.value());
	} else {
	    return expr;
	}
    }
}

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
%token 			EQUAL		"="
%token 			NOT_EQUAL	"<>"
%token 			LESS_EQUAL	"<="
%token 			LESS		"<"
%token 			GREATER_EQUAL	">="
%token 			GREATER		">"
%token 			QUOTE		"'"

%token 	ACTION ALL AND ASYMMETRIC ASC AS BETWEEN BIT
	BY CASCADE CHARACTER COLLATE COMMIT CONSTRAINT
	CREATE CROSS CUBE CURRENT_USER CURRENT DATE
	DECIMAL DEFAULT DELETE DESC DISTINCT DOUBLE
	EXCEPT EXCLUDE FALSE FETCH FIRST FLOAT FOLLOWING
	FOREIGN FROM FULL GLOBAL GROUPING GROUPS GROUP
	HAVING INNER INTEGER INTERSECT IN JOIN KEY LAST
	LEFT LOCAL MATCH NATIONAL NATURAL NCHAR NEXT NOT NO
	NULLS NULL NUMERIC OFFSET ONLY ON ORDER OR OTHERS OUTER PARTIAL
	PARTITION PERCENT PRECEDING PRECISION PRESERVE PRIMARY
	RANGE REAL REFERENCES RIGHT ROLLUP ROWS ROW
	SESSION_USER SETS SET SMALLINT SELECT SYMMETRIC
	SYSTEM_USER TABLE TEMPORARY TIES TIMESTAMP TIME
	TRUE UNBOUNDED UNION UNIQUE UNKNOWN UPDATE USER
	USING VARCHAR VARYING WHERE WINDOW WITH ZONE

%left UNION EXCEPT
%left INTERSECT
%left OR
%left AND
%right NOT
%nonassoc IS
%nonassoc LESS GREATER EQUAL LESS_EQUAL GREATER_EQUAL NOT_EQUAL COMP_OP
%right BETWEEN IN
%left PLUS MINUS
%left STAR SLASH
%precedence SUBQUERY_AS_EXPR
%left LP RP
%left JOIN CROSS LEFT FULL RIGHT INNER NATURAL

%type <Statement>						pseudo_start
%type <Expression>						numeric_literal
%type <Expression>						signed_numeric_literal
%type <Expression>						unsigned_numeric_literal
%type <StringLiteral*>						general_literal
%type <Expression>						literal
%type <Expression>						unsigned_literal
%type <Statement>						CreateStatement

%type <DataType>						data_type
%type <bool>							opt_with_time_zone
%type <uint64_t>						opt_time_precision
%type <uint64_t>						opt_timestamp_precision
%type <std::optional<uint64_t>>					precision_spec
%type <std::pair<std::optional<uint64_t>, uint64_t>>		precision_scale_spec
%type <uint64_t>						opt_length_spec
%type <uint64_t>						length_spec

%type <std::vector<Name>>					identifier_list
%type <QualifiedName>						qualified_name
%type <std::optional<Temporary>>				opt_temporary
%type <std::optional<OnCommit>>					opt_on_commit
%type <std::vector<std::variant<ColumnDef, TableConstraint>>>	column_defs_and_constraints
%type <std::variant<ColumnDef, TableConstraint>>		column_def_and_constraint
%type <ColumnDef>						column_def
%type <std::variant<DataType, DomainName>>			column_type
%type <std::optional<ColumnDefault>>				opt_default_clause
%type <ColumnDefault>						default_clause
%type <ColumnDefault>						default_option
%type <std::vector<NamedColumnConstraint>>			opt_column_constraint_def
%type <std::vector<NamedColumnConstraint>>			column_constraint_defs
%type <NamedColumnConstraint>					column_constraint_def
%type <std::optional<QualifiedName>>				opt_constraint_name
%type <ColumnConstraint>					column_constraint
%type <References*>						references_spec
%type <std::pair<QualifiedName, std::vector<Name>>>		ref_table_cols
%type <MatchOption>						opt_match
%type <MatchOption>						match_type
%type <ReferentialTriggeredAction>				opt_referential_triggered_action
%type <ReferentialTriggeredAction>				referential_triggered_action
%type <ReferentialAction>					referential_action
%type <ReferentialAction>					update_rule
%type <ReferentialAction>					delete_rule
%type <std::optional<QualifiedName>>				opt_collate_clause
%type <TableConstraint>						table_constraint_def

%type <Expression>						value_expr
%type <std::vector<Expression>>					value_expr_list
%type <Expression>						common_value_expr
%type <Expression>						bool_value_expr
%type <BoolLiteral>						truth_value
%type <bool>							opt_symmetric
%type <RelExpression>						in_expr
%type <Expression>						bool_predicand
%type <BinaryOp::Op>						comp_op
%type <Expression>						value_expr_no_parens

%type <Statement>						SelectStatement
%type <box<Query>>						select_no_parens
%type <box<Query>>						select_with_parens
%type <RelExpression>						simple_select
%type <RelExpression>						select_clause
%type <std::optional<SetQuantifier>>				opt_set_quantifier

%type <std::vector<box<SortSpec>>>				opt_order_by_clause
%type <std::vector<box<SortSpec>>>				order_by_clause
%type <std::vector<box<SortSpec>>>				sort_spec_list
%type <box<SortSpec>>						sort_spec
%type <SortSpec::Order>						opt_asc_or_desc
%type <SortSpec::NullOrder>					null_ordering

%type <std::optional<box<IntegerLiteral>>>			opt_offset_clause
%type <box<IntegerLiteral>>                                     offset_clause
%type <std::optional<Fetch>>					opt_fetch_first_clause
%type <Fetch>                                                   fetch_first_clause
%type <Fetch::Kind>						fetch_kind
%type <bool>							fetch_percent
%type <bool>							fetch_with_ties
%type <std::optional<box<IntegerLiteral>>>			opt_fetch_quantity

%type <std::vector<Expression>>					target_list
%type <Expression>						target_element
%type <std::vector<RelExpression>>				from_clause
%type <std::vector<RelExpression>>				from_list
%type <RelExpression>						table_ref
%type <JoinExpr*>						joined_table
%type <JoinExpr::Kind>						join_type
%type <std::optional<box<TableAlias>>>				opt_alias_clause
%type <box<TableAlias>>						alias_clause

%type <Expression>						where_clause

%type <std::optional<GroupClause>>				group_clause
%type <std::vector<Grouping>>					group_by_list
%type <Grouping>						group_by_element
%type <box<GroupingSet>>					empty_grouping_set
%type <Expression>						column_ref
%type <std::vector<Expression>>					column_ref_list
%type <box<GroupingSet>>					ordinary_grouping_set
%type <std::vector<box<GroupingSet>>>				ordinary_grouping_set_list
%type <box<Rollup>>						rollup_list
%type <box<Cube>>						cube_list
%type <box<GroupingSets>>					grouping_sets

%type <Expression>						having_clause

%type <std::vector<box<Window>>>				window_clause
%type <std::vector<box<Window>>>				window_definition_list
%type <box<Window>>						window_definition
%type <std::optional<Name>>					opt_existing_window_name
%type <std::vector<Expression>>					opt_partition_clause
%type <std::optional<Window::Frame>>				opt_frame_clause
%type <Window::Frame::Unit>					frame_units
%type <Window::Frame::Bound>					frame_start
%type <Window::Frame::Bound>					frame_bound
%type <std::optional<Window::Frame::Exclusion>>			opt_frame_exclusion

%%

%start pseudo_start;

pseudo_start:
    CreateStatement	{ driver.result_ = $CreateStatement; }
 |  SelectStatement	{ driver.result_ = $SelectStatement; }
 ;

/*
 *   Basics building blocks
 */

literal: numeric_literal | general_literal ;

unsigned_literal: unsigned_numeric_literal | general_literal ;

numeric_literal: signed_numeric_literal	| unsigned_numeric_literal ;

signed_numeric_literal:
    PLUS unsigned_numeric_literal	{ $$ = $unsigned_numeric_literal; }
 |  MINUS unsigned_numeric_literal	{ $$ = mkNode<UnaryOp>(@$, UnaryOp::Op::NEG, $unsigned_numeric_literal); }
 ;

unsigned_numeric_literal:
    INTEGER_VALUE	{ $$ = mkNode<IntegerLiteral>(@INTEGER_VALUE, $INTEGER_VALUE); }
 |  FLOAT_VALUE		{ $$ = mkNode<FloatLiteral>(@FLOAT_VALUE, $FLOAT_VALUE); }
 ;

general_literal:
    STRING_VALUE	{ $$ = mkNode<StringLiteral>(@STRING_VALUE, $STRING_VALUE, StringLiteralType::CHAR); }
 |  BIT_VALUE		{ $$ = mkNode<StringLiteral>(@BIT_VALUE, $BIT_VALUE, StringLiteralType::BIT); }
 |  HEX_VALUE		{ $$ = mkNode<StringLiteral>(@HEX_VALUE, $HEX_VALUE, StringLiteralType::HEX); }
 |  NATIONAL_VALUE	{ $$ = mkNode<StringLiteral>(@NATIONAL_VALUE, $NATIONAL_VALUE, StringLiteralType::NATIONAL); }

/* Missing: INTERVAL */
data_type:
    DECIMAL precision_scale_spec[spec]				{ $$ = DecimalType { $spec.first, $spec.second }; }
 |  FLOAT precision_spec[spec]					{ $$ = FloatType { $spec }; }
 |  INTEGER 							{ $$ = IntegerType { }; }
 |  NUMERIC precision_scale_spec[spec]				{ $$ = NumericType { $spec.first, $spec.second }; }
 |  SMALLINT 							{ $$ = SmallIntType { }; }
 |  REAL	 						{ $$ = RealType { }; }
 |  DOUBLE PRECISION						{ $$ = DoublePrecisionType { }; }
 |  CHARACTER opt_length_spec 					{ $$ = CharType { $opt_length_spec }; }
 |  CHARACTER VARYING length_spec				{ $$ = VarCharType { $length_spec }; }
 |  NATIONAL CHARACTER opt_length_spec				{ $$ = NationalCharType { $opt_length_spec }; }
 |  NATIONAL CHARACTER VARYING length_spec			{ $$ = NationalVarCharType { $length_spec }; }
 |  NCHAR opt_length_spec					{ $$ = NationalCharType { $opt_length_spec }; }
 |  NCHAR VARYING length_spec					{ $$ = NationalVarCharType { $length_spec }; }
 |  BIT opt_length_spec						{ $$ = Bit { $opt_length_spec }; }
 |  BIT VARYING length_spec					{ $$ = VarBit { $length_spec }; }
 |  DATE							{ $$ = DateType { }; }
 |  TIME opt_time_precision opt_with_time_zone			{ $$ = TimeType { $opt_time_precision, $opt_with_time_zone }; }
 |  TIMESTAMP opt_timestamp_precision opt_with_time_zone	{ $$ = TimeType { $opt_timestamp_precision, $opt_with_time_zone }; }
 ;

opt_with_time_zone:
    WITH TIME ZONE						{ $$ = true; }
 |  %empty							{ $$ = false; }
 ;

opt_time_precision:
    LP INTEGER_VALUE[time_precision] RP				{ $$ = $time_precision; }
 |  %empty							{ $$ = 0; }
 ;

opt_timestamp_precision:
    LP INTEGER_VALUE[timestamp_precision] RP			{ $$ = $timestamp_precision; }
 |  %empty							{ $$ = 6; }
 ;

precision_scale_spec:
    LP INTEGER_VALUE[precision] COMMA INTEGER_VALUE[scale] RP 	{ $$ = std::make_pair($precision, $scale); }
 |  LP INTEGER_VALUE[precision] RP				{ $$ = std::make_pair($precision, 0); }
 |  %empty							{ $$ = std::make_pair(std::nullopt, 0); }
 ;

precision_spec:
    LP INTEGER_VALUE RP
 |  %empty							{ $$ = std::nullopt; }
 ;

opt_length_spec:
    length_spec
 |  %empty							{ $$ = 1; }
 ;

length_spec: LP INTEGER_VALUE RP 				{ $$ = $INTEGER_VALUE; };

/*
 *   Handling Identifiers
 */

identifier_list:
    IDENTIFIER[elem] COMMA identifier_list[vec]			{ $vec.push_back($elem); $$ = $vec; }
 |  IDENTIFIER[elem]						{ $$ = std::vector<Name> { $elem }; }
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
	{
		$$ = mkNode<CreateStatement>(@$, $table_name, $opt_temporary, $opt_on_commit, $table_elems);
	}
    ;

opt_temporary:
    LOCAL TEMPORARY						{ $$ = Temporary::LOCAL; }
 |  GLOBAL TEMPORARY						{ $$ = Temporary::GLOBAL; }
 |  %empty							{ $$ = std::nullopt; }
 ;

opt_on_commit:
    ON COMMIT DELETE ROWS					{ $$ = OnCommit::DELETE; }
 |  ON COMMIT PRESERVE ROWS 					{ $$ = OnCommit::PRESERVE; }
 |  %empty		    					{ $$ = std::nullopt; }
 ;

column_defs_and_constraints:
    column_def_and_constraint[elem] COMMA column_defs_and_constraints[vec]
	{
		$vec.push_back($elem); $$ = $vec;
	}
 |  column_def_and_constraint[elem]
 	{
 		$$ = std::vector<std::variant<ColumnDef, TableConstraint>>(); $$.push_back($elem);
 	}
 ;

column_def_and_constraint:
    column_def
 |  table_constraint_def
 ;

column_def:
    IDENTIFIER[column_name] column_type opt_default_clause opt_column_constraint_def opt_collate_clause
	{
		$$ = ColumnDef($column_name, $column_type);
		$$.col_default = $opt_default_clause;
		$$.col_constraint = $opt_column_constraint_def;
		$$.collate = $opt_collate_clause;
	}
 ;

column_type:
    data_type
 |  qualified_name[domain_name]
 ;

opt_default_clause:
    default_clause
 |  %empty							{ $$ = std::nullopt; }
 ;

default_clause:
    DEFAULT default_option					{ $$ = $default_option; }
 ;

default_option:
   literal							{ $$ = $literal; }
 /*| datetime value function */
 |  USER							{ $$ = UserSpec::CURRENT_USER; }
 |  CURRENT_USER						{ $$ = UserSpec::CURRENT_USER; }
 |  SESSION_USER						{ $$ = UserSpec::SESSION_USER; }
 |  SYSTEM_USER							{ $$ = UserSpec::SYSTEM_USER; }
 |  NULL							{ $$ = V_NULL { }; }
 ;

opt_column_constraint_def:
    column_constraint_defs
 |  %empty							{ $$ = std::vector<NamedColumnConstraint>(); }
 ;

column_constraint_defs:
    column_constraint_def[elem] column_constraint_defs[vec]	{ $vec.push_back($elem); $$ = $vec; }
 |  column_constraint_def[elem]
	{
  		$$ = std::vector<NamedColumnConstraint>(); $$.push_back($elem);
  	}
 ;

column_constraint_def:
    opt_constraint_name column_constraint /* opt_constraint_attributes */
	{
		$$ = NamedColumnConstraint { $opt_constraint_name, $column_constraint };
	}
 ;

opt_constraint_name:
    CONSTRAINT qualified_name 					{ $$ = $qualified_name; }
 |  %empty							{ $$ = std::nullopt; }
 ;

column_constraint:
    NOT NULL							{ $$ = ConstraintType::NOT_NULL; }
 |  UNIQUE							{ $$ = ConstraintType::UNIQUE; }
 |  PRIMARY KEY							{ $$ = ConstraintType::PRIMARY_KEY; }
 |  references_spec						{ $$ = $references_spec; }
/* |  check_constraint_def */
 ;

references_spec:
    REFERENCES ref_table_cols opt_match opt_referential_triggered_action
    	{
    		$$ = mkNode<References>(@$,
    			$ref_table_cols.first,
    			$ref_table_cols.second,
    			$opt_match,
    			$opt_referential_triggered_action
    		);
	}
 ;

ref_table_cols:
    qualified_name[table_name] LP identifier_list RP		{ $$ = std::make_pair($table_name, $identifier_list); }
 ;

opt_match:
    MATCH match_type						{ $$ = $match_type; }
 |  %empty							{ $$ = MatchOption::NONE; }
 ;

match_type:
    FULL							{ $$ = MatchOption::FULL; }
 |  PARTIAL							{ $$ = MatchOption::PARTIAL; }
 ;

opt_referential_triggered_action:
    referential_triggered_action				{ $$ = $referential_triggered_action; }
 |  %empty
 	{
 		$$ = ReferentialTriggeredAction {
 			.on_delete = ReferentialAction::NO_ACTION,
 			.on_update = ReferentialAction::NO_ACTION
 		};
 	}
 ;

referential_triggered_action:
    update_rule delete_rule	{ $$ = ReferentialTriggeredAction { .on_delete = $delete_rule, .on_update = $update_rule }; }
 |  delete_rule update_rule	{ $$ = ReferentialTriggeredAction { .on_delete = $delete_rule, .on_update = $update_rule }; }
 |  update_rule			{ $$ = ReferentialTriggeredAction { .on_delete = ReferentialAction::NO_ACTION, .on_update = $update_rule }; }
 |  delete_rule			{ $$ = ReferentialTriggeredAction { .on_delete = $delete_rule, .on_update = ReferentialAction::NO_ACTION }; }
 ;

referential_action:
    CASCADE							{ $$ = ReferentialAction::CASCADE; }
 |  SET NULL							{ $$ = ReferentialAction::SET_NULL; }
 |  SET DEFAULT							{ $$ = ReferentialAction::SET_DEFAULT; }
 |  NO ACTION							{ $$ = ReferentialAction::NO_ACTION; }
 ;

update_rule:
    ON UPDATE referential_action 				{ $$ = $referential_action; }
 ;

delete_rule:
    ON DELETE referential_action 				{ $$ = $referential_action; }
 ;

opt_collate_clause:
    COLLATE qualified_name					{ $$ = $qualified_name; }
 |  %empty							{ $$ = std::nullopt; }
 ;

table_constraint_def:
    UNIQUE LP identifier_list RP				{ $$ = TableUniqueConstraint { $identifier_list }; }
 |  PRIMARY KEY LP identifier_list RP				{ $$ = TablePrimaryKeyConstraint { $identifier_list }; }
 |  FOREIGN KEY LP identifier_list RP references_spec
	{
		$$ = TableForeignKeyConstraint { $identifier_list, $references_spec };
	}
/* |  check_constraint_definition */
 ;

/*
 *  Expressions
 */

value_expr:
    bool_value_expr
 |  LP value_expr_list COMMA value_expr[last] RP
 	{
 		$value_expr_list.push_back($last);
 		$$ = mkNode<RowExpr>(@$, $value_expr_list);
 	}
 |  ROW LP value_expr_list RP					{ $$ =  mkNode<RowExpr>(@$, $value_expr_list); }
 ;

bool_value_expr:
    bool_predicand						{ $$ = $bool_predicand; }
 |  bool_value_expr[left] OR bool_value_expr[right]		{ $$ = mkNode<BinaryOp>(@$, $left, BinaryOp::Op::OR, $right); }
 |  bool_value_expr[left] AND bool_value_expr[right]		{ $$ = mkNode<BinaryOp>(@$, $left, BinaryOp::Op::AND, $right); }
 // NOT and IS accept bool_predicand as child, preventing `NOT NOT expr` / `expr IS TRUE IS FALSE`
 |  NOT bool_predicand[inner]					{ $$ = mkNode<UnaryOp>(@$, UnaryOp::Op::NOT, $inner); }
 |  bool_predicand[inner] IS truth_value			{ $$ = mkNode<IsExpr>(@$, $inner, $truth_value); }
 |  bool_predicand[inner] IS NOT truth_value %prec IS		{ $$ = mkNotNode<IsExpr>(@$, $inner, $truth_value); }
 ;

truth_value:
    TRUE							{ $$ = BoolLiteral::TRUE; }
 |  FALSE							{ $$ = BoolLiteral::FALSE; }
 |  UNKNOWN							{ $$ = BoolLiteral::UNKNOWN; }
 ;

bool_predicand:
    common_value_expr						{ $$ = $common_value_expr; }
 |  bool_predicand[left] comp_op bool_predicand[right] %prec COMP_OP
 	{
 		$$ = mkNode<BinaryOp>(@$, $left, $comp_op, $right);
 	}
 |  bool_predicand[val] BETWEEN opt_symmetric bool_predicand[low] AND bool_predicand[high]	%prec BETWEEN
 	{
 		auto expr = mkNode<BetweenPred>(@$, $val, $low, $high);
 		expr->symmetric = $opt_symmetric;
 		$$ = expr;
 	}
 |  bool_predicand[val] NOT BETWEEN opt_symmetric bool_predicand[low] AND bool_predicand[high]	%prec BETWEEN
 	{
 		auto expr = mkNode<BetweenPred>(@$, $val, $low, $high);
 		expr->symmetric = $opt_symmetric;
 		$$ = mkNode<UnaryOp>(@$, UnaryOp::Op::NOT, expr);
 	}
 |  bool_predicand[val] IN in_expr[rel] 	%prec IN	{ $$ = mkNode<InPred>(@$, $val, $rel); }
 |  bool_predicand[val] NOT IN in_expr[rel]	%prec IN 	{ $$ = mkNotNode<InPred>(@$, $val, $rel); }
 ;

/*
 *  In accordance with the standard, if neither symmetric or asymmetric is specified, then asymmetric is implicit
 */
opt_symmetric:
    SYMMETRIC							{ $$ = true; }
 |  ASYMMETRIC							{ $$ = false; }
 |  %empty							{ $$ = false; }
 ;

in_expr:
    select_with_parens						{ $$ = $select_with_parens; }
 |  LP value_expr_list RP					{ $$ = mkNode<ValuesExpr>(@$, $value_expr_list); }
 ;

value_expr_list:
    value_expr[elem]						{ $$ = std::vector<Expression>(); $$.emplace_back($elem); }
 |  value_expr_list[list] COMMA value_expr[elem]		{ $list.emplace_back($elem); $$ = $list; }
 ;

common_value_expr:
    select_with_parens		%prec SUBQUERY_AS_EXPR		{ $$ = mkNode<RowSubquery>(@$, $select_with_parens); }
 |  LP value_expr RP						{ $$ = $value_expr; }
 |  value_expr_no_parens					{ $$ = $value_expr_no_parens; }
 |  common_value_expr[left] MINUS common_value_expr[right]	{ $$ = mkNode<BinaryOp>(@$, $left, BinaryOp::Op::SUB, $right); }
 |  common_value_expr[left] PLUS common_value_expr[right]	{ $$ = mkNode<BinaryOp>(@$, $left, BinaryOp::Op::ADD, $right); }
 |  common_value_expr[left] SLASH common_value_expr[right]	{ $$ = mkNode<BinaryOp>(@$, $left, BinaryOp::Op::MULT, $right); }
 |  common_value_expr[left] STAR common_value_expr[right]	{ $$ = mkNode<BinaryOp>(@$, $left, BinaryOp::Op::DIV, $right); }
 |  PLUS common_value_expr[inner]				{ $$ = $inner; }
 |  MINUS common_value_expr[inner]				{ $$ = mkNode<UnaryOp>(@$, UnaryOp::Op::NEG, $inner); }
 ;

comp_op:
    NOT_EQUAL							{ $$ = BinaryOp::Op::NOT_EQUAL; }
 |  EQUAL							{ $$ = BinaryOp::Op::EQUAL; }
 |  LESS_EQUAL							{ $$ = BinaryOp::Op::LESS_EQUAL; }
 |  LESS							{ $$ = BinaryOp::Op::LESS; }
 |  GREATER_EQUAL						{ $$ = BinaryOp::Op::GREATER_EQUAL; }
 |  GREATER							{ $$ = BinaryOp::Op::GREATER; }
 ;

value_expr_no_parens:
    unsigned_literal						{ $$ = $unsigned_literal; }
 |  IDENTIFIER							{ $$ = mkNode<Var>(@$, $IDENTIFIER); }
 ;

/************
 *          *
 *  SELECT  *
 *          *
 ************/

SelectStatement:
    select_no_parens						{ $$ = mkNode<SelectStatement>(@$, $select_no_parens); }
 |  select_with_parens						{ $$ = mkNode<SelectStatement>(@$, $select_with_parens); }
 ;

select_with_parens:
    LP select_no_parens RP					{ $$ = $select_no_parens; }
 |  LP select_with_parens[inner] RP				{ $$ = $inner; }
 ;

/*
 *  MISSING: with_clause
 */
select_no_parens:
    simple_select
    	{
    		$$ = mkNode<Query>(@$, $simple_select);
    	}
 |  select_clause order_by_clause opt_offset_clause opt_fetch_first_clause
 	{
 		$$ = mkNode<Query>(@$, $select_clause);
 		$$->order = $order_by_clause;
 		$$->offset = $opt_offset_clause;
 		$$->fetch = $opt_fetch_first_clause;
 	}
 |  select_clause offset_clause opt_fetch_first_clause
 	{
 		$$ = mkNode<Query>(@$, $select_clause);
 		$$->offset = $offset_clause;
 		$$->fetch = $opt_fetch_first_clause;
 	}
 |  select_clause fetch_first_clause
 	{
 		$$ = mkNode<Query>(@$, $select_clause);
 		$$->fetch = $fetch_first_clause;
 	}
 ;

select_clause: simple_select | select_with_parens ;

opt_order_by_clause:
    order_by_clause
 |  %empty							{ $$ = std::vector<box<SortSpec>>(); }
 ;

order_by_clause:
    ORDER BY sort_spec_list					{ $$ = $sort_spec_list; }
 ;

sort_spec_list:
    sort_spec							{ $$ = std::vector<box<SortSpec>>(); $$.emplace_back($sort_spec); }
 |  sort_spec_list[list] COMMA sort_spec			{ $list.push_back($sort_spec); $$ = $list; }
 ;

sort_spec:
    value_expr opt_asc_or_desc null_ordering
	{
		$$ = mkNode<SortSpec>(@$, $value_expr);
		$$->order = $opt_asc_or_desc;
		$$->null_order = $null_ordering;
	}
 ;

opt_asc_or_desc:
    ASC								{ $$ = SortSpec::Order::ASC; }
 |  DESC							{ $$ = SortSpec::Order::DESC; }
 |  %empty							{ $$ = SortSpec::Order::ASC; }
 ;

null_ordering:
    NULLS FIRST							{ $$ = SortSpec::NullOrder::FIRST; }
 |  NULLS LAST							{ $$ = SortSpec::NullOrder::LAST; }
 |  %empty							{ $$ = SortSpec::NullOrder::DEFAULT; }
 ;

opt_offset_clause:
    offset_clause
 |  %empty                                                      { $$ = std::nullopt; }
 ;

offset_clause:
    OFFSET INTEGER_VALUE[count] opt_row_or_rows			{ $$ = mkNode<IntegerLiteral>(@count, $count); }
 ;

opt_fetch_first_clause:
    fetch_first_clause
 |  %empty
 ;

fetch_first_clause:
    FETCH fetch_kind opt_fetch_quantity fetch_percent opt_row_or_rows fetch_with_ties
	{
		Fetch fetch{};
		fetch.kind = $fetch_kind;
		fetch.value = $opt_fetch_quantity;
		fetch.with_ties = $fetch_with_ties;
		fetch.percent = $fetch_percent;
		$$ = std::move(fetch);
	}
 ;

fetch_kind:
    FIRST 							{ $$ = Fetch::Kind::FIRST; }
 |  NEXT 							{ $$ = Fetch::Kind::NEXT; }
 ;

fetch_percent:
    PERCENT							{ $$ = true; }
 |  %empty							{ $$ = false; }
 ;

fetch_with_ties:
    WITH TIES							{ $$ = true; }
 |  ONLY							{ $$ = false; }
 ;

opt_fetch_quantity:
    INTEGER_VALUE[quant]					{ $$ = mkNode<IntegerLiteral>(@quant, $quant); }
 |  %empty							{ $$ = std::nullopt; }
 ;

opt_row_or_rows: ROW | ROWS | %empty;

simple_select:
    SELECT opt_set_quantifier target_list from_clause where_clause group_clause having_clause window_clause
 	{
 		auto expr = mkNode<SelectExpr>(@$);
		expr->target_list = $target_list;
		expr->from_clause = $from_clause;
		expr->where_clause = $where_clause;
		expr->group_clause = $group_clause;
		expr->having_clause = $having_clause;
		expr->window_clause = $window_clause;
		expr->set_quantifier = $opt_set_quantifier;
		$$ = expr;
	}
 |  select_clause[left] UNION opt_set_quantifier[quant] select_clause[right]
	{
		auto expr = mkNode<SetOp>(@$, $left, SetOp::Op::UNION, $right);
		expr->quantifier = $quant;
		$$ = expr;
	}
 |  select_clause[left] INTERSECT opt_set_quantifier[quant] select_clause[right]
    	{
    		auto expr = mkNode<SetOp>(@$, $left, SetOp::Op::INTERSECT, $right);
    		expr->quantifier = $quant;
		$$ = expr;
	}
 |  select_clause[left] EXCEPT opt_set_quantifier[quant] select_clause[right]
	{
		auto expr = mkNode<SetOp>(@$, $left, SetOp::Op::EXCEPT, $right);
		expr->quantifier = $quant;
		$$ = expr;
	}
 ;

opt_set_quantifier:
    ALL								{ $$ = SetQuantifier::ALL; }
 |  DISTINCT							{ $$ = SetQuantifier::DISTINCT; }
 |  %empty							{ $$ = std::nullopt; }
 ;


/*
 *  NOTE: target_list is a slight deviation from the standard:
 *
 *  This version allows
 *     SELECT a, *, b FROM ....
 *
 *  whereas the standard doesn't allow any other targets if the asterisk is part of the target list.
 *
 *  MISSING: <qualified asterisk>
 */
target_list:
   target_element						{ $$ = std::vector<Expression>(); $$.emplace_back($target_element); }
 | target_list[list] COMMA target_element			{ $list.emplace_back($target_element); $$ = $list; }
 ;

target_element:
    value_expr
 |  value_expr AS IDENTIFIER[name]				{ $$ = mkNode<AliasExpr>(@$, $name, $value_expr); }
 |  STAR  							{ $$; }
 ;

from_clause:
    FROM from_list						{ $$ = $from_list; }
 |  %empty 							{ $$ = std::vector<RelExpression>(); }
 ;

from_list:
    table_ref							{ $$ = std::vector<RelExpression>(); $$.emplace_back($table_ref);}
 |  from_list[list] COMMA table_ref				{ $list.emplace_back($table_ref); $$ = $list; }
 ;

/*
 *  NOTE: the alias_clause follows the postgres grammar,
 *  this allows assigning an alias to a joined_table:
 *
 *  select * from (foo CROSS JOIN bar) AS boo
 *
 */
table_ref:
    qualified_name[table_name] opt_alias_clause			{ $$ = applyOptAlias($opt_alias_clause, mkNode<TableName>(@$, $table_name)); }
 |  select_with_parens opt_alias_clause				{ $$ = applyOptAlias($opt_alias_clause, $select_with_parens); }
 |  joined_table						{ $$ = $joined_table; }
 |  LP joined_table RP alias_clause				{ $alias_clause->expression = $joined_table; $$ = $alias_clause; }
 ;

joined_table:
    LP joined_table RP						{ $$ = $2; }
 |  table_ref[a] CROSS JOIN table_ref[b]			{ $$ = mkNode<JoinExpr>(@$, $a, JoinExpr::Kind::INNER, $b); }
 |  table_ref[a] join_type JOIN table_ref[b] ON value_expr[join_qual]
	{
		$$ = mkNode<JoinExpr>(@$, $a, $join_type, $b);
		$$->setQualifier($join_qual);
	}
 |  table_ref[a] join_type JOIN table_ref[b] USING LP identifier_list[names] RP
 	{
 		$$ = mkNode<JoinExpr>(@$, $a, $join_type, $b);
 		$$->columns = $names;
 	}
 |  table_ref[a] JOIN table_ref[b] ON value_expr[join_qual]
 	{
 		$$ = mkNode<JoinExpr>(@$, $a, JoinExpr::Kind::INNER, $b);
 		$$->setQualifier($join_qual);
 	}
 |  table_ref[a] JOIN table_ref[b] USING LP identifier_list[names] RP
	{
		$$ = mkNode<JoinExpr>(@$, $a, JoinExpr::Kind::INNER, $b);
		$$->columns = $names;
	}
 |  table_ref[a] NATURAL join_type JOIN table_ref[b]		{ $$ = mkNode<JoinExpr>(@$, $a, $join_type, $b); $$->setNatural(); }
 |  table_ref[a] NATURAL JOIN table_ref[b]			{ $$ = mkNode<JoinExpr>(@$, $a, JoinExpr::Kind::INNER, $b); $$->setNatural(); }

join_type:
    FULL opt_outer						{ $$ = JoinExpr::Kind::FULL; }
 |  LEFT opt_outer						{ $$ = JoinExpr::Kind::LEFT; }
 |  RIGHT opt_outer						{ $$ = JoinExpr::Kind::RIGHT; }
 |  INNER							{ $$ = JoinExpr::Kind::INNER; }
 ;

// outer is just noise
opt_outer: OUTER | %empty ;

opt_alias_clause:
    alias_clause
 |  %empty							{ $$ = std::nullopt; }
 ;

alias_clause:
    AS IDENTIFIER[name] LP identifier_list[cols] RP		{ $$ = mkNode<TableAlias>(@$, $name); $$->columns = $cols; }
 |  AS IDENTIFIER[name]						{ $$ = mkNode<TableAlias>(@$, $name); }
 |  IDENTIFIER[name] LP identifier_list[cols] RP		{ $$ = mkNode<TableAlias>(@$, $name); $$->columns = $cols; }
 |  IDENTIFIER[name]						{ $$ = mkNode<TableAlias>(@$, $name); }
 ;

where_clause:
    WHERE value_expr 						{ $$ = $value_expr; }
 |  %empty							{ $$; }
 ;

group_clause:
    GROUP BY opt_set_quantifier group_by_list			{ $$ = GroupClause { $opt_set_quantifier, $group_by_list }; }
 |  %empty							{ $$ = std::nullopt; }
 ;

group_by_list:
    group_by_element[elem]					{ $$ = std::vector<Grouping>(); $$.emplace_back($elem); }
 |  group_by_list[list] COMMA group_by_element[elem]		{ $list.emplace_back($elem); $$ = $list; }
 ;

group_by_element:  ordinary_grouping_set | empty_grouping_set | rollup_list | cube_list | grouping_sets ;

empty_grouping_set:
    LP RP							{ $$ = mkNode<GroupingSet>(@$); }
 ;

column_ref:
    IDENTIFIER[name] opt_collate_clause
	{
		auto var = mkNode<Var>(@name, $name);
		if ($opt_collate_clause.has_value()) {
                    $$ = mkNode<Collate>(@$, var, std::move($opt_collate_clause.value()));
		} else {
		    $$ = var;
		}
        }
 ;

column_ref_list:
    column_ref[elem]						{ $$ = std::vector<Expression>(); $$.emplace_back($elem); }
 |  column_ref_list[list] COMMA column_ref[elem]		{ $list.emplace_back($elem); $$ = $list; }
 ;

ordinary_grouping_set:
    column_ref							{ $$ = mkNode<GroupingSet>(@$); $$->columns.emplace_back($column_ref); }
 |  LP column_ref_list RP					{ $$ = mkNode<GroupingSet>(@$); $$->columns = $column_ref_list; }
 ;

ordinary_grouping_set_list:
    ordinary_grouping_set[elem]
	{
		$$ = std::vector<box<GroupingSet>>();
		$$.emplace_back($elem);
	}
 |  ordinary_grouping_set_list[list] COMMA ordinary_grouping_set[elem]
 	{
 		$list.emplace_back($elem);
 		$$ = $list;
 	}

rollup_list:
    ROLLUP LP ordinary_grouping_set_list[ogsl] RP		{ $$ = mkNode<Rollup>(@$); $$->sets = $ogsl; }
 ;

cube_list:
    CUBE LP ordinary_grouping_set_list[ogsl] RP			{ $$ = mkNode<Cube>(@$); $$->sets = $ogsl; }
 ;

grouping_sets:
    GROUPING SETS LP group_by_list RP				{ $$ = mkNode<GroupingSets>(@$); $$->sets = $group_by_list; }
 ;

having_clause:
    HAVING value_expr						{ $$ = $value_expr; }
 |  %empty							{ $$; }
 ;

window_clause:
    WINDOW window_definition_list				{ $$ = $window_definition_list; }
 |  %empty							{ $$ = std::vector<box<Window>>(); }
 ;

window_definition_list:
    window_definition[elem]					{ $$ = std::vector<box<Window>>(); $$.push_back($elem); }
 |  window_definition_list[list] COMMA window_definition[elem]	{ $list.push_back($elem); $$ = $list; }
 ;

window_definition:
    IDENTIFIER[name] AS
    LP opt_existing_window_name opt_partition_clause
       opt_order_by_clause opt_frame_clause
    RP
	{
		$$ = mkNode<Window>(@$, $name,
			$opt_existing_window_name,
			$opt_partition_clause,
			$opt_order_by_clause,
			$opt_frame_clause);
	}
 ;

opt_existing_window_name:
    IDENTIFIER[name]						{ $$ = $name; }
 |  %empty							{ $$ = std::nullopt; }
 ;

opt_partition_clause:
    PARTITION BY column_ref_list				{ $$ = $column_ref_list; }
 |  %empty							{ $$ = std::vector<Expression>(); }
 ;

/*
 *  We inline frame_start and frame_between to
 *  reduce the number of levels we need to pass data through
 */
opt_frame_clause:
    frame_units frame_start opt_frame_exclusion
	{
		$$ = Window::Frame {
			.unit = $frame_units,
			.start = $frame_start,
			.end = std::nullopt,
			.exclude = $opt_frame_exclusion
		};
	}
 |  frame_units BETWEEN frame_bound[start] AND frame_bound[end] opt_frame_exclusion
	{
		$$ = Window::Frame {
			.unit = $frame_units,
			.start = $start,
			.end = $end,
			.exclude = $opt_frame_exclusion
		};
	}
 |  %empty							{ $$ = std::nullopt; }
 ;

frame_units:
    ROWS							{ $$ = Window::Frame::Unit::ROWS; }
 |  RANGE							{ $$ = Window::Frame::Unit::RANGE; }
 |  GROUPS							{ $$ = Window::Frame::Unit::GROUPS; }
 ;

frame_start:
    UNBOUNDED PRECEDING						{ $$ = std::make_pair(Window::Frame::BoundKind::PRECEDING, Expression()); }
 |  unsigned_literal PRECEDING					{ std::make_pair(Window::Frame::BoundKind::PRECEDING, $unsigned_literal); }
 |  CURRENT ROW							{ $$ = std::make_pair(Window::Frame::BoundKind::PRECEDING, Expression()); }
 ;

frame_bound:
    frame_start
 |  UNBOUNDED FOLLOWING						{ $$ = std::make_pair(Window::Frame::BoundKind::FOLLOWING, Expression()); }
 |  unsigned_literal FOLLOWING					{ $$ = std::make_pair(Window::Frame::BoundKind::FOLLOWING, $unsigned_literal); }

opt_frame_exclusion:
    EXCLUDE CURRENT ROW						{ $$ = Window::Frame::Exclusion::CURRENT_ROW; }
 |  EXCLUDE GROUP						{ $$ = Window::Frame::Exclusion::GROUP; }
 |  EXCLUDE TIES						{ $$ = Window::Frame::Exclusion::TIES; }
 |  EXCLUDE NO OTHERS						{ $$ = Window::Frame::Exclusion::NO_OTHERS; }
 |  %empty							{ $$ = std::nullopt; }
 ;


%%

void psql_parse::parser::error(const location_type& l, const std::string& m)
{
    driver.error(l, m);
}

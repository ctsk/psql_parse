#include <sstream>

#include "catch2/catch_test_macros.hpp"

#include "psql_parse/driver.hpp"


TEST_CASE( "Numbers are parsed", "[expr]" ) {
    psql_parse::driver driver;

    auto parse_string = [&driver](std::string&& str) {
        std::istringstream iss(str);
        REQUIRE (driver.parse(iss));
    };

    SECTION( "integer" ) {
		parse_string("192");

		auto resultStmt = driver.getResult();
		auto result = dynamic_cast<psql_parse::ExprStatement*>(resultStmt);
		auto lit = dynamic_cast<const psql_parse::IntegerLiteral*>(result->getExpr());

        REQUIRE(lit->value == 192);

		parse_string("0");
		resultStmt = driver.getResult();
		result = dynamic_cast<psql_parse::ExprStatement*>(resultStmt);
		lit = dynamic_cast<const psql_parse::IntegerLiteral*>(result->getExpr());

		REQUIRE(lit->value == 0);
    }

    SECTION( "floats" ) {
        parse_string("192.");

		auto resultStmt = driver.getResult();
		auto result = dynamic_cast<psql_parse::ExprStatement*>(resultStmt);
		auto lit = dynamic_cast<const psql_parse::FloatLiteral*>(result->getExpr());

		REQUIRE(lit->value == 192.0);
    }
}

TEST_CASE( "Create statements", "[create]") {
	using namespace psql_parse;
	driver driver;

	auto parse_string = [&driver](std::string&& str) {
		std::istringstream iss(str);
		REQUIRE(driver.parse(iss));
	};

	CreateStatement c1(location(nullptr, 1, 2), QualifiedName {});
	CreateStatement c2(location(nullptr, 10, 20), QualifiedName {});

	REQUIRE(c1.equals(c2));

	SECTION( "create table <relname> " ) {
		parse_string("create table boo ( foo INT )");

		auto resultStmt = driver.getResult();
		auto result = dynamic_cast<CreateStatement*>(resultStmt);

		CreateStatement expected(location(nullptr, 0, 0), QualifiedName {.name = "boo"});
		ColumnDef columnDef(
			"foo",
			IntegerType { }
		);

		expected.column_defs.push_back(std::move(columnDef));
		REQUIRE(expected.equals(*result));
	}
}

TEST_CASE( "Select statements", "[select]") {
	using namespace psql_parse;
	driver driver;

	auto parse_string = [&driver](std::string&& str) {
		std::istringstream iss(str);
		REQUIRE(driver.parse(iss));
	};

	SECTION( "select 1" ) {
		parse_string("select 1");

		auto resultStmt = driver.getResult();
		auto &result = dynamic_cast<SelectStatement*>(resultStmt)->query_expr;
		REQUIRE(!result->set_quantifier.has_value());
		REQUIRE(result->target_list.size() == 1);
		auto expr = dynamic_cast<IntegerLiteral*>(result->target_list.at(0).get());
		REQUIRE(expr->value == 1);
	}

	SECTION( "select 1, 2" ) {
		parse_string("select 1, 2");

		auto resultStmt = driver.getResult();
		auto &result = dynamic_cast<SelectStatement*>(resultStmt)->query_expr;
		REQUIRE(!result->set_quantifier.has_value());
		REQUIRE(result->target_list.size() == 2);
		auto expr1 = dynamic_cast<IntegerLiteral*>(result->target_list.at(0).get());
		auto expr2 = dynamic_cast<IntegerLiteral*>(result->target_list.at(1).get());
		REQUIRE(expr1->value == 1);
		REQUIRE(expr2->value == 2);
	}

	SECTION( "select *" ) {
		parse_string("select *");

		auto resultStmt = driver.getResult();
		auto &result = dynamic_cast<SelectStatement*>(resultStmt)->query_expr;
		REQUIRE(!result->set_quantifier.has_value());
		REQUIRE(result->target_list.size() == 1);
		REQUIRE(result->target_list.at(0) == nullptr);
	}

	SECTION( "select [ALL|DISTINCT] 1" ) {
		parse_string("select ALL 1");
		auto resultStmt = driver.getResult();
		auto &result = dynamic_cast<SelectStatement*>(resultStmt)->query_expr;
		REQUIRE(result->set_quantifier.value() == SetQuantifier::ALL);


		parse_string("select DISTINCT 1");
		resultStmt = driver.getResult();
		auto &distinct_result = dynamic_cast<SelectStatement*>(resultStmt)->query_expr;
		REQUIRE(distinct_result->set_quantifier.value() == SetQuantifier::DISTINCT);
	}

	SECTION( "select 1 from hello" ) {
		parse_string("select 1 from hello");
		auto resultStmt = driver.getResult();
		auto &result = dynamic_cast<SelectStatement*>(resultStmt)->query_expr;

		REQUIRE(result->target_list.size() == 1);
		REQUIRE(result->from_clause.size() == 1);
	}

	SECTION( "select 1 from (select foo from bar)" ) {
		parse_string("select 1 from hello");
		auto resultStmt = driver.getResult();
		auto &result = dynamic_cast<SelectStatement*>(resultStmt)->query_expr;

		REQUIRE(result->target_list.size() == 1);
		REQUIRE(result->from_clause.size() == 1);
	}
}
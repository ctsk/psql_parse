#include <sstream>

#include "catch2/catch_test_macros.hpp"

#include "psql_parse/driver.hpp"

using SelectStatement = psql_parse::SelectStatement;
using psql_parse::box;


TEST_CASE( "Numbers are parsed", "[expr]" ) {
    psql_parse::driver driver;

    auto parser = [&driver](std::string&& str) -> box<SelectStatement> {
        std::istringstream iss(str);
        REQUIRE (driver.parse(iss));
		auto &resultStmt = driver.getResult();
		REQUIRE(std::holds_alternative<box<SelectStatement>>(resultStmt));
		return std::move(std::get<box<SelectStatement>>(resultStmt));
    };

    SECTION( "integer" ) {
		auto result = parser("select 192");

		auto s = new psql_parse::SelectExpr();
		auto q = new psql_parse::Query(s);
		s->target_list.push_back(new psql_parse::IntegerLiteral(192));
		auto expected = new SelectStatement(q);

		REQUIRE(result == expected);
    }

    SECTION( "floats" ) {
		auto result = parser("select 10.2");

		auto s = new psql_parse::SelectExpr();
		auto q = new psql_parse::Query(s);
		s->target_list.push_back(new psql_parse::FloatLiteral(10.2));
		auto expected = new SelectStatement(q);

		REQUIRE(result == expected);
    }
}

TEST_CASE( "lexing identifiers", "[lex-ident]") {
	using namespace psql_parse;
	driver driver;

	auto parser = [&driver](std::string&& str) -> box<SelectStatement> {
		std::istringstream iss(str);
		REQUIRE (driver.parse(iss));
		auto &resultStmt = driver.getResult();
		REQUIRE(std::holds_alternative<box<SelectStatement>>(resultStmt));
		return std::move(std::get<box<SelectStatement>>(resultStmt));
	};

	SECTION( "select FOO" ) {
		auto result = parser("select FOO");

		auto s = new psql_parse::SelectExpr();
		auto q = new psql_parse::Query(s);
		s->target_list.push_back(new psql_parse::Var("foo"));
		auto expected = box<SelectStatement>::make(q);

		REQUIRE(result == expected);
	}

	SECTION( "select \"FOO\"" ) {
		auto result = parser( R"(select "FOO")");

		auto s = new psql_parse::SelectExpr();
		auto q = new psql_parse::Query(s);
		s->target_list.push_back(new psql_parse::Var("FOO"));
		auto expected = new SelectStatement(std::move(q));

		REQUIRE(result == expected);
	}
}

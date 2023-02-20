#pragma clang diagnostic push
#pragma ide diagnostic ignored "cppcoreguidelines-pro-type-static-cast-downcast"


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

		std::unique_ptr<psql_parse::Statement> resultStmt = driver.getResult();
		auto result = static_cast<psql_parse::ExprStatement*>(resultStmt.get());
		auto lit = static_cast<const psql_parse::IntegerLiteral*>(result->getExpr());

        REQUIRE(lit->value == 192);

		parse_string("0");
		resultStmt = driver.getResult();
		result = static_cast<psql_parse::ExprStatement*>(resultStmt.get());
		lit = static_cast<const psql_parse::IntegerLiteral*>(result->getExpr());

		REQUIRE(lit->value == 0);
    }

    SECTION( "floats" ) {
        parse_string("192.");

		std::unique_ptr<psql_parse::Statement> resultStmt = driver.getResult();
		auto result = static_cast<psql_parse::ExprStatement*>(resultStmt.get());
		auto lit = static_cast<const psql_parse::FloatLiteral*>(result->getExpr());

		REQUIRE(lit->value == 192.0);
    }
}

TEST_CASE( "Create statements", "[create]") {
	psql_parse::driver driver;

	auto parse_string = [&driver](std::string&& str) {
		std::istringstream iss(str);
		REQUIRE(driver.parse(iss));
	};

	SECTION( "create table <relname> " ) {
		parse_string("create table boo ( foo INT )");

		std::unique_ptr<psql_parse::Statement> resultStmt = driver.getResult();
		auto result = static_cast<psql_parse::CreateStatement*>(resultStmt.get());

		REQUIRE(result->rel_name.name == "boo");
	}

}
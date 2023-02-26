#include <sstream>

#include "catch2/catch_test_macros.hpp"

#include "psql_parse/driver.hpp"


//TEST_CASE( "Numbers are parsed", "[expr]" ) {
//    psql_parse::driver driver;
//
//    auto parse_string = [&driver](std::string&& str) {
//        std::istringstream iss(str);
//        REQUIRE (driver.parse(iss));
//    };
//
//    SECTION( "integer" ) {
//		parse_string("select 192");
//
//		auto resultStmt = driver.getResult();
//		auto result = dynamic_cast<psql_parse::ExprStatement*>(resultStmt);
//		auto lit = dynamic_cast<const psql_parse::IntegerLiteral*>(result->getExpr());
//
//        REQUIRE(lit->value == 192);
//
//		parse_string("0");
//		resultStmt = driver.getResult();
//		result = dynamic_cast<psql_parse::ExprStatement*>(resultStmt);
//		lit = dynamic_cast<const psql_parse::IntegerLiteral*>(result->getExpr());
//
//		REQUIRE(lit->value == 0);
//    }
//
//    SECTION( "floats" ) {
//        parse_string("select 192.");
//
//		auto resultStmt = driver.getResult();
//		auto result = dynamic_cast<psql_parse::ExprStatement*>(resultStmt);
//		auto lit = dynamic_cast<const psql_parse::FloatLiteral*>(result->getExpr());
//
//		REQUIRE(lit->value == 192.0);
//    }
//}

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

	auto parse_string_into_query = [&driver](std::string&& str) -> QueryExpr* {
		std::istringstream iss(str);
		REQUIRE(driver.parse(iss));		
		auto selectStmt = dynamic_cast<SelectStatement*>(driver.getResult());
		return dynamic_cast<QueryExpr*>(selectStmt->rel_expr.get());
	};
	auto parse_string_into_set_op = [&driver](std::string&& str) -> SetOp* {
		std::istringstream iss(str);
		REQUIRE(driver.parse(iss));
		auto selectStmt = dynamic_cast<SelectStatement*>(driver.getResult());
		return dynamic_cast<SetOp*>(selectStmt->rel_expr.get());
	};

	SECTION( "select 1" ) {
		auto result = parse_string_into_query("select 1");
		REQUIRE(!result->set_quantifier.has_value());
		REQUIRE(result->target_list.size() == 1);
		auto expr = dynamic_cast<IntegerLiteral*>(result->target_list.at(0).get());
		REQUIRE(expr->value == 1);
	}

	SECTION( "select 1, 2" ) {
		auto result = parse_string_into_query("select 1, 2");
		REQUIRE(!result->set_quantifier.has_value());
		REQUIRE(result->target_list.size() == 2);
		auto expr1 = dynamic_cast<IntegerLiteral*>(result->target_list.at(0).get());
		auto expr2 = dynamic_cast<IntegerLiteral*>(result->target_list.at(1).get());
		REQUIRE(expr1->value == 1);
		REQUIRE(expr2->value == 2);
	}

	SECTION( "select *" ) {
		auto result = parse_string_into_query("select *");
		REQUIRE(!result->set_quantifier.has_value());
		REQUIRE(result->target_list.size() == 1);
		REQUIRE(result->target_list.at(0) == nullptr);
	}

	SECTION( "select ALL 1" ) {
		auto result = parse_string_into_query("select ALL 1");
		REQUIRE(result->set_quantifier.value() == SetQuantifier::ALL);
	}

	SECTION( "select DISTINCT 1") {
		auto result = parse_string_into_query("select DISTINCT 1");
		REQUIRE(result->set_quantifier.value() == SetQuantifier::DISTINCT);
	}

	SECTION( "select 1 from hello" ) {
		auto result = parse_string_into_query("select 1 from hello");
		REQUIRE(result->target_list.size() == 1);
		REQUIRE(result->from_clause.size() == 1);
	}

	SECTION( "select 1 from (select 1 from bar)" ) {
		auto result = parse_string_into_query("select 1 from (select 1 from bar)");
		REQUIRE(result->target_list.size() == 1);
		REQUIRE(result->from_clause.size() == 1);
	}

	SECTION( "select 1 from (select 1 from bar)" ) {
		auto result = parse_string_into_set_op("select 1 UNION select 2");
		REQUIRE(result->op == psql_parse::SetOp::Op::UNION);
	}

	SECTION( "select 1 from (select 1 from bar)" ) {
		auto result = parse_string_into_set_op("select 1 INTERSECT select 2");
		REQUIRE(result->op == psql_parse::SetOp::Op::INTERSECT);
	}

}
#include <sstream>
#include <functional>

#include "catch2/catch_test_macros.hpp"

#include "psql_parse/driver.hpp"
#include "psql_parse/visit.hpp"

using Expression = psql_parse::Expression;
using Statement = psql_parse::Statement;
using SelectStatement = psql_parse::SelectStatement;
using SelectExpr = psql_parse::SelectExpr;
using Query = psql_parse::Query;
using psql_parse::box;

namespace {
    auto makeStmt(psql_parse::RelExpression exprPtr) -> box<SelectStatement> {
        auto q = new Query(std::move(exprPtr));
        return new SelectStatement(q);
    }

    auto makeSelect(Expression expr) -> box<SelectExpr> {
        auto selectExpr = box<SelectExpr>::make();
        selectExpr->target_list.push_back(std::move(expr));
        return selectExpr;
    }

    auto makeStmt(Expression expr) -> box<SelectStatement> {
        return makeStmt(makeSelect(std::move(expr)));
    }

    auto mustParse(psql_parse::driver& drv, std::string&& str) -> Statement {
        std::istringstream iss(str);
        INFO(str);
        REQUIRE (drv.parse(iss));
        return std::move(drv.getResult());
    }

    template <class T>
    auto mustParseInto(psql_parse::driver& drv, std::string&& str) -> box<T> {
        std::istringstream iss(str);
        REQUIRE (drv.parse(iss));
        auto &result = drv.getResult();
        REQUIRE (std::holds_alternative<box<T>>(result));
        return std::move(std::get<box<T>>(drv.getResult()));
    }

    void mustNotParse(psql_parse::driver& drv, std::string&& str) {
        std::istringstream iss(str);
        REQUIRE_FALSE (drv.parse(iss));
    }
}


TEST_CASE( "Numbers are parsed", "[expr]" ) {
    psql_parse::driver driver;
    auto parser = std::bind_front(mustParseInto<SelectStatement>, std::ref(driver));

    SECTION( "integer" ) {
		auto result = parser("select 192");

        auto literal = new psql_parse::IntegerLiteral(192);
		auto expected = makeStmt(literal);

		REQUIRE(result == expected);
    }

    SECTION( "floats" ) {
		auto result = parser("select 10.2");

		auto literal = new psql_parse::FloatLiteral(10.2);
        auto expected = makeStmt(literal);

		REQUIRE(result == expected);
    }
}

TEST_CASE( "lexing identifiers", "[lex-ident]") {
	psql_parse::driver driver;
    auto parser = std::bind_front(mustParseInto<SelectStatement>, std::ref(driver));

	SECTION( "case-insensitive identifier" ) {
		auto result = parser("select FOO");

        auto varExpr = new psql_parse::Var("foo");
        auto expected = makeStmt(varExpr);

		REQUIRE(result == expected);
	}

	SECTION( "double-quoted identifier" ) {
		auto result = parser( R"(select "FOO")");

        auto varExpr = new psql_parse::Var("FOO");
        auto expected = makeStmt(varExpr);

		REQUIRE(result == expected);
	}
}

TEST_CASE( "set operations", "set-ops" ) {
    psql_parse::driver driver;
    auto parser = std::bind_front(mustParseInto<SelectStatement>, std::ref(driver));

    SECTION( "union" ) {
        auto result = parser("select 1 UNION select 2");
        auto expected = makeStmt(
                new psql_parse::SetOp(
                        makeSelect(new psql_parse::IntegerLiteral(1)),
                        psql_parse::SetOp::Op::UNION,
                        makeSelect(new psql_parse::IntegerLiteral(2))));

        REQUIRE(result == expected);
    }

    SECTION( "union-all" ) {
        auto result = parser("select 1 UNION ALL select 2");
        auto setOp = new psql_parse::SetOp(
                makeSelect(new psql_parse::IntegerLiteral(1)),
                psql_parse::SetOp::Op::UNION,
                makeSelect(new psql_parse::IntegerLiteral(2)));
        setOp->quantifier = psql_parse::SetQuantifier::ALL;
        auto expected = makeStmt(setOp);

        REQUIRE(result == expected);
    }

    SECTION( "union-distinct" ) {
        auto result = parser("select 1 UNION DISTINCT select 2");
        auto setOp = new psql_parse::SetOp(
                        makeSelect(new psql_parse::IntegerLiteral(1)),
                        psql_parse::SetOp::Op::UNION,
                        makeSelect(new psql_parse::IntegerLiteral(2)));
        setOp->quantifier = psql_parse::SetQuantifier::DISTINCT;
        auto expected = makeStmt(setOp);

        REQUIRE(result == expected);
    }

    SECTION( "intersect" ) {
        auto result = parser("select foo INTERSECT select bar from foo");
        auto rhs = makeSelect(new psql_parse::Var("bar"));
        rhs->from_clause.push_back(new psql_parse::TableName(new psql_parse::QualifiedName("foo")));
        auto expected = makeStmt(
                new psql_parse::SetOp(
                        makeSelect(new psql_parse::Var("foo")),
                        psql_parse::SetOp::Op::INTERSECT,
                        std::move(rhs)));

        REQUIRE(result == expected);
    }

    SECTION( "except" ) {
        auto result = parser("select 1 EXCEPT select 2.0");
        auto expected = makeStmt(
                new psql_parse::SetOp(
                        makeSelect(new psql_parse::IntegerLiteral(1)),
                        psql_parse::SetOp::Op::EXCEPT,
                        makeSelect(new psql_parse::FloatLiteral(2.0))));

        REQUIRE(result == expected);
    }
}

TEST_CASE( "supported concepts", "[cov]") {
    psql_parse::driver driver;
    auto parser = std::bind_front(mustParse, std::ref(driver));

    auto concepts = {
            /*
             * Numbers & Arithmetic
             */
            "select 1",
            "select 1.0",
            "select -1",
            "select +1",
            "select 2 * 3, 2+3, 2-3, 2/3",
            /*
             * Boolean Literals
             */
            "select TRUE,FALSE,UNKNOWN from whatever",
            /*
             * Identifiers;
             */
            "select foo",
            "select bar",
            "select \"foo\"",
            "select \"FOO\"",
            /*
             * Strings
             */
            "select 'blob'",
            R"(select "a" || "b")",
            "select \"a\" COLLATE coll",
            /*
             * Row expression
             */
            "select * from boo where (ROW (1,2,3)) = ((1,2,3))", // todo: check standard conformance
            "select (1,2,3),4 from boo",
            /*
             * Multiple select targets
             */
            "select 1, 2, 3",
            "select 1, 2 as bar, 3 AS foo",
            /*
             * Nested select in target list
             */
            "select 1, (select 2), 3",
            "(select 1)",
            "((select 1))",
            /*
             * Select asterisk
             */
            "select 1, * from bar",
            /*
             * Set Operations
             */
            "select 1 UNION select 2",
            "select 1 UNION ALL select 2",
            "select 1 UNION DISTINCT select 2",
            "select 1 INTERSECT select 2",
            "select 1 INTERSECT ALL select 2",
            "select 1 INTERSECT DISTINCT select 2",
            "select 1 EXCEPT select 2",
            "select 1 EXCEPT ALL select 2",
            "select 1 EXCEPT DISTINCT select 2",
            /*
             * Joins
             */
            "select foo from bar CROSS JOIN boo",
            "select foo from bar CROSS JOIN (boo JOIN baz ON 1)",
            "select foo from bar join bar on 1 = 1",
            "select foo from bar join bar using (foo, faz)",
            "select foo from bar INNER JOIN bar using (foo, bar)",
            "select foo from bar FULL JOIN bar ON 1 = 1",
            "select foo from bar LEFT JOIN bar ON 1 = 1",
            "select foo from bar RIGHT JOIN bar ON 1 = 1",
            "select foo from bar RIGHT OUTER JOIN bar ON 1 = 1",
            "select foo from bar NATURAL LEFT JOIN bar",
            "select foo from bar NATURAL JOIN bar",
            /*
             * multiple base tables
             */
            "select foo from bar, boo, boo NATURAL JOIN boo",
            /*
             * nested select in from_clause
             */
            "select foo from bar, (select 1, 2, 3), baz",
            /*
             * Values
             */
            "VALUES (1,2,3)",
            /*
             * Explicit table ref
             */
            "TABLE foo.bar",
            /*
             * Table Alias
             */
            "select foo from (bar NATURAL JOIN bar) as baz",
            "select foo from bar b",
            "select foo from bar b(c,d)",
            "select foo from bar AS b(c)",
            "select foo from bar, (select 1, 2, 3) AS boo, baz",
            /*
             * Where clause
             */
            "select 1 where 2",
            "select foo from bar where baz = boo",
            "select foo from bar where baz <> boo",
            "select foo from bar where baz >= boo",
            "select foo from bar where baz > boo",
            "select foo from bar where baz <= boo",
            "select foo from bar where baz < boo",
            /*
             * Group By
             */
            "select * from bar GROUP BY ()",
            "select * from bar GROUP BY (bar, foo COLLATE collation)",
            "select * from bar GROUP BY ALL bar",
            "select * from bar GROUP BY DISTINCT bar",
            "select * from bar GROUP BY bar, baz",
            "select * from bar GROUP BY ROLLUP (bar, baz)",
            "select * from bar GROUP BY Cube (bar, (bar, baz))",
            "select * from bar GROUP BY Cube (bar, baz, foo)",
            "select * from bar GROUP BY GROUPING SETS (bar, ROLLUP (baz))",
            /*
             * Having
             */
            "select 1 from bar, baz group by foo HAVING 1 <> 2",
            /*
             * Window
             */
            "select foo from bar WINDOW windowname AS (existingname)",
            "select foo from bar WINDOW windowname AS (existingname), windownametwo AS (existingnametwo)",
            "select foo from bar WINDOW windowname AS (existingname PARTITION BY column)",
            "select foo from bar WINDOW windowname AS (existingname ORDER BY somecol)",
            "select foo from bar WINDOW windowname AS (ROWS BETWEEN UNBOUNDED PRECEDING AND UNBOUNDED FOLLOWING EXCLUDE TIES)",
            "select foo from bar WINDOW windowname AS (ROWS 1 PRECEDING EXCLUDE GROUP)",
            "select foo from bar WINDOW windowname AS (ROWS CURRENT ROW EXCLUDE CURRENT ROW)",
            "select foo from bar WINDOW windowname AS (RANGE BETWEEN UNBOUNDED FOLLOWING AND UNBOUNDED FOLLOWING EXCLUDE NO OTHERS)",
            "select foo from bar WINDOW windowname AS (GROUPS BETWEEN 1 PRECEDING AND 1 FOLLOWING)",
            /*
             * Order
             */
            "select foo ORDER BY a, 2*b",
            "select foo ORDER BY foo ASC",
            "select foo ORDER BY foo DESC",
            "select foo ORDER BY foo DESC NULLS FIRST",
            "select foo ORDER BY foo DESC NULLS LAST",
            /*
             * Offset
             */
            "select foo OFFSET 1 ROW",
            /*
             * Fetch
             */
            "select foo OFFSET 1 ROW FETCH FIRST ROW WITH TIES",
            "select foo FETCH FIRST 3 ROWS ONLY",
            "select foo FETCH FIRST ROW WITH TIES",
            "select foo FETCH NEXT 1 ROW WITH TIES",
            "select foo FETCH NEXT 10 PERCENT ROWS ONLY",
            /*
             * Exists predicate
             */
            "select foo from bar where exists (select 1)",
            /*
             * Unique predicate on a subquery
             */
            "select foo from bar where unique (select 1)",
            /*
             * More predicates
             */
            "select foo from bar where baz BETWEEN 1 AND 2",
            "select foo from bar where baz NOT BETWEEN SYMMETRIC 1 AND 2",
            "select foo from bar where baz BETWEEN ASYMMETRIC 1 AND 2",

            "select foo from bar where 1 IN (select 1)",
            "select foo from bar where 1 NOT IN (1,2,3)",

            "select foo from bar where baz LIKE \"woohoo%\"",
            R"(select foo from bar where baz NOT LIKE "hello world" ESCAPE "a")",

            /*
             * Boolean logic
             */

            "select foo from bar where a AND b",
            "select foo from bar where a OR b",
            "select foo from bar where a IS TRUE",
            "select foo from bar where a IS NOT FALSE",
            "select foo from bar where a IS UNKNOWN",
            "select foo from bar where NOT (a OR b)",
    };

    for (auto const &c : concepts) {
        parser(c);
    }

}

TEST_CASE( "visit tests" ) {
    using namespace psql_parse;

    Expression expr2 = new BinaryOp(
            new IntegerLiteral(1),
            BinaryOp::Op::ADD,
            new IntegerLiteral(2));

    printer p{ std::cout };
    p.print(expr2);
}


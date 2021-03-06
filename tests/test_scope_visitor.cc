#include <catch.hpp>
#include <string>
#include "antlr4-runtime.h"
#include "eelLexer.h"
#include "eelParser.h"
#include "Visitors/ScopeVisitor.hpp"
#include "Visitors/utility.hpp"

using namespace std;
using namespace antlr4;
using namespace eel;

#define SCOPE_ANALYSIS(String) \
    ANTLRInputStream input(String); \
    eelLexer lexer(&input); \
    CommonTokenStream tokens(&lexer); \
    tokens.fill(); \
    eelParser parser(&tokens); \
    auto tree = parser.program(); \
    SymbolTable table; \
    ScopeVisitor scope_visitor(&table); \
    scope_visitor.visitProgram(tree); \


TEST_CASE("scope bleed","[scope_analysis]") {
    SCOPE_ANALYSIS("setup{u8 x = 2;} loop{f32 x = 2.2;}");
    REQUIRE(table.get_scope_count() == 3);
    auto x1 = table.get_scope(1)->find("x");
    auto x2 = table.get_scope(2)->find("x");
    REQUIRE(x1->kind == Symbol_::Kind::Variable);
    REQUIRE(x2->kind == Symbol_::Kind::Variable);
    REQUIRE(x1->value.variable->type->name != x2->value.variable->type->name);
}

TEST_CASE("scope overshadowing","[scope_analysis]") {
    SCOPE_ANALYSIS("setup{u8 x = 2; if(true) { u16 x = 4; } }")
    REQUIRE(table.get_scope_count() == 3);
    auto x1 = table.get_scope(1)->find("x");
    auto x2 = table.get_scope(2)->find("x");
    REQUIRE(x1->kind == Symbol_::Kind::Variable);
    REQUIRE(x2->kind == Symbol_::Kind::Variable);
    REQUIRE(x1->value.variable->type->name != x2->value.variable->type->name);
}

TEST_CASE("predicate less events","[scope_analysis]") {
    SCOPE_ANALYSIS("event x;")
    REQUIRE(table.get_scope_count() == 1);
    auto x1 = table.get_scope(0)->find("x");
    REQUIRE(x1->kind == Symbol_::Kind::Event);
    REQUIRE(x1->value.event->has_predicate == false);
    REQUIRE(x1->value.event->is_complete == true);
}

TEST_CASE("deferred event declaration","[scope_analysis]") {
    SCOPE_ANALYSIS("on x {} event x {return true;}")
    REQUIRE(table.get_scope_count() == 3);
    auto x1 = table.get_scope(0)->find("x");
    REQUIRE(x1->kind == Symbol_::Kind::Event);
    REQUIRE(x1->value.event->has_predicate == true);
    REQUIRE(x1->value.event->is_complete == true);
}

TEST_CASE("duplicate event", "[scope_analysis]"){
    SCOPE_ANALYSIS("event x { return true; } event x { return true;  }")
    REQUIRE(scope_visitor.errors.size() == 1);
    REQUIRE(scope_visitor.errors.at(0).kind == Error::Kind::DuplicateEvent);
}

TEST_CASE("Setup/Loop void function", "[scope_analysis]") {
    SCOPE_ANALYSIS("setup{} loop{}")
    REQUIRE(table.get_scope_count() == 3);
    auto setup = table.get_scope(0)->find(visitors::builtin_setup_name);
    auto loop = table.get_scope(0)->find(visitors::builtin_loop_name);
    REQUIRE(setup->kind == Symbol_::Kind::Function);
    REQUIRE(setup->value.function->has_return_type() == false);
    REQUIRE(loop->kind == Symbol_::Kind::Function);
    REQUIRE(loop->value.function->has_return_type() == false);
}


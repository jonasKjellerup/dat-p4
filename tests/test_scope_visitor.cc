#include <catch.hpp>
#include <string>
#include "antlr4-runtime.h"
#include "eelLexer.h"
#include "eelParser.h"
#include "Visitors/ScopeVisitor.hpp"

using namespace std;
using namespace antlr4;
using namespace eel;

#define GET_TOKENS(String, Result) \
    ANTLRInputStream input(String); \
    eelLexer lexer(&input); \
    CommonTokenStream tokens(&lexer); \
    tokens.fill(); \
    *(Result) = tokens.getTokens();\


TEST_CASE("scope bleed","[scope_analysis]") {
    ANTLRInputStream input("setup{u8 x = 2;} loop{f32 x = 2.2;}");
    eelLexer lexer(&input);
    CommonTokenStream tokens(&lexer);
    tokens.fill();
    eelParser parser(&tokens);
    auto tree = parser.program();
    SymbolTable table;
    ScopeVisitor scope_visitor(&table);
    scope_visitor.visitProgram(tree);
    REQUIRE(table.get_scope_count() == 3);
    auto x1 = table.get_scope(1)->find("x");
    auto x2 = table.get_scope(2)->find("x");
    REQUIRE(x1->kind == Symbol_::Kind::Variable);
    REQUIRE(x2->kind == Symbol_::Kind::Variable);
    REQUIRE(x1->value.variable->type->name != x2->value.variable->type->name);
}

TEST_CASE("scope overshadowing","[scope_analysis]") {
    ANTLRInputStream input("setup{u8 x = 2; if(true) { u16 x = 4; } }");
    eelLexer lexer(&input);
    CommonTokenStream tokens(&lexer);
    tokens.fill();
    eelParser parser(&tokens);
    auto tree = parser.program();
    SymbolTable table;
    ScopeVisitor scope_visitor(&table);
    scope_visitor.visitProgram(tree);
    REQUIRE(table.get_scope_count() == 3);
    auto x1 = table.get_scope(1)->find("x");
    auto x2 = table.get_scope(2)->find("x");
    REQUIRE(x1->kind == Symbol_::Kind::Variable);
    REQUIRE(x2->kind == Symbol_::Kind::Variable);
    REQUIRE(x1->value.variable->type->name != x2->value.variable->type->name);
}

TEST_CASE("predicate less events","[scope_analysis]") {
    ANTLRInputStream input("event x;");
    eelLexer lexer(&input);
    CommonTokenStream tokens(&lexer);
    tokens.fill();
    eelParser parser(&tokens);
    auto tree = parser.program();
    SymbolTable table;
    ScopeVisitor scope_visitor(&table);
    scope_visitor.visitProgram(tree);
    REQUIRE(table.get_scope_count() == 1);
    auto x1 = table.get_scope(0)->find("x");
    REQUIRE(x1->kind == Symbol_::Kind::Event);
    REQUIRE(x1->value.event->has_predicate == false);
    REQUIRE(x1->value.event->is_complete == true);
}

// Will always fail currently since there is no way to give an event signature a predicate after declaration
TEST_CASE("Event signature","[scope_analysis]") {
    ANTLRInputStream input("event x; event x {return true;}");
    eelLexer lexer(&input);
    CommonTokenStream tokens(&lexer);
    tokens.fill();
    eelParser parser(&tokens);
    auto tree = parser.program();
    SymbolTable table;
    ScopeVisitor scope_visitor(&table);
    scope_visitor.visitProgram(tree);
    REQUIRE(table.get_scope_count() == 1);
    auto x1 = table.get_scope(0)->find("x");
    REQUIRE(x1->kind == Symbol_::Kind::Event);
    REQUIRE(x1->value.event->has_predicate == true);
    REQUIRE(x1->value.event->is_complete == true);
}

TEST_CASE("Setup/Loop void function", "[scope_analysis]") {
    ANTLRInputStream input("setup {} loop{}");
    eelLexer lexer(&input);
    CommonTokenStream tokens(&lexer);
    tokens.fill();
    eelParser parser(&tokens);
    auto tree = parser.program();
    SymbolTable table;
    ScopeVisitor scope_visitor(&table);
    scope_visitor.visitProgram(tree);
    REQUIRE(table.get_scope_count() == 3);
    auto setup = table.get_scope(0)->find("setup");
    auto loop = table.get_scope(0)->find("loop");
    REQUIRE(setup->kind == Symbol_::Kind::Function);
    REQUIRE(setup->value.function->has_return_type == false);
    REQUIRE(loop->kind == Symbol_::Kind::Function);
    REQUIRE(loop->value.function->has_return_type == false);
}


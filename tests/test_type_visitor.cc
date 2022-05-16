#include <catch.hpp>
#include <string>
#include "antlr4-runtime.h"
#include "eelLexer.h"
#include "eelParser.h"
#include "Visitors/ScopeVisitor.hpp"
#include "Visitors/TypeVisitor.hpp"

using namespace std;
using namespace antlr4;
using namespace eel;

#define TYPE_ANALYSIS(String) \
    ANTLRInputStream input(String); \
    eelLexer lexer(&input); \
    CommonTokenStream tokens(&lexer); \
    tokens.fill(); \
    eelParser parser(&tokens); \
    eelParser::ProgramContext* tree = parser.program(); \
    SymbolTable table; \
    ScopeVisitor scope_visitor(&table); \
    scope_visitor.visitProgram(tree); \
    TypeVisitor type_visitor(&table); \
    type_visitor.visitProgram(tree); \

TEST_CASE("Literal expressions", "[type_analysis]"){
    TYPE_ANALYSIS("setup{ 2 + 3; 2 + 'c'  + 2.0 + false + \"oh no\"; }");
    REQUIRE(type_visitor.errors.size() == 4);
    for(const auto& e : type_visitor.errors)
        REQUIRE(e.kind == Error::Kind::TypeMisMatch);
}

TEST_CASE("void function","[type_analysis]") {
    TYPE_ANALYSIS("setup{ return true; } loop{ return; }")
    REQUIRE(type_visitor.errors.size() == 1);
    REQUIRE(type_visitor.errors.at(0).kind == Error::Kind::InvalidReturnType);
}

TEST_CASE("function with return value", "[type_analysis]"){
    // TODO: Replace with function declaration
    // Since we don't have functions it is substituted with an event function
    // Though this fixed a bug :)
    TYPE_ANALYSIS("event x { return true; } event y { return \"oh no\"; }")
    REQUIRE(type_visitor.errors.size() == 1);
    REQUIRE(type_visitor.errors.at(0).kind == Error::Kind::InvalidReturnType);
}



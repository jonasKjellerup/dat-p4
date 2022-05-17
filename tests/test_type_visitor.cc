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

TEST_CASE("Variable declaration","[type_analysis]"){
    TYPE_ANALYSIS("setup{ u8 x = 2; static f32 y = 2.2; const bool z = false; }"
                  "loop{ u8 x = false; static f32 y = 2; const bool z = 1; }")
    REQUIRE(type_visitor.errors.size() == 3);
    for(const auto& e : type_visitor.errors)
        REQUIRE(e.kind == Error::Kind::TypeMisMatch);
}

TEST_CASE("Binary Expression", "[type_analysis]"){
    TYPE_ANALYSIS("setup { u8 x; u8 y; x + y; }"
                  "loop { u8 x; bool y; x+y; }")
    REQUIRE(type_visitor.errors.size() == 1);
    REQUIRE(type_visitor.errors.at(0).kind == Error::Kind::TypeMisMatch);
}

TEST_CASE("Comparison Expression", "[type_analysis]"){
    TYPE_ANALYSIS("setup { u8 x;  x == 1; }"
                  "loop {  u8 x;  x == 'c'; }")
    REQUIRE(type_visitor.errors.size() == 1);
    REQUIRE(type_visitor.errors.at(0).kind == Error::Kind::TypeMisMatch);
}

TEST_CASE("Logical Expression", "[type_analysis]"){
    TYPE_ANALYSIS("setup { bool x;  x && true; }"
                  "loop {  u8 x;  x || false; }")
    REQUIRE(type_visitor.errors.size() == 1);
    REQUIRE(type_visitor.errors.at(0).kind == Error::Kind::TypeMisMatch);
}

TEST_CASE("Binary assigment expression", "[type_analysis]"){
    TYPE_ANALYSIS("setup { u8 x;  x += 2; }"
                  "loop {  u8 x;  x -= 3.0; }")
    REQUIRE(type_visitor.errors.size() == 1);
    REQUIRE(type_visitor.errors.at(0).kind == Error::Kind::TypeMisMatch);
}

TEST_CASE("pin declaration", "[type_analysis]"){
    TYPE_ANALYSIS("setup { pin x analog(2); pin y digital(5); pin z digital(false); }")
    REQUIRE(type_visitor.errors.size() == 1);
    REQUIRE(type_visitor.errors.at(0).kind == Error::Kind::TypeMisMatch);
}

TEST_CASE("pin statements","[type_analysis]"){
    TYPE_ANALYSIS("setup{ digital x;  set x pin 1; set x 2; set x mode 1; set x mode 4.0; }"
                  "loop { u8 y; set y mode 0; }")
    REQUIRE(type_visitor.errors.size() == 2);
    REQUIRE(type_visitor.errors.at(0).kind == Error::Kind::TypeMisMatch);
    REQUIRE(type_visitor.errors.at(1).kind == Error::Kind::TypeMisMatch);
}

TEST_CASE("condition blocks", "[type_analysis]"){
    TYPE_ANALYSIS("setup{bool t = true; if(2==3){} if(t){} if(2){} }")
    REQUIRE(type_visitor.errors.size() == 1);
    REQUIRE(type_visitor.errors.at(0).kind == Error::Kind::TypeMisMatch);
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
    TYPE_ANALYSIS("event x { return true; } event y {bool t = true; return t;} event z { return \"oh no\"; }")
    REQUIRE(type_visitor.errors.size() == 1);
    REQUIRE(type_visitor.errors.at(0).kind == Error::Kind::InvalidReturnType);
}
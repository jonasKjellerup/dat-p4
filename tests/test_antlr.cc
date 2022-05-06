#include <catch.hpp>
#include <string>
#include "antlr4-runtime.h"
#include "eelLexer.h"
#include "eelParser.h"
#include "ScopeVisitor.hpp"

using namespace std;
using namespace antlr4;
using namespace eel;

#define GET_TOKENS(String, Result) \
    ANTLRInputStream input(String); \
    eelLexer lexer(&input); \
    CommonTokenStream tokens(&lexer); \
    tokens.fill(); \
    *(Result) = tokens.getTokens(); \

static void compareTokens(vector<Token*> *source, vector<size_t> *expected){
    expected->push_back(eelLexer::EOF);
    REQUIRE(source->size() == expected->size());
    for(size_t i = 0; i < source->size(); i++) {
        REQUIRE(source->at(i)->getType() == expected->at(i));
    }
}

// Any bugs encountered should be added to the tests cases to prevent it from reoccurring.

// TODO: Remove anonymous tokens.
TEST_CASE("integer literal", "[Lexer]") {
    vector<Token*> source;
    GET_TOKENS("1 2 3 4 5 6 7 8 9 0 99 0xFF 0xA9", &source)
    vector<size_t> expected( 13, eelLexer::IntegerLiteral);
    compareTokens(&source, &expected);
}

TEST_CASE("float literal", "[Lexer]" ) {
    vector<Token*> source;
    GET_TOKENS("1.0 1.9 .0 66.88", &source)
    vector<size_t> expected( 4, eelLexer::FloatLiteral);
    compareTokens(&source, &expected);
}

TEST_CASE("Mixed float/integer literals", "[Lexer]" ) {
    vector<Token*> source;
    GET_TOKENS(" 1 .0 2 .3 2 4.5 99 33.4 0x5F 0.99", &source)
    vector<size_t> expected = { eelLexer::IntegerLiteral, eelLexer::FloatLiteral,
                                eelLexer::IntegerLiteral, eelLexer::FloatLiteral,
                                eelLexer::IntegerLiteral, eelLexer::FloatLiteral,
                                eelLexer::IntegerLiteral, eelLexer::FloatLiteral,
                                eelLexer::IntegerLiteral, eelLexer::FloatLiteral,};
    compareTokens(&source, &expected);
}


TEST_CASE("Character literals", "[Lexer]" ){
    vector<Token*> source;
    GET_TOKENS("'a' ' ' '\"' 'b' 'z' '9' '0' '\\r' '\\t' '\\n' '\\v' '\\xFF08A' '\\\\' '\\\'' '\\' ", &source)
    vector<size_t> expected( 15, eelLexer::CharLiteral);
    compareTokens(&source, &expected);
}

TEST_CASE("String literals", "[Lexer]" ){
    vector<Token*> source;
    GET_TOKENS("\" !#L~\\n\" \"'s' 'v'\" \"1234567890\\\\ \\v \\r \\t \\\" \" ", &source)
    vector<size_t> expected( 3, eelLexer::StringLiteral);
    compareTokens(&source, &expected);
}

TEST_CASE("Bool literals", "[Lexer]" ){
    vector<Token*> source;
    GET_TOKENS("true false", &source)
    vector<size_t> expected( 2, eelLexer::BoolLiteral);
    compareTokens(&source, &expected);
}

TEST_CASE("Identifier", "[Lexer]" ){
    vector<Token*> source;
    GET_TOKENS("a xy x_z AZ x0 a9 if switch continue", &source)
    vector<size_t> expected = {eelLexer::Identifier, eelLexer::Identifier,
                               eelLexer::Identifier, eelLexer::Identifier,
                               eelLexer::Identifier, eelLexer::Identifier,
                               eelLexer::If, eelLexer::Switch,
                               eelLexer::Continue};
    compareTokens(&source, &expected);
}

TEST_CASE("Comments", "[Lexer]" ){
    vector<Token*> source;
    GET_TOKENS("// test duaiud//9qpoqensaiudbasda\n //test \n /* very \n  multi \n line \n comments */", &source)
    vector<size_t> expected = {};
    compareTokens(&source, &expected);
}

TEST_CASE("T","T") {
    ANTLRInputStream input("const u8 t = 9;const u16 p = 69;");
    eelLexer lexer(&input); \
    CommonTokenStream tokens(&lexer);
    tokens.fill();
    eelParser parser(&tokens);
    auto tree = parser.program();
    ScopeVisitor scope;
    scope.visitProgram(tree);
}

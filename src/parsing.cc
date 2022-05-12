#include <iostream>
#include <fstream>
#include "antlr4-runtime.h"
#include "eelLexer.h"
#include "eelParser.h"

namespace eel {
    void parse(std::ifstream& file) {
        if (file.is_open()) {
            antlr4::ANTLRInputStream input(file);
            eelLexer lexer(&input);
            antlr4::CommonTokenStream tokens(&lexer);

            eelParser parser(&tokens);
            eelParser::ProgramContext* tree = parser.program();

        }
    }
}
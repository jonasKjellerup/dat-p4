#include <iostream>
#include <cxxopts.hpp>
#include <fmt/core.h>

#include <antlr4-runtime.h>
#include <eelLexer.h>
#include <eelParser.h>

#include <symbol_table.hpp>
#include <Visitors/ScopeVisitor.hpp>
#include <Visitors/TypeVisitor.hpp>
#include <Visitors/codegen.hpp>

static auto check_command(const char*) -> bool;

static void process_file(std::fstream& input_file, FILE* output_file);

auto main(int argc, char** argv) -> int {
    const std::string ProgramName = "eel";
    const std::string ProgramDescription = "Compiler for the ?? language";

    cxxopts::Options options(ProgramName, ProgramDescription);
    options.add_options()
            ("h,help", "Display help text", cxxopts::value<bool>())
            ("t,target", "Select target platform (default: avr)", cxxopts::value<std::string>())
            ("f,file", "Sets source file path", cxxopts::value<std::string>())
            ("list-targets", "Lists available target platforms", cxxopts::value<bool>());

    auto opts = options.parse(argc, argv);

    if (opts.count("help") > 0) {
        std::cout << options.help() << std::endl;
        return 0;
    }
    if (opts.count("file") == 0){
        std::cout << "Source file path is required." << std::endl;
        return 0;
    }

    auto& input_path = opts["file"].as<std::string>();
    auto output_path = fmt::format("{}.cc", input_path);

    std::fstream input_file(input_path);
    auto output_file = fopen(output_path.c_str(), "w");

    process_file(input_file, output_file);

    fclose(output_file);

    return 0;
}

auto check_command(const char* command) -> bool {
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command, "r"), pclose);
    return pipe != nullptr;
}

void process_file(std::fstream& input_file, FILE* output_file) {
    antlr4::ANTLRInputStream input(input_file);
    eel::eelLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);
    tokens.fill();

    eel::eelParser parser(&tokens);
    auto tree = parser.program();
    eel::SymbolTable symbol_table;

    ScopeVisitor(&symbol_table).visitProgram(tree);
    TypeVisitor(&symbol_table).visitProgram(tree);
    visitors::CodegenVisitor(symbol_table, output_file)
        .visitProgram(tree);

}
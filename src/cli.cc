#include <iostream>
#include <cxxopts.hpp>
#include <fmt/core.h>
#include <fmt/ostream.h>

#include <antlr4-runtime.h>
#include <eelLexer.h>
#include <eelParser.h>

#include <symbol_table.hpp>
#include <Visitors/ScopeVisitor.hpp>
#include <Visitors/TypeVisitor.hpp>
#include <Visitors/codegen.hpp>

struct BuildOptions {
    bool testing;
};

static void process_file(std::fstream& input_file, std::fstream&  output_file, BuildOptions& options);

static void register_test_library(SymbolTable& table);

auto main(int argc, char** argv) -> int {
    const std::string ProgramName = "eelc";
    const std::string ProgramDescription = "Compiler for the EEL language";

    BuildOptions buildOptions {};

    cxxopts::Options options(ProgramName, ProgramDescription);
    options.add_options()
            ("h,help", "Display help text", cxxopts::value<bool>())
            ("t,target", "Select target platform (default: avr)", cxxopts::value<std::string>())
            ("f,file", "Sets source file path", cxxopts::value<std::string>())
            ("list-targets", "Lists available target platforms", cxxopts::value<bool>())
            ("test", "Enables use of the testing library", cxxopts::value<bool>());

    auto opts = options.parse(argc, argv);

    if (opts.count("help") > 0) {
        std::cout << options.help() << std::endl;
        return 0;
    }

    if (opts.count("file") == 0){
        std::cout << "Source file path is required." << std::endl;
        return 0;
    }

    if (opts.count("test") > 0) {
        buildOptions.testing = true;
    }

    auto& input_path = opts["file"].as<std::string>();
    auto output_path = fmt::format("{}.cc", input_path);

    std::fstream input_file(input_path);
    std::fstream output_file(output_path, std::fstream::out);

    process_file(input_file, output_file, buildOptions);

    input_file.close();
    output_file.close();

    return 0;
}

void process_file(std::fstream& input_file, std::fstream& output_file, BuildOptions& options) {
    antlr4::ANTLRInputStream input(input_file);
    eel::eelLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);
    tokens.fill();

    eel::eelParser parser(&tokens);
    auto tree = parser.program();
    eel::SymbolTable symbol_table;

    auto scope_visitor = ScopeVisitor(&symbol_table);

    if (options.testing)
        register_test_library(symbol_table);

    scope_visitor.visitProgram(tree);
    TypeVisitor(&symbol_table).visitProgram(tree);
    visitors::CodegenVisitor(symbol_table, &output_file)
        .visitProgram(tree);

}

void register_test_library(SymbolTable& table) {
    auto s = table.root_scope;
    auto bool_type = table.get_symbol(symbols::Primitive::boolean.id);

    s->declare_fn_cpp("assert_true", "assert_true", {}, bool_type);
    s->declare_fn_cpp("assert_false", "assert_false", {}, bool_type);
}
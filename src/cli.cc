#include <iostream>
#include <cxxopts.hpp>

static auto check_command(const char*) -> bool;

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
    std::cout << opts["file"].as<std::string>() << std::endl;

    using std::cout;
    cout << "Hello world!" << std::endl;
    return 0;
}

auto check_command(const char* command) -> bool {
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command, "r"), pclose);
    return pipe != nullptr;
}
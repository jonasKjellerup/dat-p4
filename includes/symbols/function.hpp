#pragma once

#include <vector>
#include <string>
#include <symbol_table.hpp>

namespace eel::symbols {
    struct Sequence;
    struct Function {
    public:
        Scope scope;
        Symbol return_type;
        std::vector<Symbol> parameters;

        bool has_return_type;
        bool is_async = false;
        Sequence* sequence = nullptr;


        std::string type_id;

    };
}
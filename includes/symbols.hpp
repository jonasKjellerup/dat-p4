#pragma once

#include <symbol_table.hpp>
#include <vector>

namespace eel::symbols {
    struct Functions{
    public:
        Scope scope;
        Symbol return_type;
        std::vec<Symbol> parameter_types;

        bool has_return_type;

    };
}

//return_type function_id ( parameter list ) { *body }
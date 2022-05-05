#pragma once
#include <symbol_table.hpp>

namespace eel::symbols {

    struct Variable {
        struct Value {};
    public:
         Symbol type;
         Value value;
         bool has_value;
         bool is_static;
    };

}




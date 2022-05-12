#pragma once
#include <symbol_table.hpp>

namespace eel::symbols {

    // const T x = ?;
    struct Constant {
        struct Value {};
    public:
        Symbol type;
        Value value;
    };
}

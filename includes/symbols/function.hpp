#pragma once

#include <vector>
#include <string>
#include <symbol_table.hpp>
#include <eelParser.h>

namespace eel {
    struct Sequence;
}

namespace eel::symbols {

    struct Function {
    public:
        Scope scope;
        Symbol return_type;
        std::vector<Symbol> parameters;

        bool has_return_type;
        Sequence* sequence = nullptr;
        std::string type_id;

        eel::eelParser::StmtBlockContext* body = nullptr;

        [[nodiscard]] bool is_async() const;

    };
}
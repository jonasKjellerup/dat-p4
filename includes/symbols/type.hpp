#pragma once

#include <symbol_table.hpp>

namespace eel::symbols {

    struct Type {
    public:
        enum struct Kind {
            Primitive,
            Struct,
            Union,
            UntaggedUnion,
            Enum,
            Trait,
        };

        Kind kind;

    };

    struct Primitive : public Type {};

    struct Struct : public Type {
    public:
        Struct();
        Scope memberScope;
    };

}
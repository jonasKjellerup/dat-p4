#pragma once

#include <string>
#include <hash_map>
#include <vector>

namespace eel {
    // Forward declaration of SymbolTable
    struct SymbolTable;

    struct Symbol {
        using Id = uint32_t;

        enum Kind {
            Variable = 1,
            Constant = 2,
            Function = 4,
            Type = 8,
        };

        struct Expected {
            bool is_static;
        };

        union Value {
            Expected expected;
        };

        Id id;
        Kind kind;
        Value value;
        std::string name;
    };

    struct Scope {
    public:
        using Id = size_t;

        Scope(SymbolTable*, Id);
        Scope(SymbolTable*, Scope*, Id);

        /// Mark a symbol as expected in the given context
        void expect();
        void declare_const();
        void declare_var();
        void declare_type();
        void declare_func();

        bool is_root();
    private:
        Scope* parent;
        SymbolTable* context;
    };

    struct SymbolTable {
    public:
        SymbolTable();

        /// Derives a new scope from the root scope
        Scope* derive_scope();
        /// Derives a new scope from the given scope
        Scope* derive_scope(Scope*);

    private:
        std::unordered_map<Symbol::Id, Symbol> symbols;
        std::vector<Scope> scopes;
    };


}
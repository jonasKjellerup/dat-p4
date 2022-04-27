#pragma once

#include <string>
#include <hash_map>
#include <vector>

namespace eel {
    // Forward declaration of SymbolTable
    struct SymbolTable;

    struct Symbol {
        using Id = uint32_t;

        enum struct Kind {
            Variable = 1,
            Constant = 2,
            Function = 4,
            Type = 8,
            Namespace = 16,
            Expected = -1,
        };

        /// \brief Represents an expected symbol.
        /// Expected symbols are used when a symbol
        /// is referenced before being declared.
        struct Expected {
            /// \brief The kinds of symbols that are valid
            ///        in the context where the symbol was used.
            /// The value will be interpreted as a series of
            /// flags, such that cases where more than one
            /// kind is valid can be handle.
            Kind kinds;

            /// \brief Determines whether the symbol must
            ///        be declared as static.
            bool must_be_static;
        };

        union Value {
            Expected expected;
        };

        Id id;
        Kind kind;
        Value value;
        std::string name;
    };

    struct Scope_ {
    public:
        using Id = size_t;

        Scope_(SymbolTable*, Id);
        Scope_(SymbolTable*, Scope_*, Id);

        /// Mark a symbol as expected in the given context
        void expect();
        void declare_const();
        void declare_var();
        void declare_type();
        void declare_func();

        bool is_root();
    private:
        Scope_* parent;
        SymbolTable* context;
    };

    /// \brief An indirect pointer to a `Scope_` instance.
    /// Structure dereference is overloaded such that
    /// it dereferences to a `Scope_` instance.
    ///
    /// This helps avoid invalid pointers due to
    /// reallocation of the scope tables.
    struct Scope {
    public:
        Scope_* operator->();
    private:
        Scope_::Id id;
        SymbolTable* table;
    };

    struct SymbolTable {
    public:
        SymbolTable();

        /// \brief Derives a new scope from the root scope.
        /// \returns A pointer to the newly created scope.
        /// \note The created scope can not outlive the
        ///       symbol table instance.
        /// \sa derive_scope(Scope_*)
        Scope_* derive_scope();

        /// \brief Derives a new scope from the given scope.
        /// \returns A pointer to the newly created scope.
        /// \note The created scope can not outlive the
        ///       symbol table instance.
        /// \sa derive_scope()
        Scope_* derive_scope(Scope_*);

        friend Scope;

    private:
        std::unordered_map<Symbol::Id, Symbol> symbols;
        std::vector<Scope_> scopes;
    };


}
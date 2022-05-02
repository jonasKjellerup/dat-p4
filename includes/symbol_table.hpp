#pragma once

#include <string>
#include <hash_map>
#include <vector>

// Temporary declarations of SymbolDefinitions
namespace symbol_definitions {
    struct Variable {};
    struct Constant {};
    struct Function {};
    struct Type {};
    struct Namespace {

    };
}

namespace eel {
    // Forward declarations
    struct SymbolTable;
    struct Scope_;

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
            ///        be declared as a static symbol.
            bool must_be_static;
        };

        union Value {
            Expected expected;
            symbol_definitions::Variable variable;
        };

        Id id;
        Kind kind;
        Value value;
        std::string name;
    };

    /// \brief An indirect pointer to a `Scope_` instance.
    /// Structure dereference is overloaded such that
    /// it dereferences to a `Scope_` instance.
    ///
    /// This helps avoid invalid pointers due to
    /// reallocation of the scope tables.
    struct Scope {
    public:
        using Id = size_t;

        Scope() = default;
        explicit Scope(Scope_*);
        Scope_* operator->();

        bool is_nullptr();
    private:
        Id id;
        SymbolTable* table;
    };

    struct Scope_ {
    public:
        using Id = Scope::Id;
        friend Scope;

        Scope_(SymbolTable*, Id);
        Scope_(SymbolTable*, Scope, Id);

        explicit operator Scope();

        /// \brief Find a symbol by name within the scope.
        /// \returns A pointer to the symbol if found.
        ///          `nullptr` is returned otherwise.
        Symbol* find(std::string&);

        /// Mark a symbol as expected in the given context
        void expect();
        void declare_const();
        void declare_var(const std::string&);
        void declare_type();
        void declare_func();
        void declare_namespace();

        bool is_root();
    private:
        Id id;
        std::unordered_map<std::string, Symbol::Id> symbol_map;
        Scope parent{};
        SymbolTable* context;
    };



    /// \brief A table for managing program symbols.
    /// It should be noted that the lifetime of any
    /// scope or symbol created by a symbol table
    /// is intrinsically tied to that of the table.
    ///
    /// As such any reference/pointer to a scope or symbol
    /// are only valid for the duration of the tables lifetime.
    /// (this includes instances of the `Scope` type
    /// which is just a "fat pointer" to a scope)
    struct SymbolTable {
    public:
        SymbolTable();

        /// \brief Creates a new blank symbol.
        /// \returns A reference to the newly created symbol.
        Symbol& new_symbol();
        Symbol* get_symbol(Symbol::Id id);

        Scope root_scope;

        /// \brief Derives a new scope from the root scope.
        /// \returns A pointer to the newly created scope.
        Scope derive_scope();

        /// \brief Derives a new scope from the given scope.
        /// \param scope A direct pointer enclosing scope
        /// \returns A pointer to the newly created scope.
        Scope derive_scope(Scope_* scope);

        /// \brief Derives a new scope from the given scope.
        /// \param scope An indirect pointer enclosing scope
        /// \returns A pointer to the newly created scope.
        Scope derive_scope(Scope scope);

        Scope get_scope(Scope::Id id);
        Scope_* get_scope_raw(Scope::Id id);

    private:
        std::vector<Symbol> symbols;
        std::vector<Scope_> scopes;
    };


}
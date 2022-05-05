#pragma once

#include <string>
#include <unordered_map>
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

    /// \brief An indirect pointer to an object owned by a `SymbolTable`.
    /// A generic type for referencing objects stored within
    /// a symbol table.
    ///
    /// The type requires a specialized implementation of the `->` operator
    /// for each type `T` that is to be used with this type.
    template<typename T>
    struct TablePtr {
    public:
        using Id = size_t;
        /// \brief A convenience alias of the direct/raw equivalent to TablePtr type.
        using Raw = T*;

        TablePtr() = default;
        explicit TablePtr(T* ptr) {
            this->id = ptr->id;
            this->table = ptr->context;
        }

        TablePtr(Id id, SymbolTable* context) : id(id), table(context){}

        T* operator->();

        bool is_nullptr() {
            return this->table == nullptr;
        }

    private:
        Id id {};
        SymbolTable* table{};
    };

    struct Symbol_ {
        using Id = size_t;

        enum struct Kind {
            Variable = 1,
            Constant = 2,
            Function = 4,
            Type = 8,
            Namespace = 16,
            Indirect = 32,
        };

        struct Indirect {
            Kind kind;
            Id id;

            bool is_set() const;
        };

        union Value {
            Indirect indirect;
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
    using Scope = TablePtr<Scope_>;
    using Symbol = TablePtr<Symbol_>;


    struct Scope_ {
    public:
        using Id = size_t;
        friend Scope;

        Scope_(SymbolTable*, Id);
        Scope_(SymbolTable*, Scope, Id);

        explicit operator Scope();

        /// \brief Find a symbol by name within the scope.
        /// \returns A pointer to the symbol if found.
        ///          `nullptr` is returned otherwise.
        Symbol find(const std::string&);

        /// \brief Defers the declaration of a symbol for later.
        /// This is used whenever a symbol that has yet to be
        /// declared is to used. This results in an indirect symbol
        /// which will at some point (assuming that the correct symbol
        /// is declared) point to the expected symbol.
        Symbol defer_symbol(std::string name, Symbol_::Kind);
        void declare_const();
        void declare_var(const std::string&);
        void declare_type();
        void declare_func();
        void declare_namespace();

        bool is_root();
    private:
        Id id;
        std::unordered_map<std::string, Symbol_::Id> symbol_map;
        Scope parent{};
        SymbolTable* context;
    };

    /// \brief Info used to track unresolved symbols.
    struct UnresolvedSymbol {
        /// \brief The kind of symbol that is expected.
        Symbol_::Kind expected_kind;

        /// \brief The scope in which the symbol was found to be unresolvable.
        Scope::Id origin_scope;

        /// \brief The symbol that refers to the unresolved symbol.
        /// This is the symbol that any dependant symbols use
        /// to refer to the symbol without knowing whether it exists.
        Symbol::Id indirection_symbol;

        /// \brief The name of the symbol.
        std::string name;
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
        Symbol_& new_symbol();
        Symbol get_symbol(Symbol_::Id id);
        Symbol::Raw get_symbol_raw(Symbol::Id id);

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
        Scope::Raw get_scope_raw(Scope::Id id);

        /// \brief Attempts to resolve unresolved symbols.
        /// All symbols that are successfully resolved will
        /// be removed from the unresolved symbol list,
        /// allowing for the remaining symbols to be reported.
        void try_resolve_unresolved();

        void report_unresolved_symbol(UnresolvedSymbol&& symbol);

    private:
        std::vector<Symbol_> symbols;
        std::vector<Scope_> scopes;
        std::vector<UnresolvedSymbol> unresolved_symbols;
    };

}
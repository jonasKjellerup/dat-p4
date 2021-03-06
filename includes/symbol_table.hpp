#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include <symbols/forward_decl.hpp>
#include <error.hpp>

// Temporary declarations of SymbolDefinitions

namespace eel {
    // Forward declarations
    struct SymbolTable;
    struct Scope_;
    struct Symbol_;

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
        const T* operator->() const {
            // We cast away the constness to reuse the specialisations of `T* operator->();`
            auto ptr = const_cast<eel::TablePtr<T>*>(this);
            return static_cast<const T*>(ptr->operator->());
        }

        bool operator==(const eel::TablePtr<T>& other) const {
            return table == other.table && id == other.id;
        }

        [[nodiscard]] bool is_nullptr() const {
            return this->table == nullptr;
        }

    private:
        Id id {};
        SymbolTable* table{};
    };

    /// \brief An indirect pointer to a `Scope_` instance.
    /// Structure dereference is overloaded such that
    /// it dereferences to a `Scope_` instance.
    ///
    /// This helps avoid invalid pointers due to
    /// reallocation of the scope tables.
    using Scope = TablePtr<Scope_>;
    using Symbol = TablePtr<Symbol_>;

    struct Symbol_ {
        using Id = size_t;

        enum struct Kind {
            None = 0,
            Variable,
            Constant,
            Function,
            ExternFunction,
            Type,
            Namespace,
            Event,
            Indirect,
        };

        struct Indirect {
            Kind kind;
            Id id;

            [[nodiscard]] bool is_set() const;
        };

        union Value {
            struct None {};
            None none {};

            Indirect indirect;
            Scope namespace_;

            // TODO: figure out how we want to handle
            //       deallocation of these pointers
            symbols::Variable* variable;
            symbols::Constant* constant;
            symbols::Event* event;
            symbols::Function* function;
            symbols::ExternalFunction* extern_function;
            // NOTE: can't naively deallocate this one since
            //       some types (primitives) have static storage
            symbols::Type* type;
        };

        Id id;
        Kind kind;
        Value value {};
        std::string name;
    };



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

        /// \brief Find a symbol by name in the scope without inspecting precursors.
        /// \returns A pointer to the symbol if found.
        ///          `nullptr` is returned otherwise.
        /// This is mainly intended for use when trying to access
        /// members of an object (a.x) or namespace (a::x).
        Symbol find_member(const std::string&);

        /// \brief Defers the declaration of a symbol for later.
        /// This is used whenever a symbol that has yet to be
        /// declared is to used. This results in an indirect symbol
        /// which will at some point (assuming that th
        /// e correct symbol
        /// is declared) point to the expected symbol.
        Symbol defer_symbol(std::string name, Symbol_::Kind);

        symbols::Variable* declare_var(Symbol& type, const std::string& name, bool is_static = false);

        struct ConstExpr {}; // TODO replace with proper type
        void declare_const(Symbol& type, const std::string& name, ConstExpr expr);

        /// \brief Registers an already existing type.
        void declare_type(symbols::Type*);

        /// \brief Declares an external (c++) function
        symbols::ExternalFunction* declare_fn_cpp_(
                const std::string& eel_name,
                const std::string& cpp_name,
                Symbol return_type,
                std::vector<Symbol>&& parameters
                );

        symbols::ExternalFunction* declare_fn_cpp_(
                const std::string& eel_name,
                const std::string& cpp_name,
                Symbol return_type
        );

        template<typename... Parameters>
        symbols::ExternalFunction* declare_fn_cpp(
                const std::string& eel_name,
                const std::string& cpp_name,
                Symbol return_type,
                Parameters... parameters
        ) {
            std::vector<Symbol> params {parameters...};
            return declare_fn_cpp_(eel_name, cpp_name, return_type, std::move(params));
        }

        Symbol declare_event(const std::string& name);
        Symbol declare_event(const std::string& name, symbols::Function* function);
        symbols::Event& declare_event_handle(const std::string& event_name, Error::Pos pos);

        /// \brief Declares a function by the given name, with no return type.
        /// The function is always declared in the root scope
        /// even when called on a non-root scope.
        /// \param name The name of the function that is being declared.
        /// \returns The newly created function symbol.
        Symbol declare_func(const std::string& name);

        /// \brief Declares a function by the given name.
        /// The function is always declared in the root scope
        /// event when called on a non-root scope.
        /// \param name The name of the function that is being declared.
        /// \param return_type A type symbol representing the return type.
        /// \returns The newly created function symbol.
        Symbol declare_func(const std::string& name, Symbol return_type);
        void declare_namespace(const std::string& name);

        bool is_root();

        Scope parent{};
    private:
        std::unordered_map<std::string, Symbol_::Id> symbol_map;
        SymbolTable* context;
        Id id;
    public:
        [[nodiscard]] const decltype(symbol_map)& members() const;
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
        size_t get_scope_count();

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
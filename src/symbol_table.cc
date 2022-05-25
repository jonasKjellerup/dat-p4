#include <symbol_table.hpp>
#include <symbols/type.hpp>
#include <symbols/variable.hpp>
#include <symbols/constant.hpp>
#include <symbols/event.hpp>

#include <utility>
#include <fmt/core.h>

using namespace eel;

/*
 * Template specialisations
 */

template<>
Scope_ *TablePtr<Scope_>::operator->() {
    return this->table->get_scope_raw(this->id);
}

template<>
Symbol_ *TablePtr<Symbol_>::operator->() {
    return this->table->get_symbol_raw(this->id);
}

/*
 * Scope_ implementation
 */

Scope_::Scope_(SymbolTable *context, Scope_::Id id) {
    this->context = context;
    this->id = id;
}

Scope_::Scope_(SymbolTable *context, Scope parent, Scope_::Id id) {
    this->context = context;
    this->id = id;
    this->parent = parent;
}

// Cast Scope_ to Scope
Scope_::operator Scope() {
    return Scope(this);
}

bool Scope_::is_root() {
    return this->parent.is_nullptr();
}

const decltype(Scope_::symbol_map) &Scope_::members() const {
    return this->symbol_map;
}


/*
 * Symbol table implementation
 */

SymbolTable::SymbolTable() {
    // Create the root scope
    this->scopes.emplace_back(this, 0);
    this->root_scope = this->get_scope(0);
}

Scope SymbolTable::derive_scope() {
    return this->derive_scope(&this->scopes[0]);
}

Scope SymbolTable::derive_scope(Scope_ *scope) {
    return this->derive_scope(static_cast<Scope>(scope));
}

Scope SymbolTable::derive_scope(Scope scope) {
    return static_cast<Scope>(this->scopes.emplace_back(this, scope, this->scopes.size()));
}

Scope SymbolTable::get_scope(Scope::Id id) {
    return Scope(this->get_scope_raw(id));
}

Scope_ *SymbolTable::get_scope_raw(Scope::Id id) {
    return &this->scopes.at(id);
}

Symbol_ &SymbolTable::new_symbol() {
    Symbol_ &symbol = this->symbols.emplace_back();
    symbol.id = this->symbols.size() - 1;

    return symbol;
}

Symbol SymbolTable::get_symbol(Symbol_::Id id) {
    // TODO check if id is valid
    return {id, this};
}

Symbol::Raw SymbolTable::get_symbol_raw(Symbol::Id id) {
    return &this->symbols[id];
}

bool Symbol_::Indirect::is_set() const {
    return this->id > 0;
}

/*
 * Symbol declaration
 */

symbols::Variable *Scope_::declare_var(Symbol &type, const std::string &name, bool is_static) {
    if (this->symbol_map.contains(name)) {
        // TODO throw exception
        return nullptr;
    }

    auto &symbol = this->context->new_symbol();
    symbol.kind = Symbol_::Kind::Variable;
    symbol.name = name;

    auto var = new symbols::Variable;
    var->type = type;
    var->is_static = is_static;
    var->has_value = false;

    symbol.value.variable = var;

    this->symbol_map.insert(std::make_pair(name, symbol.id));

    return var;
}

void Scope_::declare_const(Symbol &type, const std::string &name, ConstExpr expr) {
    if (this->symbol_map.contains(name)) {
        // TODO throw exception
    }

    auto &symbol = this->context->new_symbol();
    symbol.kind = Symbol_::Kind::Variable;
    symbol.name = name;

    auto var = new symbols::Constant;
    var->type = type;

    symbol.value.constant = var;

    this->symbol_map.insert(std::make_pair(name, symbol.id));
}

void Scope_::declare_type(symbols::Type *type) {
    auto &name = type->type_source_name(); // Changed since find would be unable to find digital or analog variable declarations.
    if (this->symbol_map.contains(name)) {
        // TODO throw exception
    }

    auto &symbol = this->context->new_symbol();
    symbol.kind = Symbol_::Kind::Type;
    symbol.name = name;
    symbol.value.type = type;

    if (type->kind == symbols::Type::Kind::Primitive) {
        auto p = dynamic_cast<symbols::Primitive *>(type);
        p->id = symbol.id;
    }

    this->symbol_map.insert(std::make_pair(name, symbol.id));
}

void Scope_::declare_namespace(const std::string &name) {
    if (this->symbol_map.contains(name)) {
        // TODO throw exception
    }

    auto &symbol = this->context->new_symbol();
    symbol.kind = Symbol_::Kind::Type;
    symbol.name = name;
    symbol.value.namespace_ = this->context->derive_scope(this);

    this->symbol_map.insert(std::make_pair(name, symbol.id));
}

static Symbol_ &create_event_symbol(SymbolTable *context, const std::string &name) {
    auto &symbol = context->new_symbol();
    symbol.kind = Symbol_::Kind::Event;
    symbol.name = name;
    symbol.value.event = new symbols::Event;
    symbol.value.event->id = fmt::format("event{}", symbol.id);

    return symbol;
}

static Symbol_ &create_function_symbol(SymbolTable *context, const std::string &name) {
    auto &symbol = context->new_symbol();
    symbol.kind = Symbol_::Kind::Function;
    symbol.name = name;
    symbol.value.function = new symbols::Function;
    return symbol;
}

Symbol Scope_::declare_event(const std::string &name) {
    auto root = this->context->root_scope;
    auto symbol = root->find(name);

    // If a symbol already exists
    if (!symbol.is_nullptr()) {
        if (symbol->kind != Symbol_::Kind::Event) {
            // TODO throw error
            return {};
        }
    } else {
        auto &symbol_ref = create_event_symbol(this->context, name);
        symbol = Symbol(symbol_ref.id, this->context);
        root->symbol_map.insert(std::make_pair(name, symbol->id));
    }
    auto &event = symbol->value.event;


    event->has_predicate = false;
    event->is_awaited = false;
    event->is_complete = true;

    return symbol;
}

Symbol Scope_::declare_func(const std::string &name) {
    auto root = this->context->root_scope;
    auto symbol = root->find(name);

    // If a symbol already exists
    if (!symbol.is_nullptr()) {
        // TODO throw error
        return {};
    } else {
        auto &symbol_ref = create_function_symbol(this->context, name);
        symbol = Symbol(symbol_ref.id, this->context);
        root->symbol_map.insert(std::make_pair(name, symbol->id));
    }
    auto &function = *symbol->value.function;
    function.scope = this->context->derive_scope(root);
    function.parameters = std::vector<Symbol>();
    function.type_id = fmt::format("func{}_{}", symbol->id, name);

    return symbol;
}

symbols::ExternalFunction *
Scope_::declare_fn_cpp(
        const std::string &eel_name,
        const std::string &cpp_name,
        Symbol return_type,
        std::vector<Symbol>&& parameters
) {
    auto root = this->context->root_scope;
    auto symbol = root->find(eel_name);

    if (!symbol.is_nullptr()) {
        throw InternalError(InternalError::SymbolTable,
                            fmt::format("Invalid external function declaration of `{}`,"
                                        " symbol by that name already exists.",
                                        eel_name));
    } else {
        symbol = Symbol(context->new_symbol().id, context);
        symbol->kind = Symbol_::Kind::ExternFunction;
        symbol->name = eel_name;
        auto fn = symbol->value.extern_function = new symbols::ExternalFunction;

        fn->_target_id = cpp_name;
        fn->return_type = return_type;
        fn->parameters = parameters;

        return fn;
    }

    return nullptr;
}

symbols::ExternalFunction *
Scope_::declare_fn_cpp(
        const std::string &eel_name,
        const std::string &cpp_name,
        Symbol return_type
) {
    return declare_fn_cpp(eel_name, cpp_name, return_type, {});
}

Symbol Scope_::declare_func(const std::string &name, Symbol return_type) {
    auto root = this->context->root_scope;
    auto symbol = root->find(name);

    // If a symbol already exists
    if (!symbol.is_nullptr()) {
        // TODO throw error
        return {};
    } else if (return_type.is_nullptr() || return_type->kind != Symbol_::Kind::Type) {
        // if the return symbol is not a type
        // TODO throw error
        return {};
    } else {
        auto &symbol_ref = create_function_symbol(this->context, name);
        symbol = Symbol(symbol_ref.id, this->context);
        root->symbol_map.insert(std::make_pair(name, symbol->id));
    }
    auto function = symbol->value.function;
    function->return_type = return_type;
    function->scope = this->context->derive_scope(root);
    function->parameters = std::vector<Symbol>();

    return symbol;
}

Symbol Scope_::declare_event(const std::string &name, symbols::Function *function) {
    auto root = this->context->root_scope;
    auto symbol = root->find(name);

    // If a symbol already exists
    if (!symbol.is_nullptr()) {
        if (symbol->kind != Symbol_::Kind::Event) {
            // TODO throw error
            return {};
        }

        if (symbol->value.event->is_complete) {
            // TODO throw error
            return {};
        }

        if (function != nullptr) {
            symbol->value.event->predicate = function;
            symbol->value.event->has_predicate = true;
        }

        symbol->value.event->is_complete = true;
    } else {
        auto &symbol_ref = create_event_symbol(this->context, name);
        symbol = Symbol(symbol_ref.id, this->context);
        root->symbol_map.insert(std::make_pair(name, symbol->id));
    }
    auto &event = symbol->value.event;
    event->predicate = function;

    event->has_predicate = true;
    event->is_awaited = false;
    event->is_complete = true;

    return symbol;
}

symbols::Event &Scope_::declare_event_handle(const std::string &event_name, Error::Pos pos) {
    auto root = this->context->root_scope;
    auto event_symbol = root->find(event_name);

    // if the event_symbol was not found create it and mark as incomplete
    if (event_symbol.is_nullptr()) {
        event_symbol = root->declare_event(event_name);
        event_symbol->value.event->is_complete = false;
    }

    event_symbol->value.event->add_handle(root, pos);
    return *event_symbol->value.event;
}

/*
 * Symbol resolution
 */

Symbol Scope_::find(const std::string &name) {
    auto s = this->symbol_map.find(name);
    if (s == this->symbol_map.end()) {
        if (!this->is_root())
            return this->parent->find(name);
        else return {};
    }
    return this->context->get_symbol(s->second);
}

Symbol Scope_::find_member(const std::string &name) {
    auto s = this->symbol_map.find(name);
    if (s == this->symbol_map.end())
        return {};
    return this->context->get_symbol(s->second);
}

Symbol Scope_::defer_symbol(std::string name, Symbol_::Kind kind) {
    // Throw error if we are trying to defer an already existing symbol.
    if (this->symbol_map.contains(name)) {
        // TODO throw exception (this is considered an internal error, not user source error)
    }

    // TODO: figure out if the symbol needs to be registered in
    //       the scopes symbol map.
    //       The main considerations are as follows:
    //          - Multiple uses of the same symbol will result in
    //            duplicate indirection symbols. (Without registration)
    //          - (With registration) an resolution conflict can occur
    //            between a outer static and a inner non-static decl.

    Symbol_ &s = this->context->new_symbol();
    s.name = name;
    s.kind = Symbol_::Kind::Indirect;
    s.value.indirect = {kind, 0};

    this->context->report_unresolved_symbol({
                                                    kind,
                                                    this->id,
                                                    s.id,
                                                    name,
                                            });

    return this->context->get_symbol(s.id);
}

void SymbolTable::report_unresolved_symbol(UnresolvedSymbol &&symbol) {
    this->unresolved_symbols.emplace_back(symbol);
}

/// \brief Attempts to resolve an unresolved symbol.
/// \returns True if the symbol was resolved. Otherwise false.
static bool try_resolve(const UnresolvedSymbol &symbol, SymbolTable *context) {
    bool resolved;
    auto scope = context->get_scope(symbol.origin_scope);
    auto matching_symbol = scope->find(symbol.name);

    resolved = !matching_symbol.is_nullptr() // was a matching symbol found
               && matching_symbol->kind == symbol.expected_kind // is it of same kind
               && (symbol.expected_kind != Symbol_::Kind::Variable // is it not a variable
                   || matching_symbol->value.variable->is_static); // or is it static

    if (resolved) {
        auto indirection_symbol = context->get_symbol(symbol.indirection_symbol);
        indirection_symbol->value.indirect.id = matching_symbol->id;
    }

    return resolved;
}

void SymbolTable::try_resolve_unresolved() {
    // Removes any symbol that can be resolved by `try_resolve`
    // from the list of unresolved symbols.
    this->unresolved_symbols.erase(
            std::remove_if(
                    this->unresolved_symbols.begin(),
                    this->unresolved_symbols.end(),
                    [this](const UnresolvedSymbol &symbol) { return try_resolve(symbol, this); }),
            this->unresolved_symbols.end()
    );
}

size_t SymbolTable::get_scope_count() {
    return this->scopes.size();
}

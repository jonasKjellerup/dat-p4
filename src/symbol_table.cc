#include <symbol_table.hpp>

using namespace eel;

/*
 * Template specialisations
 */

template<>
Scope_* TablePtr<Scope_>::operator->() {
    return this->table->get_scope_raw(this->id);
}

template<>
Symbol_* TablePtr<Symbol_>::operator->() {
    return this->table->get_symbol_raw(this->id);
}

/*
 * Scope_ implementation
 */

Scope_::Scope_(SymbolTable* context, Scope_::Id id) {
    this->context = context;
    this->id = id;
}

Scope_::Scope_(SymbolTable* context, Scope parent, Scope_::Id id) {
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

void Scope_::declare_var(const std::string& name) {
    if (this->symbol_map.contains(name)) {
        // TODO throw exception
    }

    Symbol_& symbol = this->context->new_symbol();
    symbol.kind = Symbol_::Kind::Variable;
    symbol.name = name;

    this->symbol_map.insert(std::make_pair(name, symbol.id));
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

Scope SymbolTable::derive_scope(Scope_* scope) {
    return this->derive_scope(static_cast<Scope>(scope));
}

Scope SymbolTable::derive_scope(Scope scope) {
    return static_cast<Scope>(this->scopes.emplace_back(this, scope, this->scopes.size()));
}

Scope SymbolTable::get_scope(Scope::Id id) {
    return Scope(this->get_scope_raw(id));
}

Scope_* SymbolTable::get_scope_raw(Scope::Id id) {
    return &this->scopes.at(id);
}

Symbol_& SymbolTable::new_symbol() {
    Symbol_& symbol = this->symbols.emplace_back();
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
 * Symbol resolution
 */

Symbol Scope_::find(const std::string& name) {
    auto s = this->symbol_map.find(name);
    if (s == this->symbol_map.end()) {
        if (!this->is_root())
            return this->parent->find(name);
        else return {};
    }
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

    Symbol_& s = this->context->new_symbol();
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

void SymbolTable::report_unresolved_symbol(UnresolvedSymbol&& symbol) {
    this->unresolved_symbols.emplace_back(symbol);
}

/// \brief Attempts to resolve an unresolved symbol.
/// \returns True if the symbol was resolved. Otherwise false.
static bool try_resolve(const UnresolvedSymbol& symbol, SymbolTable* context) {
    bool resolved;
    auto scope = context->get_scope(symbol.origin_scope);
    auto matching_symbol = scope->find(symbol.name);

    resolved = !matching_symbol.is_nullptr()
               && matching_symbol->kind == symbol.expected_kind;
    // TODO: Include check that the symbol is static.
    //       Any symbol that is non-static that was not
    //       resolved originally will per definition be
    //       inaccessible in the given context.
    //       The only reason why this is not done yet
    //       is the fact that symbol definitions are
    //       yet to be implemented.

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
                    [this](const UnresolvedSymbol& symbol) { return try_resolve(symbol, this); }),
            this->unresolved_symbols.end()
    );
}

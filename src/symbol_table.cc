#include <symbol_table.hpp>

using namespace eel;

/*
 * Scope implementation
 */

Scope::Scope(Scope_* scope) {
    this->id = scope->id;
    this->table = scope->context;
}

Scope_* Scope::operator->() { return this->table->get_scope_raw(id); }


bool Scope::is_nullptr() { return this->table == nullptr; }


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

Scope_::operator Scope() {
    return Scope(this);
}

void Scope_::declare_var(const std::string& name) {
    if (this->symbol_map.contains(name)) {
        // TODO throw exception
    }

    Symbol& symbol = this->context->new_symbol();
    symbol.kind = Symbol::Kind::Variable;
    symbol.name = name;

    this->symbol_map.insert(std::make_pair(name, symbol.id));
}

bool Scope_::is_root() {
    return this->parent.is_nullptr();
}

Symbol* Scope_::find(std::string& name) {
    auto s = this->symbol_map.find(name);
    if (s == this->symbol_map.end()) {
        if (!this->parent.is_nullptr())
            return this->parent->find(name);
        else return nullptr;
    }
    return this->context->get_symbol(s->second);
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

Symbol& SymbolTable::new_symbol() {
    Symbol& symbol = this->symbols.emplace_back();
    symbol.id = this->symbols.size() - 1;

    return symbol;
}

Symbol* SymbolTable::get_symbol(Symbol::Id id) {
    return &symbols[id];
}

#include <symbol_table.hpp>

using namespace eel;

/*
 * Scope_ implementation
 */

bool Scope_::is_root() {
    return this->parent != nullptr;
}

Scope_* Scope::operator->() { return &this->table->scopes[this->id]; }

/*
 * Symbol table implementation
 */

SymbolTable::SymbolTable() {
    // Create the root scope
    this->scopes.emplace_back(this, 0);
}


Scope_* SymbolTable::derive_scope() {
    return this->derive_scope(&this->scopes[0]);
}

Scope_* SymbolTable::derive_scope(Scope_* scope) {
    return &this->scopes.emplace_back(this, scope, this->scopes.size());
}

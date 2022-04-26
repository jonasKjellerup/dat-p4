#include <symbol_table.hpp>

using namespace eel;

/*
 * Scope implementation
 */

bool Scope::is_root() {
    return this->parent != nullptr;
}

/*
 * Symbol table implementation
 */

SymbolTable::SymbolTable() {
    // Create the root scope
    this->scopes.emplace_back(this, 0);
}

Scope* SymbolTable::derive_scope() {
    return this->derive_scope(&this->scopes[0]);
}

Scope* SymbolTable::derive_scope(Scope* scope) {
    return &this->scopes.emplace_back(this, scope, this->scopes.size());
}
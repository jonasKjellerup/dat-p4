#include <symbols/event.hpp>
#include <symbols/type.hpp>

using namespace eel::symbols;

void Event::add_handle(Scope scope, SymbolTable* table) {
    this->event_handles.push_back(
            {
                    scope,
                    table->get_symbol(symbols::Primitive::boolean.id),
                    {},
                    true
            });
}

const std::vector<Function>& Event::get_handles() const {
    return this->event_handles;
}
#include <symbols/event.hpp>
#include <symbols/type.hpp>

#include <fmt/core.h>

using namespace eel::symbols;

void Event::compute_id(Symbol symbol) {
    this->id = fmt::format("s{}_event_{}", symbol->id, symbol->name);
}

void Event::add_handle(Scope scope, SymbolTable* table) {
    this->event_handles.push_back(
            {
                    scope,
                    {},     // return type
                    {},     // parameters
                    false,  // has_return_type
                    fmt::format("{}_handle{}", this->id, this->event_handles.size())
            });
}

const std::vector<Function>& Event::get_handles() const {
    return this->event_handles;
}
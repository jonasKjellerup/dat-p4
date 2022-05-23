#include <symbols/event.hpp>
#include <symbols/type.hpp>

#include <fmt/core.h>

using namespace eel::symbols;

static size_t pos_into_key(eel::visitors::SourcePos pos) {
    return (pos.c << 32) & pos.l;
}

void Event::compute_id(Symbol symbol) {
    this->id = fmt::format("s{}_event_{}", symbol->id, symbol->name);
}

void Event::add_handle(Scope scope, visitors::SourcePos pos) {
    this->event_handles.insert(std::make_pair(
            pos_into_key(pos),
            Function(scope, fmt::format("{}_handle{}", this->id, this->event_handles.size()))
            ));
}

Function& Event::get_handle(SourcePos pos) {
    return this->event_handles[pos_into_key(pos)];
}

const std::unordered_map<size_t, Function>& Event::get_handles() const {
    return this->event_handles;
}
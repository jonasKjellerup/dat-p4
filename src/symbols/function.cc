#include <symbols/function.hpp>
#include <sequence.hpp>

using namespace eel::symbols;

bool Function_::has_return_type() const {
    return !return_type.is_nullptr();
}

const std::string& ExternalFunction::target_id() const {
    return _target_id;
}

constexpr bool ExternalFunction::is_async() const {
    return false;
}

Function::Function(Scope scope, std::string&& name)
: scope(scope), type_id(name){}

const std::string& Function::target_id() const {
    return type_id;
}

bool Function::is_async() const {
    return sequence != nullptr
        && sequence->start->is_async();
}
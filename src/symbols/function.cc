#include <symbols/function.hpp>
#include <sequence.hpp>

using namespace eel::symbols;

bool Function::is_async() const {
    return sequence != nullptr
        && sequence->start->kind == SequencePoint::AsyncPoint;
}
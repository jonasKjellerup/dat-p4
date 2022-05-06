#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#pragma ide diagnostic ignored "cert-err58-cpp"
#include <symbols/type.hpp>

using namespace eel::symbols;

Type::Type(eel::symbols::Type::Kind kind) {
    this->kind = kind;
}

Struct::Struct() : Type(Type::Kind::Struct) {}

/*
 * Primitives
 */

Primitive::Primitive(const std::string& global_name) noexcept
        : Primitive(global_name, global_name) {}

Primitive::Primitive(const std::string& source_name, const std::string& target_name) noexcept
        : Type(Type::Kind::Primitive),
          source_name(source_name),
          target_name(target_name) {}

const std::string& Primitive::type_target_name() const {
    return this->target_name;
}

const std::string& Primitive::type_source_name() const {
    return this->source_name;
}

Primitive Primitive::u8("u8");
Primitive Primitive::u16("u16");
Primitive Primitive::u32("u32");
Primitive Primitive::u64("u64");

Primitive Primitive::i8("i8");
Primitive Primitive::i16("i16");
Primitive Primitive::i32("i32");
Primitive Primitive::i64("i64");

Primitive Primitive::f32("f32");
Primitive Primitive::f64("f64");

Primitive Primitive::boolean("bool");

Primitive Primitive::digital("digital", "pin<digital>");
Primitive Primitive::analog("analog", "pin<analog>");

void Primitive::register_primitives(eel::Scope scope) {
    scope->declare_type(&u8);
    scope->declare_type(&u16);
    scope->declare_type(&u32);
    scope->declare_type(&u64);

    scope->declare_type(&i8);
    scope->declare_type(&i16);
    scope->declare_type(&i32);
    scope->declare_type(&i64);

    scope->declare_type(&f64);
    scope->declare_type(&f32);

    scope->declare_type(&digital);
    scope->declare_type(&analog);
}

#pragma clang diagnostic pop
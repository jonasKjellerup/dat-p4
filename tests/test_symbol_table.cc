#include <catch.hpp>
#include <symbol_table.hpp>
#include <symbols/type.hpp>
#include <string>

void prepare_test_table(eel::SymbolTable& table) {
    eel::symbols::Primitive::register_primitives(table.root_scope);
}

/*
 * Tests finding symbols across scope boundaries.
 * Specifically accessing variable symbols defined in an outer scope
 * from an inner scope.
 *
 * Example:
 * i32 x;
 * {
 *  // We test that the following x refers to the x defined before the block
 *  x = 2;
 * }
 */
TEST_CASE( "symbol lookup propagates into parent scope", "[symbol_table]" ) {
    using namespace eel;
    SymbolTable table;
    prepare_test_table(table);

    auto scope_a = table.derive_scope();
    auto type = table.root_scope->find("u8");

    std::string symbol_name = "symbolA";
    table.root_scope->declare_var(type, symbol_name);
    auto symbol = scope_a->find(symbol_name);

    REQUIRE_FALSE(symbol.is_nullptr());
    REQUIRE(symbol->kind == Symbol_::Kind::Variable);
}

/*
 * Tests shadowing of symbols from outer scopes.
 *
 * Example:
 * i32 x = 2;
 * {
 *  // x in the right operand should refer to the original symbol from the outer scope
 *  f32 x = x as f32;
 *  x *= 4.3; // x here should refer to the inner x
 * }
 */
TEST_CASE( "symbol shadows similar symbol from outer scope", "[symbol_table]" ) {
    eel::SymbolTable table;
    prepare_test_table(table);

    auto type = table.root_scope->find("u8");
    std::string symbol_name = "symbolA";
    table.root_scope->declare_var(type, symbol_name);

    auto outer_symbol = table.root_scope->find(symbol_name);

    auto inner_scope = table.derive_scope();
    inner_scope->declare_var(type, symbol_name);

    auto inner_symbol = inner_scope->find(symbol_name);

    REQUIRE(!outer_symbol.is_nullptr());
    REQUIRE(!inner_symbol.is_nullptr());
    REQUIRE(inner_symbol->id != outer_symbol->id);
}

/*
 * Tests handling of an undeclared expected static symbols.
 *
 * Example:
 * fn a() {
 *  b(); // The compiler is not yet aware of the existence of a symbol named b
 * }
 *
 * // The compiler then becomes aware of the symbol.
 * // The reference to `b` in the function `a` should be
 * // updated to refer to `b`
 * fn b() {}
 */
TEST_CASE( "static declaration resolves expected symbol", "[symbol_table]" ) {
    eel::SymbolTable table;
    prepare_test_table(table);

    using Kind = eel::Symbol_::Kind;

    std::string symbol_name = "symbolA";
    auto deferred_symbol = table.root_scope->defer_symbol(symbol_name, Kind::Variable);
    auto type = table.root_scope->find("u8");

    REQUIRE(!deferred_symbol.is_nullptr());
    REQUIRE(deferred_symbol->kind == Kind::Indirect);
    REQUIRE(deferred_symbol->value.indirect.kind == Kind::Variable);
    REQUIRE_FALSE(deferred_symbol->value.indirect.is_set());

    table.root_scope->declare_var(type, symbol_name, true);
    table.try_resolve_unresolved();

    REQUIRE(deferred_symbol->value.indirect.is_set());

    auto s = table.get_symbol(deferred_symbol->value.indirect.id);
    REQUIRE_FALSE(s.is_nullptr());
    REQUIRE(s->kind == Kind::Variable);
    REQUIRE(s->name == symbol_name);

}

// TODO test out of order static declaration with duplicate name
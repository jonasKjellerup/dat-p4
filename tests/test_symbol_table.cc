#include <catch.hpp>
#include <symbol_table.hpp>
#include <string>

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
    eel::SymbolTable table;
    auto scope_a = table.derive_scope();

    std::string symbol_name = "symbolA";
    table.root_scope->declare_var(symbol_name);
    auto symbol = scope_a->find(symbol_name);

    REQUIRE(symbol != nullptr);
    REQUIRE(symbol->kind == eel::Symbol::Kind::Variable);
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

    std::string symbol_name = "symbolA";
    table.root_scope->declare_var(symbol_name);

    auto outer_symbol = table.root_scope->find(symbol_name);

    auto inner_scope = table.derive_scope();
    inner_scope->declare_var(symbol_name);

    auto inner_symbol = inner_scope->find(symbol_name);

    REQUIRE(outer_symbol != nullptr);
    REQUIRE(inner_symbol != nullptr);
    REQUIRE(inner_symbol != outer_symbol);
    REQUIRE(inner_symbol->id != outer_symbol->id);
}
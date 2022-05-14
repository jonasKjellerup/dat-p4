#include <Visitors/codegen.hpp>
#include <symbols/event.hpp>
#include <symbols/variable.hpp>
#include <symbols/type.hpp>
#include <error.hpp>

#include <fmt/core.h>

using namespace eel;
using namespace eel::visitors;

const char* kind_labels[] = {
        "None", "Variable", "Constant",
        "Function", "Type", "Namespace",
        "Event", "Indirect"
};

static void generate_event(FILE* stream, Symbol event);
static void generate_function_type_body(FILE* stream, const symbols::Function& function);
static std::string generate_variable_id(Symbol symbol);
static void check_symbol(Symbol symbol, Symbol_::Kind expected_kind, const std::string& identifier);
static void resolve(Symbol& symbol, SymbolTable& table);


CodegenVisitor::CodegenVisitor(eel::SymbolTable& table, FILE* stream)
: table(table), stream(stream) {
    current_scope = table.root_scope;
}

CodegenVisitor::Any CodegenVisitor::visitIdentifier(eelParser::IdentifierContext *ctx) {
    auto identifier = ctx->Identifier()->getText();
    auto symbol = current_scope->find(identifier);

    resolve(symbol, table);
    check_symbol(symbol, Symbol_::Kind::Variable, identifier);

    return generate_variable_id(symbol);
}

CodegenVisitor::Any CodegenVisitor::visitPinDecl(eelParser::PinDeclContext* ctx) {
    auto identifier = ctx->Identifier()->getText();
    auto symbol = current_scope->find(identifier);

    resolve(symbol, table);
    check_symbol(symbol, Symbol_::Kind::Variable, identifier);

    auto variable = symbol->value.variable;
    auto type = variable->type;
    auto type_v = type->value.type;

    auto pin_id = std::any_cast<std::string>(visitChildren(ctx->expr()));

    fmt::print(stream,
               "{} {} {{ {} }};",
               type_v->type_target_name(),
               generate_variable_id(symbol),
               pin_id);


    return {};
}

CodegenVisitor::Any CodegenVisitor::visitEventDecl(eelParser::EventDeclContext* ctx) {
    auto symbol = table.root_scope->find(ctx->Identifier()->getText());
    if (symbol->kind == Symbol_::Kind::Event)
        throw InternalError(InternalError::Codegen, "Invalid symbol. Expected event.");

    auto event = symbol->value.event;
    if (!event->is_complete) // TODO this should have been reported earlier
        // TODO report error
        return {};

    // If the event is never used (no `awaits` or `on` blocks)
    // don't bother generating the event.
    if (!event->is_awaited && event->get_handles().empty())
        return {};

    auto block = ctx->stmtBlock();
    if (block == nullptr)
        event->has_predicate = false;

    generate_event(stream, symbol);

    return eelBaseVisitor::visitEventDecl(ctx);
}

void generate_event(FILE* stream, Symbol event) {
    static const std::string predicateless_type = "PredicateLess";

    // Generate function types for event handles
    auto& handles = event->value.event->get_handles();

    for (auto& handle : handles) {
        fmt::print(stream, "struct {}", handle.type_id);
        // TODO add check for async function
        generate_function_type_body(stream, handle);
    }

    auto predicate_type = predicateless_type;
    if (event->value.event->has_predicate) {
        predicate_type = fmt::format("{}_predicate", event->value.event->id);
    }

    //
    fmt::print(stream, "Event<{}", predicate_type);
    for (auto& handle : handles) {
        fmt::print(stream, ", {}", handle.type_id);
    }

    fmt::print(stream, "> {};\n", event->value.event->id);
}

void generate_function_type_body(FILE* stream, const symbols::Function& function) {
    fmt::print(stream, "{{ static void invoke () {{ {} }} }};", "INSERT INVOKE body here");
}

std::string generate_variable_id(Symbol symbol) {
    return fmt::format("__v{}", symbol->id);
}

void check_symbol(Symbol symbol, Symbol_::Kind expected_kind, const std::string& identifier) {
    if (symbol.is_nullptr())
        throw InternalError(
                InternalError::Codegen,
                fmt::format("Symbol lookup failure. `{}` not found.", identifier)
        );

    if (symbol->kind != expected_kind)
        throw InternalError(
                InternalError::Codegen,
                fmt::format("Symbol `{}` is not a {}.",
                            identifier,
                            kind_labels[static_cast<size_t>(expected_kind)])
        );
}

void resolve(Symbol& symbol, SymbolTable& table) {
    if (symbol->kind == Symbol_::Kind::Indirect) {
        symbol = table.get_symbol(symbol->value.indirect.id);
    }
}
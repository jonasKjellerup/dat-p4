#include <Visitors/codegen.hpp>
#include <symbols/event.hpp>
#include <symbols/variable.hpp>
#include <symbols/type.hpp>
#include <error.hpp>

#include <functional>
#include <fmt/core.h>

using namespace eel;
using namespace eel::visitors;
using std::any;

const char* kind_labels[] = {
        "None", "Variable", "Constant",
        "Function", "Type", "Namespace",
        "Event", "Indirect"
};

/// Generates and writes an event Symbol to the given stream.
static void generate_event(FILE* stream, Symbol event);

/// \brief Generates a functor type for a synchronous function.
/// \param stream The output stream where the functor is written.
/// \param function The function for which the functor is being generated
/// \param invoke_generator A function that is responsible for calling
///                         the necessary functions for generating the invoke body.
static void generate_sync_functor_type(
        FILE* stream,
        const symbols::Function& function,
        const std::function<void()>& invoke_generator
);

/// Generates the body of a function type based of a Functions definition
/// and writes it to the stream.
static void generate_function_type_body(FILE* stream, const symbols::Function& function);

/// Generate a variable identifier, for use in the generated c++ code.
static std::string generate_variable_id(Symbol symbol);

/// Check if a Symbol is non-null and is of correct kind.
/// \throws InternalError If null or kind != expected_kind
static void check_symbol(Symbol symbol, Symbol_::Kind expected_kind, const std::string& identifier);

/// Substitutes the symbol with the indirect symbol if of indirect kind.
static void resolve(Symbol& symbol, SymbolTable& table);


CodegenVisitor::CodegenVisitor(eel::SymbolTable& table, FILE* stream)
        : table(table), stream(stream) {
    current_scope = table.root_scope;
}

any CodegenVisitor::visitProgram(eelParser::ProgramContext* ctx) {
    fmt::print(stream, "#include <runtime/all.hpp>\n");
    visitChildren(ctx);

    // TODO
    fmt::print(stream, "int main(void) {{ return 0; }}\n");
    return {};
}

/*
 * Expressions - All expression visitors should produce
 *             a std::string object.
 */

/*
 * Literal expressions
 */
any CodegenVisitor::visitBoolLiteral(eelParser::BoolLiteralContext* ctx) {
    // our boolean literals are a 1:1 match with c++ literals
    return ctx->BoolLiteral()->getText();
}

any CodegenVisitor::visitCharLiteral(eelParser::CharLiteralContext* ctx) {
    return ctx->getText(); // TODO our character literals are not a 1:1 with c++
}

any CodegenVisitor::visitFloatLiteral(eelParser::FloatLiteralContext* ctx) {
    return eelBaseVisitor::visitFloatLiteral(ctx);// TODO
}

any CodegenVisitor::visitIntegerLiteral(eelParser::IntegerLiteralContext* ctx) {
    return {}; // TODO
}

any CodegenVisitor::visitStringLiteral(eelParser::StringLiteralContext* ctx) {
    return {}; // TODO
}

/*
 * Access expressions
 */

any CodegenVisitor::visitIdentifier(eelParser::IdentifierContext* ctx) {
    auto identifier = ctx->Identifier()->getText();
    auto symbol = current_scope->find(identifier);

    resolve(symbol, table);
    check_symbol(symbol, Symbol_::Kind::Variable, identifier);

    return generate_variable_id(symbol);
}

/*
 * Arithmetic operators
 */

any CodegenVisitor::visitPos(eelParser::PosContext* ctx) {
    return visit(ctx->expr());
}

any CodegenVisitor::visitNeg(eelParser::NegContext* ctx) {
    return fmt::format("-({})", std::any_cast<std::string>(visit(ctx->expr())));
}

any CodegenVisitor::visitScalingExpr(eelParser::ScalingExprContext* ctx) {
    return fmt::format("({}){}({})",
                       std::any_cast<std::string>(visit(ctx->left)),
                       ctx->op->getText(),
                       std::any_cast<std::string>(visit(ctx->right)));
}

any CodegenVisitor::visitAdditiveExpr(eelParser::AdditiveExprContext* ctx) {
    return fmt::format("({}){}({})",
                       std::any_cast<std::string>(visit(ctx->left)),
                       ctx->op->getText(),
                       std::any_cast<std::string>(visit(ctx->right)));
}

/*
 * Logical/comparative operators
 */

any CodegenVisitor::visitComparisonExpr(eelParser::ComparisonExprContext* ctx) {
    return fmt::format("({}){}({})",
                       std::any_cast<std::string>(visit(ctx->left)),
                       ctx->op->getText(),
                       std::any_cast<std::string>(visit(ctx->right)));
}

any CodegenVisitor::visitNot(eelParser::NotContext* ctx) {
    return fmt::format("!({})", std::any_cast<std::string>(visit(ctx->expr())));
}

/*
 * Declarations
 */

any CodegenVisitor::visitVariableDecl(eelParser::VariableDeclContext* ctx) {
    auto identifier = ctx->typedIdentifier()->Identifier()->getText();
    auto symbol = current_scope->find(identifier);

    check_symbol(symbol, Symbol_::Kind::Variable, identifier);
    auto variable = symbol->value.variable;
    auto type = variable->type;

    if (variable->has_value) {
        fmt::print(stream, "{} {} = {};",
                   type->value.type->type_target_name(),
                   generate_variable_id(symbol),
                   std::any_cast<std::string>(visit(ctx->expr())));
    } else {
        fmt::print(stream, "{} {};",
                   type->value.type->type_target_name(),
                   generate_variable_id(symbol));
    }

    return {};
}

any CodegenVisitor::visitSetupDecl(eelParser::SetupDeclContext* ctx) {
    // TODO handle async
    auto symbol = current_scope->find("__eel_setup");

    generate_sync_functor_type(stream, *symbol->value.function, [this, ctx]() {
        this->visit(ctx->stmtBlock());
    });

    return {};
}

any CodegenVisitor::visitLoopDecl(eelParser::LoopDeclContext* ctx) {
    // TODO handle async
    auto symbol = current_scope->find("__eel_loop");

    generate_sync_functor_type(stream, *symbol->value.function, [this, ctx]() {
        this->visit(ctx->stmtBlock());
    });

    return {};
}

any CodegenVisitor::visitPinDecl(eelParser::PinDeclContext* ctx) {
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

any CodegenVisitor::visitEventDecl(eelParser::EventDeclContext* ctx) {
    auto symbol = table.root_scope->find(ctx->Identifier()->getText());
    if (symbol->kind == Symbol_::Kind::Event)
        throw InternalError(InternalError::Codegen, "Invalid symbol. Expected event.");

    auto event = symbol->value.event;
    if (!event->is_complete)
        throw InternalError(InternalError::Codegen, "Incomplete event encountered during codegen.");

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

    for (auto& handle: handles) {
        fmt::print(stream, "struct {}", handle.second.type_id);
        // TODO add check for async function
        generate_function_type_body(stream, handle.second);
    }

    auto predicate_type = predicateless_type;
    if (event->value.event->has_predicate) {
        predicate_type = fmt::format("{}_predicate", event->value.event->id);
    }

    //
    fmt::print(stream, "Event<{}", predicate_type);
    for (auto& handle: handles) {
        fmt::print(stream, ", {}", handle.second.type_id);
    }

    fmt::print(stream, "> {};\n", event->value.event->id);
}

void generate_sync_functor_type(
        FILE* stream,
        const symbols::Function& function,
        const std::function<void()>& invoke_generator
) {
    fmt::print(stream,
               "struct {} {{ static void invoke() {{",
               function.type_id);
    invoke_generator();
    fmt::print(stream, " }} }};");
}

void generate_async_functor_type(
        FILE* stream,
        const symbols::Function& function,
        const std::function<void()>& invoke_generator
) {
    fmt::print(stream,
        "struct {} : AsyncFunction {{"
        "struct State {{",
        function.type_id);

    fmt::print(stream, "}};" // End of State struct decl
                       "static int begin_invoke(State& state"); // Start of begin_invoke param list

    for (auto param: function.parameters) {
        auto var = param->value.variable;
        fmt::print(stream, ", {} {}",
                   var->type->value.type->type_target_name(),
                   generate_variable_id(param));
    }

    fmt::print(stream, ") {{"); // End of begin_invoke param list + start of body

    // TODO generate being_invoke body

    fmt::print(stream, "}}\n static int step(State& state) {{ ");

    // TODO generate step body

    fmt::print(stream, "}} }};"); // End of step body and functor struct

};

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
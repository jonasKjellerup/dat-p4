#include <Visitors/codegen.hpp>
#include <symbols/event.hpp>
#include <symbols/variable.hpp>
#include <symbols/type.hpp>
#include <error.hpp>
#include <sequence.hpp>

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

/// \brief Generates a functor type for a synchronous function.
/// \param stream The output stream where the functor is written.
/// \param function The function for which the functor is being generated
/// \param invoke_generator A function that is responsible for calling
///                         the necessary functions for generating the invoke body.
static void generate_sync_functor_type(
        FILE* stream,
        const symbols::Function& function,
        CodegenVisitor& visitor
);

/// \brief Generates a functor type for a asynchronous function.
/// \param stream The output stream where the functor is written.
/// \param function The function for which the functor is being generated
/// \param step_generator A function that is responsible for calling
///                         the necessary functions for generating the step body.
static void generate_async_functor_type(
        FILE* stream,
        const symbols::Function& function,
        CodegenVisitor& visitor
);

static void close_open_async_case(CodegenVisitor& visitor);

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
    // TODO the syntax should be mostly compatible, but this should be double checked
    return ctx->FloatLiteral()->getText();
}

any CodegenVisitor::visitIntegerLiteral(eelParser::IntegerLiteralContext* ctx) {
    // TODO the syntax should be mostly compatible, but this should be double checked
    return ctx->IntegerLiteral()->getText();
}

any CodegenVisitor::visitStringLiteral(eelParser::StringLiteralContext*) {
    return {}; // TODO
}

/*
 * Access expressions
 */

// TODO make this visitFqn at some point if we wanna implement member/namespace access
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

    if (current_sequence == nullptr || current_sequence->current_point->kind == SequencePoint::SyncPoint) {
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
    } else if (variable->has_value) {
        fmt::print(stream, "state.{} = {};",
                   generate_variable_id(symbol),
                   std::any_cast<std::string>(visit(ctx->expr())));
    }

    return {};
}

any CodegenVisitor::visitSetupDecl(eelParser::SetupDeclContext*) {
    auto symbol = current_scope->find("__eel_setup");
    auto func = symbol->value.function;

    if (func->sequence->start->kind == SequencePoint::AsyncPoint) {
        generate_async_functor_type(stream, *func, *this);
    } else {
        generate_sync_functor_type(stream, *func, *this);
    }

    return {};
}

any CodegenVisitor::visitLoopDecl(eelParser::LoopDeclContext*) {
    auto symbol = current_scope->find("__eel_loop");
    auto func = symbol->value.function;

    if (func->sequence->start->kind == SequencePoint::AsyncPoint) {
        generate_async_functor_type(stream, *func, *this);
    } else {
        generate_sync_functor_type(stream, *func, *this);
    }

    return {};
}

any CodegenVisitor::visitPinDecl(eelParser::PinDeclContext* ctx) {
    // TODO figure out if we need to translate analog pin ids
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
        event->has_predicate = false; // TODO ensure that this has not already been done (no point in doing it twice)

    static const std::string predicateless_type = "PredicateLess";

    // Generate function types for event handles
    auto& handles = event->get_handles();
    for (auto& handle: handles) {
        if (handle.second.sequence->start->kind == SequencePoint::AsyncPoint) {
            generate_async_functor_type(stream, handle.second, *this);
        } else {
            generate_sync_functor_type(stream, handle.second, *this);
        }
    }

    auto predicate_type = predicateless_type;
    if (event->has_predicate) {
        predicate_type = event->predicate->type_id;
        if (event->predicate->sequence->start->kind == SequencePoint::AsyncPoint) {
            generate_async_functor_type(stream, *event->predicate, *this);
        } else {
            generate_sync_functor_type(stream, *event->predicate, *this);
        }
    }

    // Generate the event field
    fmt::print(stream, "Event<{}", predicate_type);
    for (auto& handle: handles) {
        fmt::print(stream, ", {}", handle.second.type_id);
    }

    fmt::print(stream, "> {} {{}};\n", event->id);

    return {};
}

any CodegenVisitor::visitOnDecl(eelParser::OnDeclContext*) {
    // the event handler code is generated by the event declaration visitor
    return {};
}

/*
 * Statements
 */

any CodegenVisitor::visitStmtBlock(eelParser::StmtBlockContext* ctx) {
    auto sequence_point = dynamic_cast<Block*>(current_sequence->next());

    if (sequence_point == nullptr)
        throw InternalError(InternalError::Codegen, "Out of sync sequence point. Block object expected.");

    if (sequence_point->kind == SequencePoint::AsyncPoint) {
        close_open_async_case(*this);
        fmt::print(stream, "case {}:", async_state_counter++);
        is_in_async_state_case = true;
    }

    fmt::print(stream, "{{");
    visitChildren(ctx); // TODO this will cause issues with nested cases

    if (sequence_point->kind == SequencePoint::AsyncPoint) {
        close_open_async_case(*this);
    } else {
        fmt::print("}}");
    }

    return {};
}

any CodegenVisitor::visitAwaitStmt(eelParser::AwaitStmtContext* ctx) {
    auto sequence_point = current_sequence->next();

    if (sequence_point == nullptr || sequence_point->kind == SequencePoint::YieldPoint)
        throw InternalError(InternalError::Codegen, "Out of sync sequence point. YieldPoint expected.");

    auto fqn = dynamic_cast<eelParser::FqnExprContext*>(ctx->expr()->children[0]);
    std::string predicate;
    if (fqn == nullptr) {
        predicate = std::any_cast<std::string>(visitChildren(ctx));
    } else {
        auto symbol = current_scope->find(fqn->getText());
        if (symbol->kind == Symbol_::Kind::Event) {
            auto event = symbol->value.event;
            predicate = fmt::format("{}.has_emit_flag()", event->id);
        } else if (symbol->kind == Symbol_::Kind::Variable) {
            predicate = std::any_cast<std::string>(visit(fqn));
        } else {
            // Report error: "Cannot await non-event/bool-expr"
            return {};
        }
    }

    close_open_async_case(*this);

    fmt::print(stream, "case {}: {{"
                       "if ({}) state.s += 1;return 0;}}",
                       async_state_counter++,
                       predicate);

    return {};
}

/*
 * Helper functions
 */

static void generate_functor_core(const symbols::Function& function,
                                   CodegenVisitor& visitor) {
    auto outer_scope = visitor.current_scope;
    visitor.current_sequence = function.sequence;
    visitor.current_scope = visitor.current_sequence->start->scope;

    visitor.visitChildren(function.body);

    visitor.current_scope = outer_scope; // restore previous scope
    visitor.current_sequence = nullptr;
}

void generate_sync_functor_type(
        FILE* stream,
        const symbols::Function& function,
        CodegenVisitor& visitor
) {
    fmt::print(stream,
               "struct {} {{ static void invoke() {{",
               function.type_id);
    generate_functor_core(function, visitor);
    fmt::print(stream, " }} }};");
}

void generate_async_functor_type(
        FILE* stream,
        const symbols::Function& function,
        CodegenVisitor& visitor
) {
    visitor.async_state_counter = 0;
    visitor.is_in_async_state_case = false;

    fmt::print(stream,
        "struct {} : AsyncFunction {{"
        "struct State {{"
        "u8 s;",
        function.type_id);

    if (function.has_return_type) {
        auto return_type = function.return_type->value.type;
        fmt::print(stream, "{} r;", return_type->type_target_name());
    }

    // TODO generate state fields

    fmt::print(stream, "}};" // End of State struct decl
                       "static int step(State&);" // Forward declare step
                       "static int begin_invoke(State& state"); // Start of begin_invoke param list

    for (auto param: function.parameters) {
        auto var = param->value.variable;
        fmt::print(stream, ", {}& {}",
                   var->type->value.type->type_target_name(),
                   generate_variable_id(param));
    }

    fmt::print(stream, ") {{ state.s = 0;"); // End of begin_invoke param list + start of body

    for (auto param: function.parameters) {
        auto name = generate_variable_id(param);
        fmt::print(stream, "state.{} = {};", name, name);
    }

    fmt::print(stream, "return step(state);}}\n"
                       "static int step(State& state) {{"
                       "switch (state.s) {{");

    generate_functor_core(function, visitor);

    fmt::print(stream, "}} }} }};"); // End of step body and functor struct

};

void close_open_async_case(CodegenVisitor& visitor) {
    if (visitor.is_in_async_state_case) {
        fmt::print(visitor.stream, "state.s += 1; return 0; }}");
        visitor.is_in_async_state_case = false;
    }
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
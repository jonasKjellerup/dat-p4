#include <Visitors/codegen.hpp>
#include <symbols/event.hpp>
#include <symbols/variable.hpp>
#include <symbols/type.hpp>
#include <error.hpp>
#include <sequence.hpp>

#include <unordered_set>
#include <fmt/core.h>
#include <fmt/ostream.h>

using namespace eel;
using namespace eel::visitors;
using std::any;

const char *kind_labels[] = {
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
        std::iostream *stream,
        const symbols::Function &function,
        CodegenVisitor &visitor
);

/// \brief Generates a functor type for a asynchronous function.
/// \param stream The output stream where the functor is written.
/// \param function The function for which the functor is being generated
/// \param step_generator A function that is responsible for calling
///                         the necessary functions for generating the step body.
static void generate_async_functor_type(
        std::iostream *stream,
        const symbols::Function &function,
        CodegenVisitor &visitor
);

static void close_open_async_case(CodegenVisitor &visitor);

/// Generate a variable identifier, for use in the generated c++ code.
static std::string generate_variable_id(Symbol symbol);

/// Check if a Symbol is non-null and is of correct kind.
/// \throws InternalError If null or kind != expected_kind
static void check_symbol(Symbol symbol, Symbol_::Kind expected_kind, const std::string &identifier);

/// Substitutes the symbol with the indirect symbol if of indirect kind.
static void resolve(Symbol &symbol, SymbolTable &table);


CodegenVisitor::CodegenVisitor(eel::SymbolTable &table, std::iostream *stream)
        : table(table), stream(stream) {
    current_scope = table.root_scope;

    auto size_type = table.get_symbol(symbols::Primitive::usize.id);
    auto u32_type = table.get_symbol(symbols::Primitive::u32.id);

    // Parameter types are currently omitted for print function due
    // to not having a way of handling overloaded functions.
    // This works exclusively because type checking has not been
    // implemented.
    current_scope->declare_fn_cpp("serial_print", "Serial.print", size_type);
    current_scope->declare_fn_cpp("serial_println", "Serial.println", size_type);
    current_scope->declare_fn_cpp("serial_begin", "Serial.begin", size_type, u32_type);
}

any CodegenVisitor::visitProgram(eelParser::ProgramContext *ctx) {
    static const char *setup_state_id = "__setup_state";
    static const char *loop_state_id = "__loop_state";
    fmt::print(*stream, "#include <runtime/all.hpp>\n");
    visitChildren(ctx);

    fmt::print(*stream, "\nint main(void) {{\n");

    // This can break if the user has defined another symbol
    // using the predetermined names. We just assume that if
    // it exists then it is a function.
    auto setup = current_scope->find(builtin_setup_name);
    if (!setup.is_nullptr()) {
        auto f = setup->value.function;
        if (f->is_async()) {
            fmt::print(*stream, "{setup_type}::State {setup_state} {{}};\n"
                                "while (!{setup_type}::step({setup_state})) {{\n",
                       fmt::arg("setup_type", f->type_id),
                       fmt::arg("setup_state", setup_state_id));

            for (auto event: events) {
                fmt::print(*stream, "run_handles<decltype({event_id}})>({event_id}});\n",
                           fmt::arg("event_id", event->id));
            }

            fmt::print(*stream, "}}\n");
        } else {
            fmt::print(*stream, "{}::invoke();\n", f->type_id);
        }
    }


    auto loop = current_scope->find(builtin_loop_name);
    if (!loop.is_nullptr()) {
        auto f = loop->value.function;
        if (f->is_async()) {
            fmt::print(*stream, "{}::State {} {{}};\n",
                       f->type_id, loop_state_id);
        }
    }

    fmt::print(*stream, "while (true) {{\n");

    for (auto event: events) {
        fmt::print(*stream, "run_handles<decltype({event_id})>({event_id});\n",
                   fmt::arg("event_id", event->id));
    }

    if (!loop.is_nullptr()) {
        auto f = loop->value.function;
        if (f->is_async()) {
            fmt::print(*stream, "if ({loop_type}::step({loop_state})) {loop_state}.s = 0;\n",
                       fmt::arg("loop_type", f->type_id),
                       fmt::arg("loop_state", loop_state_id));
        } else {
            fmt::print(*stream, "{}::invoke();\n", f->type_id);
        }
    }

    fmt::print(*stream, "}}\n");

    fmt::print(*stream, "return 0; }}\n");
    return {};
}

/*
 * Expressions - All expression visitors should produce
 *             a std::string object.
 */

/*
 * Literal expressions
 */
any CodegenVisitor::visitBoolLiteral(eelParser::BoolLiteralContext *ctx) {
    // our boolean literals are a 1:1 match with c++ literals
    return ctx->BoolLiteral()->getText();
}

any CodegenVisitor::visitCharLiteral(eelParser::CharLiteralContext *ctx) {
    return ctx->getText(); // TODO our character literals are not a 1:1 with c++
}

any CodegenVisitor::visitFloatLiteral(eelParser::FloatLiteralContext *ctx) {
    // TODO the syntax should be mostly compatible, but this should be double checked
    return ctx->FloatLiteral()->getText();
}

any CodegenVisitor::visitIntegerLiteral(eelParser::IntegerLiteralContext *ctx) {
    // TODO the syntax should be mostly compatible, but this should be double checked
    return ctx->IntegerLiteral()->getText();
}

any CodegenVisitor::visitStringLiteral(eelParser::StringLiteralContext *ctx) {
    return ctx->getText(); // TODO our strings/characters are not a 1:1 with c++
}

/*
 * Access/assign expressions
 */

// TODO make this visitFqn at some point when we implement member/namespace access
any CodegenVisitor::visitIdentifier(eelParser::IdentifierContext *ctx) {
    auto identifier = ctx->Identifier()->getText();
    auto is_in_async_state = false;
    Symbol symbol;

    if (current_sequence->start->is_async()) {
        auto state_root = current_sequence->start->scope;
        auto current = current_scope;

        while (current != state_root) {
            symbol = current->find_member(identifier);
            if (!symbol.is_nullptr()) {
                is_in_async_state = true;
                break;
            }

            current = current->parent;
        }
    }

    if (symbol.is_nullptr()) {
        symbol = current_scope->find(identifier);
    }

    resolve(symbol, table);
    check_symbol(symbol, Symbol_::Kind::Variable, identifier);

    auto id = generate_variable_id(symbol);

    return is_in_async_state ? fmt::format("state.{}", id) : id;
}

any CodegenVisitor::visitFnCallExpr(eelParser::FnCallExprContext *ctx) {
    auto symbol = resolve_fqn(current_scope, ctx->fqn());

    if (symbol->kind == Symbol_::Kind::Function)
        throw InternalError(InternalError::Codegen, "Call to non external functions in not currently supported.");
    else if (symbol->kind != Symbol_::Kind::ExternFunction)
        throw InternalError(InternalError::Codegen, "Call to non function symbol encountered.");

    auto fn = symbol->value.extern_function;

    fmt::print(*stream, "{}(", fn->target_id());

    if (ctx->params)
        visit(ctx->params);

    fmt::print(*stream, ");");
    return {};
}

any CodegenVisitor::visitAssignExpr(eelParser::AssignExprContext *ctx) {
    auto lop = std::any_cast<std::string>(visit(ctx->var));
    auto rop = std::any_cast<std::string>(visit(ctx->right));

    return fmt::format("{} = {}", lop, rop);
}

any CodegenVisitor::visitReadPinExpr(eelParser::ReadPinExprContext *ctx) {
    auto identifier = std::any_cast<std::string>(ctx->fqn());
    return fmt::format("{}.read()", identifier);
}

/*
 * Arithmetic operators
 */

any CodegenVisitor::visitPos(eelParser::PosContext *ctx) {
    return visit(ctx->expr());
}

any CodegenVisitor::visitNeg(eelParser::NegContext *ctx) {
    return fmt::format("-({})", std::any_cast<std::string>(visit(ctx->expr())));
}

any CodegenVisitor::visitScalingExpr(eelParser::ScalingExprContext *ctx) {
    return fmt::format("({}){}({})",
                       std::any_cast<std::string>(visit(ctx->left)),
                       ctx->op->getText(),
                       std::any_cast<std::string>(visit(ctx->right)));
}

any CodegenVisitor::visitAdditiveExpr(eelParser::AdditiveExprContext *ctx) {
    return fmt::format("({}){}({})",
                       std::any_cast<std::string>(visit(ctx->left)),
                       ctx->op->getText(),
                       std::any_cast<std::string>(visit(ctx->right)));
}

/*
 * Logical/comparative operators
 */

any CodegenVisitor::visitComparisonExpr(eelParser::ComparisonExprContext *ctx) {
    return fmt::format("({}){}({})",
                       std::any_cast<std::string>(visit(ctx->left)),
                       ctx->op->getText(),
                       std::any_cast<std::string>(visit(ctx->right)));
}

any CodegenVisitor::visitLAndExpr(eelParser::LAndExprContext *ctx) {
    return fmt::format("({})&&({})",
                       std::any_cast<std::string>(visit(ctx->left)),
                       std::any_cast<std::string>(visit(ctx->right)));
}

any CodegenVisitor::visitLOrExpr(eelParser::LOrExprContext *ctx) {
    return fmt::format("({})||({})",
                       std::any_cast<std::string>(visit(ctx->left)),
                       std::any_cast<std::string>(visit(ctx->right)));
}

any CodegenVisitor::visitNot(eelParser::NotContext *ctx) {
    return fmt::format("!({})", std::any_cast<std::string>(visit(ctx->expr())));
}

/*
 * Bitwise operators
 */

any CodegenVisitor::visitAndExpr(eelParser::AndExprContext *ctx) {
    return fmt::format("({})&({})",
                       std::any_cast<std::string>(visit(ctx->left)),
                       std::any_cast<std::string>(visit(ctx->right)));
}

any CodegenVisitor::visitOrExpr(eelParser::OrExprContext *ctx) {
    return fmt::format("({})|({})",
                       std::any_cast<std::string>(visit(ctx->left)),
                       std::any_cast<std::string>(visit(ctx->right)));
}

any CodegenVisitor::visitXorExpr(eelParser::XorExprContext *ctx) {
    return fmt::format("({})^({})",
                       std::any_cast<std::string>(visit(ctx->left)),
                       std::any_cast<std::string>(visit(ctx->right)));
}

any CodegenVisitor::visitBitComp(eelParser::BitCompContext *ctx) {
    return fmt::format("~({})", std::any_cast<std::string>(visit(ctx->expr())));
}

/*
 * Other expressions
 */

any CodegenVisitor::visitCastExpr(eelParser::CastExprContext *ctx) {
    auto type_symbol = current_scope->find(ctx->type()->getText());

    return fmt::format("static_cast<{}>({})",
                       type_symbol->value.type->type_target_name(),
                       std::any_cast<std::string>(visit(ctx->expr())));
}

any CodegenVisitor::visitExprList(eelParser::ExprListContext *ctx) {
    fmt::print(*stream, std::any_cast<std::string>(visit(ctx->expr())));
    if (ctx->exprList()) {
        fmt::print(*stream, ",");
        visit(ctx->exprList());
    }

    return {};
}

/*
 * Declarations
 */

any CodegenVisitor::visitVariableDecl(eelParser::VariableDeclContext *ctx) {
    auto identifier = ctx->typedIdentifier()->Identifier()->getText();
    auto symbol = current_scope->find(identifier);

    check_symbol(symbol, Symbol_::Kind::Variable, identifier);
    auto variable = symbol->value.variable;
    auto type = variable->type;

    if (current_sequence == nullptr || current_sequence->current_point->kind == SequencePoint::SyncPoint) {
        if (variable->has_value) {
            fmt::print(*stream, "{} {} = {};",
                       type->value.type->type_target_name(),
                       generate_variable_id(symbol),
                       std::any_cast<std::string>(visit(ctx->expr())));
        } else {
            fmt::print(*stream, "{} {};",
                       type->value.type->type_target_name(),
                       generate_variable_id(symbol));
        }
    } else if (variable->has_value) {
        fmt::print(*stream, "state.{} = {};",
                   generate_variable_id(symbol),
                   std::any_cast<std::string>(visit(ctx->expr())));
    }

    return {};
}

any CodegenVisitor::visitSetupDecl(eelParser::SetupDeclContext *) {
    auto symbol = current_scope->find("__eel_setup");
    auto func = symbol->value.function;

    if (func->sequence->start->kind == SequencePoint::AsyncPoint) {
        generate_async_functor_type(stream, *func, *this);
    } else {
        generate_sync_functor_type(stream, *func, *this);
    }

    return {};
}

any CodegenVisitor::visitLoopDecl(eelParser::LoopDeclContext *) {
    auto symbol = current_scope->find("__eel_loop");
    auto func = symbol->value.function;

    if (func->sequence->start->kind == SequencePoint::AsyncPoint) {
        generate_async_functor_type(stream, *func, *this);
    } else {
        generate_sync_functor_type(stream, *func, *this);
    }

    return {};
}

any CodegenVisitor::visitPinDecl(eelParser::PinDeclContext *ctx) {
    // TODO figure out if we need to translate analog pin ids
    auto identifier = ctx->Identifier()->getText();
    auto symbol = current_scope->find(identifier);

    resolve(symbol, table);
    check_symbol(symbol, Symbol_::Kind::Variable, identifier);

    auto variable = symbol->value.variable;
    auto type = variable->type;
    auto type_v = type->value.type;

    auto pin_id = std::any_cast<std::string>(visitChildren(ctx->expr()));

    fmt::print(*stream,
               "{} {} {{ {} }};",
               type_v->type_target_name(),
               generate_variable_id(symbol),
               pin_id);


    return {};
}

any CodegenVisitor::visitEventDecl(eelParser::EventDeclContext *ctx) {
    auto symbol = table.root_scope->find(ctx->Identifier()->getText());
    if (symbol->kind != Symbol_::Kind::Event)
        throw InternalError(InternalError::Codegen, "Invalid symbol. Expected event.");

    auto event = symbol->value.event;
    if (!event->is_complete)
        throw InternalError(InternalError::Codegen, "Incomplete event encountered during codegen.");

    events.push_back(event);

    // If the event is never used (no `awaits` or `on` blocks)
    // don't bother generating the event.
    // TODO re-enable this check once is_awaited is updated properly
    // if (!event->is_awaited && event->get_handles().empty())
    //    return {};

    auto block = ctx->stmtBlock();
    if (block == nullptr)
        event->has_predicate = false; // TODO ensure that this has not already been done (no point in doing it twice)

    static const std::string predicateless_type = "PredicateLess";

    // Generate function types for event handles
    auto &handles = event->get_handles();
    for (auto &handle: handles) {
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
    fmt::print(*stream, "Event<{}", predicate_type);
    for (auto &handle: handles) {
        fmt::print(*stream, ", {}", handle.second.type_id);
    }

    fmt::print(*stream, "> {} {{}};\n", event->id);

    return {};
}

any CodegenVisitor::visitOnDecl(eelParser::OnDeclContext *) {
    // the event handler code is generated by the event declaration visitor
    return {};
}

/*
 * Statements
 */

any CodegenVisitor::visitStmt(eelParser::StmtContext *ctx) {
    if (current_sequence->current_block->is_async() && !current_sequence->is_next_yield() && !is_in_async_state_case) {
        fmt::print(*stream, "case {}: {{", async_state_counter++);
        is_in_async_state_case = true;
    }

    if (ctx->expr() != nullptr) {
        auto expr_result = std::any_cast<std::string>(visit(ctx->expr()));
        fmt::print(*stream, "{};", expr_result);
    } else {
        visitChildren(ctx);
    }

    return {};
}

any CodegenVisitor::visitStmtBlock(eelParser::StmtBlockContext *ctx) {
    auto sequence_point = dynamic_cast<Block *>(current_sequence->next());

    if (sequence_point == nullptr)
        throw InternalError(InternalError::Codegen, "Out of sync sequence point. Block object expected.");

    if (sequence_point->kind == SequencePoint::AsyncPoint) {
        close_open_async_case(*this);
        fmt::print(*stream, "case {}:", async_state_counter++);
        is_in_async_state_case = true;
    }

    fmt::print(*stream, "{{");
    visitChildren(ctx);

    if (sequence_point->kind == SequencePoint::AsyncPoint) {
        close_open_async_case(*this);
    } else {
        fmt::print("}}");
    }

    return {};
}

any CodegenVisitor::visitAwaitStmt(eelParser::AwaitStmtContext *ctx) {
    auto sequence_point = current_sequence->next();

    if (sequence_point == nullptr || sequence_point->kind != SequencePoint::YieldPoint)
        throw InternalError(InternalError::Codegen, "Out of sync sequence point. YieldPoint expected.");

    auto fqn = dynamic_cast<eelParser::FqnExprContext *>(ctx->expr()->children[0]);
    std::string predicate;
    if (fqn == nullptr || ctx->expr()->children.size() > 1) {
        predicate = std::any_cast<std::string>(visit(ctx->expr()));
    } else {
        auto symbol = current_scope->find(fqn->getText());
        if (symbol->kind == Symbol_::Kind::Event) {
            auto event = symbol->value.event;
            predicate = fmt::format("{}.has_emit_flag()", event->id);
        } else if (symbol->kind == Symbol_::Kind::Variable) {
            predicate = std::any_cast<std::string>(visit(fqn));
        } else {
            // This should have been checked by the type checker? TODO check up on this
            // Throw error for edge case where type checker does not catch this.
            throw InternalError(InternalError::Codegen, "Cannot await non-event/bool expr.");
        }
    }

    close_open_async_case(*this);

    fmt::print(*stream, "case {}: {{"
                        "if ({}) state.s += 1;return 0;}}",
               async_state_counter++,
               predicate);

    return {};
}

any CodegenVisitor::visitReturnStmt(eelParser::ReturnStmtContext *ctx) {
    auto is_async_return = current_sequence->start->kind == SequencePoint::AsyncPoint;
    if (ctx->expr() != nullptr) {
        auto value = std::any_cast<std::string>(visit(ctx->expr()));
        if (is_async_return) {
            fmt::print(*stream, "state.r = {};return 1;", value);
        } else {
            fmt::print(*stream, "return {};", value);
        }
    } else if (is_async_return) {
        fmt::print(*stream, "return 1;");
    } else {
        fmt::print(*stream, "return;");
    }

    return {};
}

any CodegenVisitor::visitIfStmt(eelParser::IfStmtContext *ctx) {
    auto const stmt = ctx->stmt();
    auto const else_stmt = ctx->elseStmt() != nullptr ? ctx->elseStmt()->stmt() : nullptr;
    auto const if_cond = std::any_cast<std::string>(visit(ctx->conditionBlock()->expr()));

    auto const sequence_snapshot = current_sequence->snapshot();
    bool if_is_async = false,
            else_is_async;

    current_sequence->next();

    if (stmt->awaitStmt() != nullptr || stmt->stmtBlock() != nullptr) {
        if_is_async = current_sequence->current_point->is_async();
        // Set current point as adjacent point.
        // Assuming the presence of a sequence point for a block or yield in
        // an else statement it would be adjacent to the current point
        current_sequence->current_point = current_sequence->current_point->next;
    }

    else_is_async =
            // If the else branch is present
            else_stmt != nullptr
            // and if it is an await statement or a stmt block
            && (else_stmt->awaitStmt() != nullptr || else_stmt->stmtBlock() != nullptr)
            // and the current sequence point is marked async
            && current_sequence->current_point->is_async();

    current_sequence->restore(sequence_snapshot);

    if (!is_in_async_state_case) {
        fmt::print(*stream, "case {}: {{", async_state_counter++);
        is_in_async_state_case = true;
    }

    fmt::print(*stream, "if ({})", if_cond);
    // If all branches are non-async
    if (!(if_is_async | else_is_async)) {
        visit(stmt);

        if (else_stmt != nullptr) {
            fmt::print(*stream, "else ");
            visit(else_stmt);
        }
    } else {
        auto original_stream = stream;
        is_in_async_state_case = false;

        // Create output buffer
        std::stringstream if_buffer;

        if (if_is_async) {
            fmt::print(*stream, "{{ state.s = {}; return 0; }}", async_state_counter);
            stream = &if_buffer;
        }

        visit(stmt);
        stream = original_stream;

        auto if_case_is_closed = !is_in_async_state_case;
        auto if_redirection_case = if_case_is_closed ? async_state_counter++ : 0;
        is_in_async_state_case = false;

        std::stringstream else_buffer;
        if (else_stmt != nullptr) {
            fmt::print(*stream, "else ");
            if (else_is_async) {
                fmt::print(*stream, "{{ state.s = {}; return 0; }}", async_state_counter);
                stream = &else_buffer;
            }

            visit(else_stmt);
            stream = original_stream;
        }

        // Proceed to the case after the if/else statement
        // This is dead-code if both branches are async,
        // but we let the c++ compiler deal with that
        fmt::print(*stream, "state.s = {}; return 0; }}", async_state_counter);

        fmt::print(*stream, if_buffer.str());

        if (if_is_async) {
            // If the if-case is closed then create a new
            // case for redirecting
            if (if_case_is_closed) {
                fmt::print(*stream, "case {}: {{", if_redirection_case);
            }

            // Code for going from end of if to the case proceeding the last case
            // of the if else chain
            fmt::print(*stream, "state.s = {}; return 0;}}", async_state_counter);
        }

        // write else code and close case if open
        if (else_stmt != nullptr) {
            fmt::print(*stream, else_buffer.str());
            close_open_async_case(*this);
        }

        fmt::print(*stream, "case {}: {{", async_state_counter++);
        is_in_async_state_case = true;
    }


    return {};
}

any CodegenVisitor::visitWhileStmt(eelParser::WhileStmtContext *ctx) {
    auto const cond_expr = std::any_cast<std::string>(visit(ctx->conditionBlock()->expr()));
    auto const stmt = ctx->stmtBlock();

    auto seq = current_sequence->next();
    if (seq->is_async()) {
        close_open_async_case(*this);

        auto while_starting_case = async_state_counter++;

        auto original_stream = stream;
        std::stringstream buffer;
        stream = &buffer;

        visitChildren(stmt);

        stream = original_stream;

        fmt::print(*stream, "case {}: {{"
                            "if (!({})) {{ state.s = {}; return 0; }}",
                   while_starting_case, cond_expr, async_state_counter);

        fmt::print(*stream, "{} state.s = {}; return 0; }}", buffer.str(), while_starting_case);

    } else {
        // TODO check if there exists a case where we need to open a case here
        fmt::print(*stream, "while ({}) {{", cond_expr);
        visitChildren(stmt);
        fmt::print(*stream, "}}");
    }


    return {}; // TODO
}

any CodegenVisitor::visitBreakStmt(eelParser::BreakStmtContext *ctx) {
    fmt::print(*stream, "break;");
    return {};
}

any CodegenVisitor::visitContinueStmt(eelParser::ContinueStmtContext *ctx) {
    fmt::print(*stream, "continue;");
    return {};
}

/*
 * Pin statements
 */

any CodegenVisitor::visitSetPinValueStmt(eelParser::SetPinValueStmtContext *ctx) {
    auto pin_name = std::any_cast<std::string>(visit(ctx->fqn()));
    auto value = std::any_cast<std::string>(visit(ctx->expr()));

    fmt::print(*stream, "{}.write({});", pin_name, value);
    return {};
}

any CodegenVisitor::visitSetPinModeStmt(eelParser::SetPinModeStmtContext *ctx) {
    auto pin_name = std::any_cast<std::string>(visit(ctx->fqn()));
    auto value = std::any_cast<std::string>(visit(ctx->expr()));

    fmt::print(*stream, "{}.set_mode({});", pin_name, value);
    return {};
}

any CodegenVisitor::visitSetPinNumberStmt(eelParser::SetPinNumberStmtContext *ctx) {
    auto pin_name = std::any_cast<std::string>(visit(ctx->fqn()));
    auto value = std::any_cast<std::string>(visit(ctx->expr()));

    fmt::print(*stream, "{}.pin_id = {};", pin_name, value);
    return {};
}

/*
 * Helper functions
 */

static void generate_functor_core(const symbols::Function &function, CodegenVisitor &visitor) {
    auto outer_scope = visitor.current_scope;
    visitor.current_sequence = function.sequence;
    visitor.current_scope = visitor.current_sequence->start->scope;

    visitor.visitChildren(function.body);

    if (function.is_async()) {
        close_open_async_case(visitor);
        fmt::print(*visitor.stream, "case {}: {{ return 1; }}", visitor.async_state_counter);
    }


    visitor.current_scope = outer_scope; // restore previous scope
    visitor.current_sequence = nullptr;
}

void generate_sync_functor_type(
        std::iostream *stream,
        const symbols::Function &function,
        CodegenVisitor &visitor
) {
    const char *return_type_name = "void";
    if (function.has_return_type()) {
        return_type_name = function.return_type->value.type->type_target_name().c_str();
    }

    fmt::print(*stream,
               "struct {} {{ static {} invoke() {{",
               function.type_id,
               return_type_name);
    generate_functor_core(function, visitor);
    fmt::print(*stream, "}} }};");
}

void generate_async_functor_type(
        std::iostream *stream,
        const symbols::Function &function,
        CodegenVisitor &visitor
) {
    visitor.async_state_counter = 0;
    visitor.is_in_async_state_case = false;

    fmt::print(*stream,
               "struct {} : AsyncFunction {{"
               "struct State {{"
               "u8 s;",
               function.type_id);

    if (function.has_return_type()) {
        auto return_type = function.return_type->value.type;
        fmt::print(*stream, "{} r;", return_type->type_target_name());
    }

    auto seq = function.sequence;
    while (seq->current_block != nullptr) {
        if (seq->current_block->kind == SequencePoint::AsyncPoint) {
            auto &scope_members = seq->current_block->scope->members();
            for (const auto &member: scope_members) {
                auto symbol = visitor.table.get_symbol(member.second);
                if (symbol->kind != Symbol_::Kind::Variable)
                    continue;

                fmt::print(*stream, "{} {};\n",
                           symbol->value.type->type_target_name(),
                           generate_variable_id(symbol));
            }
        }

        auto b = seq->current_block;
        while (seq->current_block == b) {
            seq->next();
        }
    }

    seq->reset();

    fmt::print(*stream, "}};"); // End of State struct decl
    fmt::print(*stream, "static int step(State& state) {{"
                        "switch (state.s) {{");
    generate_functor_core(function, visitor);

    fmt::print(*stream, "}} }} static int begin_invoke(State& state"); // Start of begin_invoke param list

    for (auto param: function.parameters) {
        auto var = param->value.variable;
        fmt::print(*stream, ", {}& {}",
                   var->type->value.type->type_target_name(),
                   generate_variable_id(param));
    }

    fmt::print(*stream, ") {{ state.s = 0;"); // End of begin_invoke param list + start of body

    for (auto param: function.parameters) {
        auto name = generate_variable_id(param);
        fmt::print(*stream, "state.{} = {};", name, name);
    }

    fmt::print(*stream, "return step(state);}} }};");

}

void close_open_async_case(CodegenVisitor &visitor) {
    if (visitor.is_in_async_state_case) {
        fmt::print(*visitor.stream, "state.s += 1; return 0; }}");
        visitor.is_in_async_state_case = false;
    }
}

std::string generate_variable_id(Symbol symbol) {
    return fmt::format("__v{}", symbol->id);
}

void check_symbol(Symbol symbol, Symbol_::Kind expected_kind, const std::string &identifier) {
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

void resolve(Symbol &symbol, SymbolTable &table) {
    if (symbol->kind == Symbol_::Kind::Indirect) {
        symbol = table.get_symbol(symbol->value.indirect.id);
    }
}
#include <Visitors/async_analysis.hpp>

#include <Visitors/utility.hpp>
#include <symbols/event.hpp>
#include <symbols/function.hpp>

using namespace eel;
using namespace eel::visitors;

AsyncAnalysisVisitor::AsyncAnalysisVisitor(SymbolTable& table) : table(table) {}

static void mark_async(Symbol symbol);

static void set_ctx(AsyncAnalysisVisitor* visitor, Symbol symbol) {
    visitor->context_kind = AsyncAnalysisVisitor::Context::Kind::Symbol;
    visitor->context.symbol = symbol;
}

static void set_ctx(AsyncAnalysisVisitor* visitor, symbols::Function* event_handle) {
    visitor->context_kind = AsyncAnalysisVisitor::Context::Kind::EventHandle;
    visitor->context.event_handle = event_handle;
}

std::any AsyncAnalysisVisitor::visitEventDecl(eelParser::EventDeclContext* ctx) {
    auto symbol = table.root_scope->find(ctx->Identifier()->getText());
    if (symbol->kind == Symbol_::Kind::Event)
        throw InternalError(InternalError::Codegen, "Invalid symbol. Expected event.");

    auto block = ctx->stmtBlock();
    if (block != nullptr) {
        set_ctx(this, symbol);

        visitChildren(block);
    }

    return {};
}

std::any AsyncAnalysisVisitor::visitOnDecl(eelParser::OnDeclContext* ctx) {
    auto event_symbol = resolve_fqn(table.root_scope, ctx->fqn());
    if (event_symbol->kind == Symbol_::Kind::Event)
        throw InternalError(InternalError::Codegen, "Invalid event_symbol. Expected event.");

    auto& handle = event_symbol->value.event->get_handle(get_source_location(ctx->start));
    set_ctx(this, &handle);

    visitChildren(ctx->stmtBlock());

    return {};
}

std::any AsyncAnalysisVisitor::visitStmtBlock(eelParser::StmtBlockContext* ctx) {
    auto outer_context = context;
    auto outer_context_kind = context_kind;



    return {};
}

std::any AsyncAnalysisVisitor::visitAwaitStmt(eelParser::AwaitStmtContext*) {
    switch (context_kind) {
        case Context::Kind::Symbol:
            mark_async(context.symbol);
            break;
        case Context::Kind::EventHandle:
            context.event_handle->is_async = true;
            break;
        case Context::Kind::Block:
            break;
        case Context::Kind::None:
            throw InternalError(InternalError::Codegen, "await stmt in invalid context: None.");
        default:
            throw InternalError(InternalError::Codegen, "Invalid context type encountered.");
    }
    return {};
}

void mark_async(Symbol symbol) {
    using Kind = typename eel::Symbol_::Kind;
    switch (symbol->kind) {
        case Kind::Event: {
            auto& event = *symbol->value.event;
            if (!event.has_predicate)
                throw InternalError(InternalError::Codegen, "Cannot mark predicateless event as async.");

            event.predicate->is_async = true;
            break;
        }
        case Kind::Function:
            symbol->value.function->is_async = true;
            break;
        default:
            throw InternalError(InternalError::Codegen, "Cannot mark non event/function symbol as async.");
            break;
    }
}

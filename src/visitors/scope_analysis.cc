#pragma once

#include <Visitors/ScopeVisitor.hpp>

std::any ScopeVisitor::visitIdentifier(eelParser::IdentifierContext* ctx) {
    auto symbol_name = ctx->Identifier()->getText();
    auto symbol = current_scope->find(symbol_name);

    if (symbol.is_nullptr()) {
        // TODO pass additional info for potential error messages.
        symbol = current_scope->defer_symbol(symbol_name, Symbol_::Kind::Variable);
    }

    return symbol;
}

std::any ScopeVisitor::visitSetupDecl(eelParser::SetupDeclContext* ctx) {
    if (!current_scope->is_root())
        throw InternalError(InternalError::ScopeAnalysis, "SetupDecl node encountered in non-root scope context.");

    current_scope->declare_func("__eel_setup");
    return visit(ctx->stmtBlock());
}

std::any ScopeVisitor::visitLoopDecl(eelParser::LoopDeclContext* ctx) {
    if (!current_scope->is_root())
        throw InternalError(InternalError::ScopeAnalysis, "LoopDecl node encountered in non-root scope context.");

    current_scope->declare_func("__eel_loop");
    return visit(ctx->stmtBlock());
}

std::any ScopeVisitor::visitEventDecl(eelParser::EventDeclContext* ctx) {
    auto predicate = ctx->stmtBlock();
    auto name = ctx->Identifier()->getText();
    if(predicate == nullptr){
        current_scope->declare_event(name);
    } else {
        auto function = new symbols::Function;
        function->has_return_type = true;
        function->return_type = table->get_symbol(symbols::Primitive::boolean.id);
        function->scope = table->derive_scope(current_scope);

        current_scope->declare_event(name, function);
    }

    return {};
}
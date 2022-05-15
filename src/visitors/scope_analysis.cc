#pragma once

#include <Visitors/ScopeVisitor.hpp>

antlrcpp::Any ScopeVisitor::visitIdentifier(eelParser::IdentifierContext* ctx) {
    auto symbol_name = ctx->Identifier()->getText();
    auto symbol = current_scope->find(symbol_name);

    if (symbol.is_nullptr()) {
        // TODO pass additional info for potential error messages.
        symbol = current_scope->defer_symbol(symbol_name, Symbol_::Kind::Variable);
    }

    return symbol;
}
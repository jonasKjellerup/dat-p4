#pragma once

#include <eelBaseVisitor.h>
#include <antlr4-runtime.h>

#include <symbol_table.hpp>

namespace eel::visitors {
    using namespace eel;

    struct CodegenVisitor : eel::eelBaseVisitor {
        using Any = antlrcpp::Any;

        SymbolTable& table;
        FILE* stream;

        Scope current_scope;

        CodegenVisitor(SymbolTable& table, FILE* stream);

        Any visitIdentifier(eelParser::IdentifierContext *ctx) override;
        Any visitVariableDecl(eelParser::VariableDeclContext* ctx) override;

        Any visitEventDecl(eelParser::EventDeclContext* ctx) override;
        Any visitPinDecl(eelParser::PinDeclContext *ctx) override;

    };
}
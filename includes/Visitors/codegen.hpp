#pragma once

#include <eelBaseVisitor.h>
#include <antlr4-runtime.h>

#include <symbol_table.hpp>

namespace eel::visitors {
    using namespace eel;
    using std::any;

    struct CodegenVisitor : eel::eelBaseVisitor {

        SymbolTable& table;
        FILE* stream;

        Scope current_scope;

        CodegenVisitor(SymbolTable& table, FILE* stream);

        any visitProgram(eelParser::ProgramContext *ctx) override;

        // Expressions - Literals
        any visitBoolLiteral(eelParser::BoolLiteralContext *ctx) override;
        any visitCharLiteral(eelParser::CharLiteralContext *ctx) override;
        any visitFloatLiteral(eelParser::FloatLiteralContext *ctx) override;
        any visitIntegerLiteral(eelParser::IntegerLiteralContext *ctx) override;
        any visitStringLiteral(eelParser::StringLiteralContext *ctx) override;

        // Expressions - Access
        any visitIdentifier(eelParser::IdentifierContext *ctx) override;

        // Expressions - Arithmetic operators
        any visitPos(eelParser::PosContext *ctx) override;
        any visitNeg(eelParser::NegContext *ctx) override;
        any visitScalingExpr(eelParser::ScalingExprContext *ctx) override;
        any visitAdditiveExpr(eelParser::AdditiveExprContext *ctx) override;

        // Expressions - Logical/comparative operators

        any visitComparisonExpr(eelParser::ComparisonExprContext *ctx) override;
        any visitNot(eelParser::NotContext *ctx) override;

        // Declarations
        any visitVariableDecl(eelParser::VariableDeclContext* ctx) override;
        any visitSetupDecl(eelParser::SetupDeclContext *ctx) override;
        any visitLoopDecl(eelParser::LoopDeclContext *ctx) override;
        any visitEventDecl(eelParser::EventDeclContext* ctx) override;
        any visitPinDecl(eelParser::PinDeclContext *ctx) override;

    };
}
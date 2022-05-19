#pragma once

#include <eelBaseVisitor.h>
#include <antlr4-runtime.h>

#include <symbol_table.hpp>
#include <sequence.hpp>

namespace eel::visitors {
    using namespace eel;
    using std::any;

    struct CodegenVisitor : eelBaseVisitor {

        SymbolTable& table;
        FILE* stream;

        Scope current_scope;
        Sequence* current_sequence = nullptr;
        uint8_t  async_state_counter = 0;
        bool is_in_async_state_case = false;

        std::vector<symbols::Event*> events;

        CodegenVisitor(SymbolTable& table, FILE* stream);

        any visitProgram(eelParser::ProgramContext *ctx) override;

        // Expressions - Literals
        any visitBoolLiteral(eelParser::BoolLiteralContext *ctx) override;
        any visitCharLiteral(eelParser::CharLiteralContext *ctx) override;
        any visitFloatLiteral(eelParser::FloatLiteralContext *ctx) override;
        any visitIntegerLiteral(eelParser::IntegerLiteralContext *ctx) override;
        any visitStringLiteral(eelParser::StringLiteralContext *ctx) override;

        // Expressions - Access/assign
        any visitIdentifier(eelParser::IdentifierContext *ctx) override;
        any visitAssignExpr(eelParser::AssignExprContext *ctx) override;

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
        any visitOnDecl(eelParser::OnDeclContext* ctx) override;
        any visitPinDecl(eelParser::PinDeclContext*ctx) override;

        // Stmts
        any visitStmt(eelParser::StmtContext *ctx) override;
        any visitStmtBlock(eelParser::StmtBlockContext* ctx) override;
        any visitAwaitStmt(eelParser::AwaitStmtContext* ctx) override;
        any visitReturnStmt(eelParser::ReturnStmtContext *ctx) override;

    };
}
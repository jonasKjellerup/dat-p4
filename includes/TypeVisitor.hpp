#pragma once
#include <eelBaseVisitor.h>
#include <antlr4-runtime.h>
#include <symbol_table.hpp>
#include <symbols/type.hpp>
using namespace eel;

class TypeVisitor : eelBaseVisitor {
public:

    SymbolTable* table;
    Scope current_scope;
    Scope previous_scope;
    explicit TypeVisitor(SymbolTable* _table) {
        table = _table;
        symbols::Primitive::register_primitives(table->root_scope);
        current_scope = previous_scope = table->root_scope;
    }
    /*
     * This rule is here to act as a starting point for the traversal of the parse tree.
     * */
    antlrcpp::Any visitProgram (eelParser::ProgramContext* ctx) override {
        return visitChildren(ctx);
    }

    /*
     * Variable declarations
     * */
    antlrcpp::Any visitVariableDecl (eelParser::VariableDeclContext* ctx) override {
        return visitChildren(ctx);
    }
    antlrcpp::Any visitPinDecl (eelParser::PinDeclContext* ctx) override {
        return visitChildren(ctx);
    }
    antlrcpp::Any visitArrayDecl (eelParser::ArrayDeclContext* ctx) override {
        return visitChildren(ctx);
    }
    antlrcpp::Any visitReferenceDecl (eelParser::ReferenceDeclContext* ctx) override {
        return visitChildren(ctx);
    }
    antlrcpp::Any visitPointerDecl (eelParser::PointerDeclContext* ctx) override {
        return visitChildren(ctx);
    }
    antlrcpp::Any visitTypedIdentifier (eelParser::TypedIdentifierContext* ctx) override {
        return visitChildren(ctx);
    }

    /*
     * Literal Expressions
     * */
    antlrcpp::Any visitIntegerLiteral (eelParser::IntegerLiteralContext* ctx) override {
        return visitChildren(ctx);
    }
    antlrcpp::Any visitFloatLiteral (eelParser::FloatLiteralContext* ctx) override {
        return visitChildren(ctx);
    }
    antlrcpp::Any visitBoolLiteral (eelParser::BoolLiteralContext* ctx) override {
        return visitChildren(ctx);
    }
    antlrcpp::Any visitCharLiteral (eelParser::CharLiteralContext* ctx) override {
        return visitChildren(ctx);
    }
    antlrcpp::Any visitStringLiteral (eelParser::StringLiteralContext* ctx) override {
        return visitChildren(ctx);
    }

    /*
     * Function Expressions
     * */
    antlrcpp::Any visitFnCallExpr (eelParser::FnCallExprContext* ctx) override {
        return visitChildren(ctx);
    }
    antlrcpp::Any visitInstanceAssociatedFnCallExpr (eelParser::InstanceAssociatedFnCallExprContext* ctx) override {
        return visitChildren(ctx);
    }
    antlrcpp::Any visitExprList (eelParser::ExprListContext* ctx) override {
        return visitChildren(ctx);
    }
    /*
     * Init Expressions
     * */
    antlrcpp::Any visitStructExpr (eelParser::StructExprContext* ctx) override {
        return visitChildren(ctx);
    }
    antlrcpp::Any visitArrayExpr (eelParser::ArrayExprContext* ctx) override {
        return visitChildren(ctx);
    }
    antlrcpp::Any visitPointerExpr (eelParser::PointerExprContext* ctx) override {
        return visitChildren(ctx);
    }
    antlrcpp::Any visitReferenceExpr (eelParser::ReferenceExprContext* ctx) override {
        return visitChildren(ctx);
    }
    antlrcpp::Any visitFqnExpr (eelParser::FqnExprContext* ctx) override {
        return visitChildren(ctx);
    }

    /*
     * Math Expressions
     * */
    antlrcpp::Any visitScalingExpr (eelParser::ScalingExprContext* ctx) override {
        return visitChildren(ctx);
    }
    antlrcpp::Any visitAdditiveExpr (eelParser::AdditiveExprContext* ctx) override {
        return visitChildren(ctx);
    }
    antlrcpp::Any visitShiftingExpr (eelParser::ShiftingExprContext* ctx) override {
        return visitChildren(ctx);
    }
    antlrcpp::Any visitComparisonExpr (eelParser::ComparisonExprContext* ctx) override {
        return visitChildren(ctx);
    }
    antlrcpp::Any visitAndExpr (eelParser::AndExprContext* ctx) override {
        return visitChildren(ctx);
    }
    antlrcpp::Any visitXorExpr (eelParser::XorExprContext* ctx) override {
        return visitChildren(ctx);
    }
    antlrcpp::Any visitOrExpr (eelParser::OrExprContext* ctx) override {
        return visitChildren(ctx);
    }
    antlrcpp::Any visitLAndExpr (eelParser::LAndExprContext* ctx) override {
        return visitChildren(ctx);
    }
    antlrcpp::Any visitLOrExpr (eelParser::LOrExprContext* ctx) override {
        return visitChildren(ctx);
    }
    antlrcpp::Any visitAssignExpr (eelParser::AssignExprContext* ctx) override {
        return visitChildren(ctx);
    }
    antlrcpp::Any visitAdditiveAssignExpr (eelParser::AdditiveAssignExprContext* ctx) override {
        return visitChildren(ctx);
    }
    antlrcpp::Any visitScalingAssignExpr (eelParser::ScalingAssignExprContext* ctx) override {
        return visitChildren(ctx);
    }
    antlrcpp::Any visitShiftingAssignExpr (eelParser::ShiftingAssignExprContext* ctx) override {
        return visitChildren(ctx);
    }
    antlrcpp::Any visitOrAssignExpr (eelParser::OrAssignExprContext* ctx) override {
        return visitChildren(ctx);
    }
    antlrcpp::Any visitAndAssignExpr (eelParser::AndAssignExprContext* ctx) override {
        return visitChildren(ctx);
    }
    antlrcpp::Any visitXorAssignExpr (eelParser::XorAssignExprContext* ctx) override {
        return visitChildren(ctx);
    }

    /*
     * Other
     * */
    antlrcpp::Any visitReadPinExpr (eelParser::ReadPinExprContext* ctx) override {
        return visitChildren(ctx);
    }
    antlrcpp::Any visitPinStmt (eelParser::PinStmtContext* ctx) override {
        return visitChildren(ctx);
    }

};
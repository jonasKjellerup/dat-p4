#pragma once
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#pragma clang diagnostic ignored "-Wunused-variable"

#include <utility>

#include "eelBaseVisitor.h"
#include "antlr4-runtime.h"
#include "symbol_table.hpp"
#include "symbols/type.hpp"
#include "symbols/variable.hpp"
#include "symbols/constant.hpp"
#include "symbols/event.hpp"
#include "error.hpp"

using namespace eel;
using namespace antlr4;


class TypeVisitor : eelBaseVisitor {
public:
    struct Type {
        enum Kind{
            Undefined,
            NotAType,
            None,
            Integer,
            Float,
            Bool,
            Char,
            String,
        };

        union Value {
            Symbol symbol;
            Kind literal;
        };

        explicit Type();
        explicit Type(Kind _value, Token* _token);
        explicit Type(Symbol _value, Token* _token);

        [[nodiscard]] Kind literal() const;
        [[nodiscard]] Symbol symbol() const;

        [[nodiscard]] std::string to_string() const;
        bool equals(Type* other);

        [[nodiscard]] bool is_null() const;

        Value value {};
        Token* token;
        bool is_literal;
    };

    SymbolTable* table;
    Scope current_scope;
    Scope previous_scope;
    size_t scope_index;
    Type expected_type;
    std::vector<Error> errors;
    explicit TypeVisitor(SymbolTable* _table);

    /*
     * This rule is here to act as a starting point for the traversal of the parse tree.
     * */
    antlrcpp::Any visitProgram (eelParser::ProgramContext* ctx) override;

    /*
     * Variable declarations
     * */
    antlrcpp::Any visitVariableDecl (eelParser::VariableDeclContext* ctx) override;
    antlrcpp::Any visitStaticDecl (eelParser::StaticDeclContext* ctx) override;
    antlrcpp::Any visitConstDecl (eelParser::ConstDeclContext* ctx) override;
    antlrcpp::Any visitPinDecl (eelParser::PinDeclContext* ctx) override;
    antlrcpp::Any visitArrayDecl (eelParser::ArrayDeclContext* ctx) override;
    antlrcpp::Any visitReferenceDecl (eelParser::ReferenceDeclContext* ctx) override;
    antlrcpp::Any visitPointerDecl (eelParser::PointerDeclContext* ctx) override;
    antlrcpp::Any visitTypedIdentifier (eelParser::TypedIdentifierContext* ctx) override;

    /*
     *  Other Declarations
     * */
    antlrcpp::Any visitOnDecl (eelParser::OnDeclContext* ctx) override;

    /*
     * Literal Expressions
     * */
    antlrcpp::Any visitIntegerLiteral (eelParser::IntegerLiteralContext* ctx) override;
    antlrcpp::Any visitFloatLiteral (eelParser::FloatLiteralContext* ctx) override;
    antlrcpp::Any visitBoolLiteral (eelParser::BoolLiteralContext* ctx) override;
    antlrcpp::Any visitCharLiteral (eelParser::CharLiteralContext* ctx) override;
    antlrcpp::Any visitStringLiteral (eelParser::StringLiteralContext* ctx) override;

    /*
     * Function Expressions
     * */
    antlrcpp::Any visitFnCallExpr (eelParser::FnCallExprContext* ctx) override;
    antlrcpp::Any visitInstanceAssociatedFnCallExpr (eelParser::InstanceAssociatedFnCallExprContext* ctx) override;
    antlrcpp::Any visitExprList (eelParser::ExprListContext* ctx) override;

    /*
     * Init Expressions
     * */
    antlrcpp::Any visitStructExpr (eelParser::StructExprContext* ctx) override;
    antlrcpp::Any visitArrayExpr (eelParser::ArrayExprContext* ctx) override;
    antlrcpp::Any visitPointerExpr (eelParser::PointerExprContext* ctx) override;
    antlrcpp::Any visitReferenceExpr (eelParser::ReferenceExprContext* ctx) override;
    antlrcpp::Any visitFqnExpr (eelParser::FqnExprContext* ctx) override;

    /*
     * Math Expressions
     * */
    antlrcpp::Any visitAdditiveExpr (eelParser::AdditiveExprContext* ctx) override;
    antlrcpp::Any visitScalingExpr (eelParser::ScalingExprContext* ctx) override;
    antlrcpp::Any visitShiftingExpr (eelParser::ShiftingExprContext* ctx) override;
    antlrcpp::Any visitComparisonExpr (eelParser::ComparisonExprContext* ctx) override;
    antlrcpp::Any visitAndExpr (eelParser::AndExprContext* ctx) override;
    antlrcpp::Any visitXorExpr (eelParser::XorExprContext* ctx) override;
    antlrcpp::Any visitOrExpr (eelParser::OrExprContext* ctx) override;

    antlrcpp::Any visitLAndExpr (eelParser::LAndExprContext* ctx) override;
    antlrcpp::Any visitLOrExpr (eelParser::LOrExprContext* ctx) override;

    antlrcpp::Any visitAssignExpr (eelParser::AssignExprContext* ctx) override;
    antlrcpp::Any visitAdditiveAssignExpr (eelParser::AdditiveAssignExprContext* ctx) override;
    antlrcpp::Any visitScalingAssignExpr (eelParser::ScalingAssignExprContext* ctx) override;
    antlrcpp::Any visitShiftingAssignExpr (eelParser::ShiftingAssignExprContext* ctx) override;
    antlrcpp::Any visitOrAssignExpr (eelParser::OrAssignExprContext* ctx) override;
    antlrcpp::Any visitAndAssignExpr (eelParser::AndAssignExprContext* ctx) override;
    antlrcpp::Any visitXorAssignExpr (eelParser::XorAssignExprContext* ctx) override;

    /*
     * Other Expr
     * */
    antlrcpp::Any visitReadPinExpr (eelParser::ReadPinExprContext* ctx) override;

    /*
     * Stmts
     * */
    antlrcpp::Any visitSetPinValueStmt (eelParser::SetPinValueStmtContext* ctx) override;
    antlrcpp::Any visitSetPinModeStmt (eelParser::SetPinModeStmtContext* ctx) override;
    antlrcpp::Any visitSetPinNumberStmt (eelParser::SetPinNumberStmtContext* ctx) override;
    antlrcpp::Any visitStmtBlock (eelParser::StmtBlockContext* ctx) override;
    antlrcpp::Any visitAwaitStmt (eelParser::AwaitStmtContext* ctx) override;

    /*
     * Other
     */
    antlrcpp::Any visitConditionBlock (eelParser::ConditionBlockContext* ctx) override;

};
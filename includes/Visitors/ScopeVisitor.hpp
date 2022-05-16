#pragma once
#include "eelBaseVisitor.h"
#include "antlr4-runtime.h"
#include "symbol_table.hpp"
#include "symbols/type.hpp"
#include "symbols/variable.hpp"
#include "symbols/event.hpp"

using namespace eel;
struct TypedIdentifier {
    Symbol type;
    std::string identifier;
};


class ScopeVisitor : eelBaseVisitor {
public:
    /*
     * TODO: Might need to add some fields for storing additional context
     * */
    SymbolTable* table;
    Scope current_scope;
    Scope previous_scope;
    symbols::Event* current_event;

    explicit ScopeVisitor(SymbolTable* _table);

    /*
     * This rule is here to act as a starting point for the traversal of the parse tree.
     * */
    antlrcpp::Any visitProgram (eelParser::ProgramContext* ctx) override;

    /*
     * Top level declarations
     * */
    antlrcpp::Any visitLoopDecl (eelParser::LoopDeclContext* ctx) override;
    antlrcpp::Any visitSetupDecl(eelParser::SetupDeclContext* ctx) override;
    antlrcpp::Any visitIncludeDirective (eelParser::IncludeDirectiveContext* ctx) override;
    antlrcpp::Any visitTraitImplBlock (eelParser::TraitImplBlockContext* ctx) override;
    antlrcpp::Any visitImplBlock (eelParser::ImplBlockContext* ctx) override;

    /*
     * Variable declarations
     * */
    antlrcpp::Any visitVariableDecl (eelParser::VariableDeclContext* ctx) override;
    antlrcpp::Any visitConstDecl (eelParser::ConstDeclContext* ctx) override;
    antlrcpp::Any visitStaticDecl (eelParser::StaticDeclContext* ctx) override;
    antlrcpp::Any visitPinDecl (eelParser::PinDeclContext* ctx) override;
    antlrcpp::Any visitArrayDecl (eelParser::ArrayDeclContext* ctx) override;
    antlrcpp::Any visitReferenceDecl (eelParser::ReferenceDeclContext* ctx) override;
    antlrcpp::Any visitPointerDecl (eelParser::PointerDeclContext* ctx) override;
    antlrcpp::Any visitTypedIdentifier (eelParser::TypedIdentifierContext* ctx) override;

    /*
     * Type declarations
     * */
    antlrcpp::Any visitStructDecl (eelParser::StructDeclContext* ctx) override;
    antlrcpp::Any visitUnionDecl (eelParser::UnionDeclContext* ctx) override;
    antlrcpp::Any visitUntaggedUnionDecl (eelParser::UntaggedUnionDeclContext* ctx) override;
    antlrcpp::Any visitEnumDecl (eelParser::EnumDeclContext* ctx) override;
    antlrcpp::Any visitEventDecl (eelParser::EventDeclContext* ctx) override;
    antlrcpp::Any visitIntervalDecl (eelParser::IntervalDeclContext* ctx) override;
    antlrcpp::Any visitOnDecl (eelParser::OnDeclContext* ctx) override;
    antlrcpp::Any visitTraitDecl (eelParser::TraitDeclContext* ctx) override;

    /*
     * Function declarations
     * */
    antlrcpp::Any visitFnDecl (eelParser::FnDeclContext* ctx) override;
    antlrcpp::Any visitParamList (eelParser::ParamListContext* ctx) override;
    antlrcpp::Any visitFnParam (eelParser::FnParamContext* ctx) override;

    /*
     * Associated Function declarations
     * */
    antlrcpp::Any visitInstanceAssociatedFn (eelParser::InstanceAssociatedFnContext* ctx) override;
    antlrcpp::Any visitPartialInstanceAssociatedFn (eelParser::PartialInstanceAssociatedFnContext* ctx) override;
    antlrcpp::Any visitTypeAssociatedFn (eelParser::TypeAssociatedFnContext* ctx) override;
    antlrcpp::Any visitPartialTypeAssociatedFn (eelParser::PartialTypeAssociatedFnContext* ctx) override;
    antlrcpp::Any visitInstanceAssocParamList (eelParser::InstanceAssocParamListContext* ctx) override;

    /*
     * Namespace declaration
     * */
    antlrcpp::Any visitNamespaceDecl (eelParser::NamespaceDeclContext* ctx) override;

    /*
     * Access Expressions
     * */
    antlrcpp::Any visitFqnExpr (eelParser::FqnExprContext* ctx) override;
    antlrcpp::Any visitStructExpr (eelParser::StructExprContext* ctx) override;
    antlrcpp::Any visitArrayExpr (eelParser::ArrayExprContext* ctx) override;
    antlrcpp::Any visitPointerExpr (eelParser::PointerExprContext* ctx) override;
    antlrcpp::Any visitReferenceExpr (eelParser::ReferenceExprContext* ctx) override;
    antlrcpp::Any visitReadPinExpr (eelParser::ReadPinExprContext* ctx) override;

    /*
     * Function Expressions
     * */
    antlrcpp::Any visitFnCallExpr (eelParser::FnCallExprContext* ctx) override;
    antlrcpp::Any visitInstanceAssociatedFnCallExpr (eelParser::InstanceAssociatedFnCallExprContext* ctx) override;
    antlrcpp::Any visitExprList (eelParser::ExprListContext* ctx) override;

    /*
     * Statements
     * */
    antlrcpp::Any visitStmtBlock (eelParser::StmtBlockContext* ctx) override;
};

#pragma once
#include <eelBaseVisitor.h>
#include <antlr4-runtime.h>
using namespace eel;

class ScopeVisitor : eelBaseVisitor {
public:
    /*
     * TODO: Might need to add some fields for storing additional context
     * */

    /*
     * This rule is here to act as a starting point for the traversal of the parse tree.
     * */
    antlrcpp::Any visitProgram (eelParser::ProgramContext* ctx) override {
        return visitChildren(ctx);
    }

    /*
     * Declaration Section
     * */

    antlrcpp::Any visitStmtBlock (eelParser::StmtBlockContext* ctx) override {
        return visitChildren(ctx);
    }

    /*
     * Top level declarations
     * */
    antlrcpp::Any visitLoopDecl (eelParser::LoopDeclContext* ctx) override {
            return visitChildren(ctx);
    }
    antlrcpp::Any visitSetupDecl (eelParser::SetupDeclContext* ctx) override {
        return visitChildren(ctx);
    }
    antlrcpp::Any visitIncludeDirective (eelParser::IncludeDirectiveContext* ctx) override {
        return visitChildren(ctx);
    }
    antlrcpp::Any visitTraitImplBlock (eelParser::TraitImplBlockContext* ctx) override {
        return visitChildren(ctx);
    }
    antlrcpp::Any visitImplBlock (eelParser::ImplBlockContext* ctx) override {
        return visitChildren(ctx);
    }

    /*
     * Variable declarations
     * */
    antlrcpp::Any visitVariableDecl (eelParser::VariableDeclContext* ctx) override {
        return visitChildren(ctx);
    }
    antlrcpp::Any visitConstDecl (eelParser::ConstDeclContext* ctx) override {
        return visitChildren(ctx);
    }
    antlrcpp::Any visitStaticDecl (eelParser::StaticDeclContext* ctx) override {
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
     * Type declarations
     * */
    antlrcpp::Any visitStructDecl (eelParser::StructDeclContext* ctx) override {
        return visitChildren(ctx);
    }
    antlrcpp::Any visitUnionDecl (eelParser::UnionDeclContext* ctx) override {
        return visitChildren(ctx);
    }
    antlrcpp::Any visitUntaggedUnionDecl (eelParser::UntaggedUnionDeclContext* ctx) override {
        return visitChildren(ctx);
    }
    antlrcpp::Any visitEnumDecl (eelParser::EnumDeclContext* ctx) override {
        return visitChildren(ctx);
    }
    antlrcpp::Any visitEventDecl (eelParser::EventDeclContext* ctx) override {
        return visitChildren(ctx);
    }
    antlrcpp::Any visitIntervalDecl (eelParser::IntervalDeclContext* ctx) override {
        return visitChildren(ctx);
    }
    antlrcpp::Any visitOnDecl (eelParser::OnDeclContext* ctx) override {
        return visitChildren(ctx);
    }
    antlrcpp::Any visitTraitDecl (eelParser::TraitDeclContext* ctx) override {
        return visitChildren(ctx);
    }

    /*
     * Function declarations
     * */
    antlrcpp::Any visitFnDecl (eelParser::FnDeclContext* ctx) override {
        return visitChildren(ctx);
    }
    antlrcpp::Any visitParamList (eelParser::ParamListContext* ctx) override {
        return visitChildren(ctx);
    }
    antlrcpp::Any visitFnParam (eelParser::FnParamContext* ctx) override {
        return visitChildren(ctx);
    }

    /*
     * Associated Function declarations
     * */
    antlrcpp::Any visitInstanceAssociatedFn (eelParser::InstanceAssociatedFnContext* ctx) override {
        return visitChildren(ctx);
    }
    antlrcpp::Any visitPartialInstanceAssociatedFn (eelParser::PartialInstanceAssociatedFnContext* ctx) override {
        return visitChildren(ctx);
    }
    antlrcpp::Any visitTypeAssociatedFn (eelParser::TypeAssociatedFnContext* ctx) override {
        return visitChildren(ctx);
    }
    antlrcpp::Any visitPartialTypeAssociatedFn (eelParser::PartialTypeAssociatedFnContext* ctx) override {
        return visitChildren(ctx);
    }
    antlrcpp::Any visitInstanceAssocParamList (eelParser::InstanceAssocParamListContext* ctx) override {
        return visitChildren(ctx);
    }

    /*
     * Namespace declaration
     * */
    antlrcpp::Any visitNamespaceDecl (eelParser::NamespaceDeclContext* ctx) override {
        return visitChildren(ctx);
    }


    /*
     * Usage Section
     * */

    /*
     * Access Expressions
     * */
    antlrcpp::Any visitFqnExpr (eelParser::FqnExprContext* ctx) override {
        return visitChildren(ctx);
    }
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
    antlrcpp::Any visitReadPinExpr (eelParser::ReadPinExprContext* ctx) override {
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


};

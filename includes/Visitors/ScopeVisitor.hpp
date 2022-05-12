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
    explicit ScopeVisitor(SymbolTable* _table) {
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
     * Declaration Section
     * */


    /*
     * Top level declarations
     * */
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
        auto res = any_cast<TypedIdentifier>(visit(ctx->typedIdentifier()));
        current_scope->declare_var(res.type, res.identifier);
        return {};
    }
    antlrcpp::Any visitConstDecl (eelParser::ConstDeclContext* ctx) override {
        auto res = any_cast<TypedIdentifier>(visit(ctx->typedIdentifier()));
        current_scope->declare_const(res.type, res.identifier, {});
        return {};
    }
    antlrcpp::Any visitStaticDecl (eelParser::StaticDeclContext* ctx) override {
        auto res = any_cast<TypedIdentifier>(visit(ctx->typedIdentifier()));
        current_scope->declare_var(res.type, res.identifier, true);
        return {};
    }
    antlrcpp::Any visitPinDecl (eelParser::PinDeclContext* ctx) override {
        auto identifier = ctx->Identifier()->getText();
        auto _type = ctx->PinType()->getText();
        auto type = current_scope->find(_type);
        if (type.is_nullptr()){
            current_scope->defer_symbol(_type, Symbol_::Kind::Type);
        }else if (type->kind != Symbol_::Kind::Type){
            std::cout << "Symbol is not a type" << std::endl;
        }
        current_scope->declare_var(type, identifier);
        return {};
    }

    antlrcpp::Any visitArrayDecl (eelParser::ArrayDeclContext* ctx) override {
        auto res = any_cast<TypedIdentifier>(visit(ctx->typedIdentifier()));
        return {};
    }
    antlrcpp::Any visitReferenceDecl (eelParser::ReferenceDeclContext* ctx) override {
        return visitChildren(ctx);
    }
    antlrcpp::Any visitPointerDecl (eelParser::PointerDeclContext* ctx) override {
        return visitChildren(ctx);
    }

    antlrcpp::Any visitTypedIdentifier (eelParser::TypedIdentifierContext* ctx) override {
        auto identifier = ctx->Identifier()->getText();
        auto _type = ctx->type()->getText();
        auto type = current_scope->find(_type);
        if (type.is_nullptr()){
            current_scope->defer_symbol(_type, Symbol_::Kind::Type);
        } else if (type->kind != Symbol_::Kind::Type){
            std::cout << "Symbol is not a type" << std::endl;
            // TODO: Throw error
        }
        return TypedIdentifier{ type, identifier};
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
        auto predicate = ctx->stmtBlock();
        if(predicate == nullptr){
            current_scope->declare_event(ctx->Identifier()->getText());
        } else {
            auto name = ctx->Identifier()->getText();
            auto func_name = "Event-" + name;
            auto scope = any_cast<Scope>(visit(ctx->stmtBlock()));
            auto symbol = current_scope->declare_func(func_name, current_scope->find("bool"), scope);
            current_scope->declare_event(name, symbol->value.function);
        }

        return {};
    }
    antlrcpp::Any visitIntervalDecl (eelParser::IntervalDeclContext* ctx) override {
        return visitChildren(ctx);
    }
    antlrcpp::Any visitOnDecl (eelParser::OnDeclContext* ctx) override {
        current_scope->declare_event_handle(ctx->fqn()->getText());
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

    /*
     * Statements
     * */
    antlrcpp::Any visitStmtBlock (eelParser::StmtBlockContext* ctx) override {
        auto _previous_scope = previous_scope;
        previous_scope = current_scope;
        current_scope = table->derive_scope();
        auto result = current_scope;
        visitChildren(ctx);
        current_scope = previous_scope;
        previous_scope = _previous_scope;
        return result;
    }
    antlrcpp::Any visitForEachStmt (eelParser::ForEachStmtContext* ctx) override {
        auto array_symbol = current_scope->find(ctx->x3->getText());
        if(array_symbol->kind != Symbol_::Kind::Variable){
            //TODO: Throw error
        }
        symbols::Variable* array = array_symbol->value.variable;
        auto type = array->type;
        if(type->kind != Symbol_::Kind::Type){
            //TODO: Throw error
        } else {
            current_scope->declare_var(type, ctx->x1->getText());
        }
        if(ctx->x2 != nullptr){
            // TODO: Do constant analysis to get the array size and choose a appropriate size to increase performance.
            auto _type = current_scope->find("u64");
            if(_type->kind != Symbol_::Kind::Type){
                // TODO: Throw error
            } else{
                current_scope->declare_var(_type, ctx->x2->getText());
            }
        }
        return {};
    }
};

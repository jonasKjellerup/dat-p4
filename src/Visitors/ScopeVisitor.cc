#include <Visitors/ScopeVisitor.hpp>

ScopeVisitor::ScopeVisitor(SymbolTable* _table) {
    table = _table;
    symbols::Primitive::register_primitives(table->root_scope);
    current_scope = previous_scope = table->root_scope;
    current_event = nullptr;
}

antlrcpp::Any ScopeVisitor::visitProgram(eelParser::ProgramContext* ctx) {
    return visitChildren(ctx);
}


/*
 * Top level declarations
 * */
antlrcpp::Any ScopeVisitor::visitLoopDecl(eelParser::LoopDeclContext* ctx) {
    Symbol func = current_scope->declare_func("loop", {}); // has no return type
    auto scope = any_cast<Scope>(visit(ctx->stmtBlock()));
    func->value.function->scope = scope;
    return {};
}

antlrcpp::Any ScopeVisitor::visitSetupDecl(eelParser::SetupDeclContext* ctx) {
    Symbol func = current_scope->declare_func("setup", {}); // has no return type
    auto scope = any_cast<Scope>(visit(ctx->stmtBlock()));
    func->value.function->scope = scope;
    return {};
}

antlrcpp::Any ScopeVisitor::visitIncludeDirective(eelParser::IncludeDirectiveContext* ctx) {
    return eelBaseVisitor::visitIncludeDirective(ctx);
}

antlrcpp::Any ScopeVisitor::visitTraitImplBlock(eelParser::TraitImplBlockContext* ctx) {
    return eelBaseVisitor::visitTraitImplBlock(ctx);
}

antlrcpp::Any ScopeVisitor::visitImplBlock(eelParser::ImplBlockContext* ctx) {
    return eelBaseVisitor::visitImplBlock(ctx);
}


/*
 * Variable declarations
 * */
antlrcpp::Any ScopeVisitor::visitVariableDecl (eelParser::VariableDeclContext* ctx) {
    auto res = any_cast<TypedIdentifier>(visit(ctx->typedIdentifier()));
    current_scope->declare_var(res.type, res.identifier);
    return {};
}

antlrcpp::Any ScopeVisitor::visitConstDecl(eelParser::ConstDeclContext* ctx) {
    auto res = any_cast<TypedIdentifier>(visit(ctx->typedIdentifier()));
    current_scope->declare_const(res.type, res.identifier, {});
    return {};
}

antlrcpp::Any ScopeVisitor::visitStaticDecl(eelParser::StaticDeclContext* ctx) {
    auto res = any_cast<TypedIdentifier>(visit(ctx->typedIdentifier()));
    current_scope->declare_var(res.type, res.identifier, true);
    return {};
}

antlrcpp::Any ScopeVisitor::visitPinDecl(eelParser::PinDeclContext* ctx) {
    auto identifier = ctx->Identifier()->getText();
    auto _type = ctx->PinType()->getText();
    auto type = current_scope->find(_type);
    if (type.is_nullptr()){
        current_scope->defer_symbol(_type, Symbol_::Kind::Type);
    }
    current_scope->declare_var(type, identifier);
    return {};
}

antlrcpp::Any ScopeVisitor::visitArrayDecl(eelParser::ArrayDeclContext* ctx) {
    return eelBaseVisitor::visitArrayDecl(ctx);
}

antlrcpp::Any ScopeVisitor::visitReferenceDecl(eelParser::ReferenceDeclContext* ctx) {
    return eelBaseVisitor::visitReferenceDecl(ctx);
}

antlrcpp::Any ScopeVisitor::visitPointerDecl(eelParser::PointerDeclContext* ctx) {
    return eelBaseVisitor::visitPointerDecl(ctx);
}

antlrcpp::Any ScopeVisitor::visitTypedIdentifier(eelParser::TypedIdentifierContext* ctx) {
    auto identifier = ctx->Identifier()->getText();
    auto _type = ctx->type()->getText();
    auto type = current_scope->find(_type);
    if (type.is_nullptr()){
        type = current_scope->defer_symbol(_type, Symbol_::Kind::Type);
    }
    return TypedIdentifier{ type, identifier };
}


/*
 * Type declarations
 * */
antlrcpp::Any ScopeVisitor::visitStructDecl(eelParser::StructDeclContext* ctx) {
    return eelBaseVisitor::visitStructDecl(ctx);
}

antlrcpp::Any ScopeVisitor::visitUnionDecl(eelParser::UnionDeclContext* ctx) {
    return eelBaseVisitor::visitUnionDecl(ctx);
}

antlrcpp::Any ScopeVisitor::visitUntaggedUnionDecl(eelParser::UntaggedUnionDeclContext* ctx) {
    return eelBaseVisitor::visitUntaggedUnionDecl(ctx);
}

antlrcpp::Any ScopeVisitor::visitEnumDecl(eelParser::EnumDeclContext* ctx) {
    return eelBaseVisitor::visitEnumDecl(ctx);
}

antlrcpp::Any ScopeVisitor::visitEventDecl(eelParser::EventDeclContext* ctx) {
    auto name = ctx->Identifier()->getText();
    auto func_name = "Event-" + name;
    auto event = current_scope->find(name);
    auto predicate = ctx->stmtBlock();
    if(event.is_nullptr()){
        // Create predicate less event.
        if(predicate == nullptr){
            current_scope->declare_event(ctx->Identifier()->getText());
        } else {
            // Create event with predicate
            auto func = current_scope->declare_func(func_name, current_scope->find("bool"));
            auto event_symbol = current_scope->declare_event(name, func->value.function);
            if(!event_symbol.is_nullptr()) {
                this->current_event = event_symbol->value.event;
                auto scope = any_cast<Scope>(visit(ctx->stmtBlock()));
                func->value.function->scope = scope;
                this->current_event = nullptr;
            }
        }
    } else {
        // Name already used
        if(event->kind != Symbol_::Kind::Event){
            auto error = Error(Error::AlreadyDefined, ctx->Identifier()->getSymbol(), ctx, "");
            this->errors.push_back(error);
            return {};
        }
        // if no predicate supplied or event has a predicate
        if(predicate == nullptr || event->value.event->is_complete){
            auto error = Error(Error::DuplicateEvent, ctx->Identifier()->getSymbol(), ctx, "");
            this->errors.push_back(error);
            return {};
        }
        // Otherwise defined deferred event.
        auto func = current_scope->declare_func(func_name, current_scope->find("bool"));
        auto event_symbol = current_scope->declare_event(name, func->value.function);
        if(!event_symbol.is_nullptr()) {
            this->current_event = event_symbol->value.event;
            auto scope = any_cast<Scope>(visit(ctx->stmtBlock()));
            func->value.function->scope = scope;
            this->current_event = nullptr;
        }
    }
    return {};
}

antlrcpp::Any ScopeVisitor::visitIntervalDecl(eelParser::IntervalDeclContext* ctx) {
    return eelBaseVisitor::visitIntervalDecl(ctx);
}

antlrcpp::Any ScopeVisitor::visitOnDecl(eelParser::OnDeclContext* ctx) {
    current_scope->declare_event_handle(ctx->fqn()->getText());
    return visitChildren(ctx);
}

antlrcpp::Any ScopeVisitor::visitTraitDecl(eelParser::TraitDeclContext* ctx) {
    return eelBaseVisitor::visitTraitDecl(ctx);
}


/*
 * Function declarations
 * */
antlrcpp::Any ScopeVisitor::visitFnDecl(eelParser::FnDeclContext* ctx) {
    return eelBaseVisitor::visitFnDecl(ctx);
}

antlrcpp::Any ScopeVisitor::visitParamList(eelParser::ParamListContext* ctx) {
    return eelBaseVisitor::visitParamList(ctx);
}

antlrcpp::Any ScopeVisitor::visitFnParam(eelParser::FnParamContext* ctx) {
    return eelBaseVisitor::visitFnParam(ctx);
}


/*
 * Associated Function declarations
 * */
antlrcpp::Any ScopeVisitor::visitInstanceAssociatedFn(eelParser::InstanceAssociatedFnContext* ctx) {
    return eelBaseVisitor::visitInstanceAssociatedFn(ctx);
}

antlrcpp::Any ScopeVisitor::visitPartialInstanceAssociatedFn(eelParser::PartialInstanceAssociatedFnContext* ctx) {
    return eelBaseVisitor::visitPartialInstanceAssociatedFn(ctx);
}

antlrcpp::Any ScopeVisitor::visitTypeAssociatedFn(eelParser::TypeAssociatedFnContext* ctx) {
    return eelBaseVisitor::visitTypeAssociatedFn(ctx);
}

antlrcpp::Any ScopeVisitor::visitPartialTypeAssociatedFn(eelParser::PartialTypeAssociatedFnContext* ctx) {
    return eelBaseVisitor::visitPartialTypeAssociatedFn(ctx);
}

antlrcpp::Any ScopeVisitor::visitInstanceAssocParamList(eelParser::InstanceAssocParamListContext* ctx) {
    return eelBaseVisitor::visitInstanceAssocParamList(ctx);
}

/*
 * Namespace declaration
 * */
antlrcpp::Any ScopeVisitor::visitNamespaceDecl(eelParser::NamespaceDeclContext* ctx) {
    return eelBaseVisitor::visitNamespaceDecl(ctx);
}


/*
 * Access Expressions
 * */
antlrcpp::Any ScopeVisitor::visitFqnExpr(eelParser::FqnExprContext* ctx) {
    return eelBaseVisitor::visitFqnExpr(ctx);
}

antlrcpp::Any ScopeVisitor::visitStructExpr(eelParser::StructExprContext* ctx) {
    return eelBaseVisitor::visitStructExpr(ctx);
}

antlrcpp::Any ScopeVisitor::visitArrayExpr(eelParser::ArrayExprContext* ctx) {
    return eelBaseVisitor::visitArrayExpr(ctx);
}

antlrcpp::Any ScopeVisitor::visitPointerExpr(eelParser::PointerExprContext* ctx) {
    return eelBaseVisitor::visitPointerExpr(ctx);
}

antlrcpp::Any ScopeVisitor::visitReferenceExpr(eelParser::ReferenceExprContext* ctx) {
    return eelBaseVisitor::visitReferenceExpr(ctx);
}

antlrcpp::Any ScopeVisitor::visitReadPinExpr(eelParser::ReadPinExprContext* ctx) {
    return eelBaseVisitor::visitReadPinExpr(ctx);
}


/*
 * Function Expressions
 * */
antlrcpp::Any ScopeVisitor::visitFnCallExpr(eelParser::FnCallExprContext* ctx) {
    return eelBaseVisitor::visitFnCallExpr(ctx);
}

antlrcpp::Any ScopeVisitor::visitInstanceAssociatedFnCallExpr(eelParser::InstanceAssociatedFnCallExprContext* ctx) {
    return eelBaseVisitor::visitInstanceAssociatedFnCallExpr(ctx);
}

antlrcpp::Any ScopeVisitor::visitExprList(eelParser::ExprListContext* ctx) {
    return eelBaseVisitor::visitExprList(ctx);
}


/*
 * Statements
 * */
antlrcpp::Any ScopeVisitor::visitStmtBlock(eelParser::StmtBlockContext* ctx) {
    auto _previous_scope = previous_scope;
    previous_scope = current_scope;
    current_scope = table->derive_scope();
    auto result = current_scope;
    visitChildren(ctx);
    current_scope = previous_scope;
    previous_scope = _previous_scope;
    return result;
}

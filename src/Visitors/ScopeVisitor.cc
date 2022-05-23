#include <Visitors/ScopeVisitor.hpp>
#include <Visitors/utility.hpp>
#include <fmt/core.h>

ScopeVisitor::ScopeVisitor(SymbolTable* _table) {
    table = _table;
    symbols::Primitive::register_primitives(table->root_scope);
    current_scope = table->root_scope;
    current_event = nullptr;
    active_sequence = nullptr;
}

antlrcpp::Any ScopeVisitor::visitProgram(eelParser::ProgramContext* ctx) {
    return visitChildren(ctx);
}


/*
 * Top level declarations
 * */
std::any ScopeVisitor::visitLoopDecl(eelParser::LoopDeclContext* ctx) {
    if (!current_scope->is_root())
        throw InternalError(InternalError::ScopeAnalysis, "LoopDecl node encountered in non-root scope context.");

    auto func = current_scope->declare_func(visitors::builtin_loop_name);
    func->value.function->body = ctx->stmtBlock();

    auto scope = current_scope;
    current_scope = func->value.function->scope;
    active_sequence = func->value.function->sequence = new Sequence(func->value.function->scope);
    visitChildren(ctx->stmtBlock());
    current_scope = scope;
    active_sequence = nullptr;
    return {};
}

std::any ScopeVisitor::visitSetupDecl(eelParser::SetupDeclContext* ctx) {
    if (!current_scope->is_root())
        throw InternalError(InternalError::ScopeAnalysis, "SetupDecl node encountered in non-root scope context.");

    auto func = current_scope->declare_func(visitors::builtin_setup_name);
    func->value.function->body = ctx->stmtBlock();

    auto scope = current_scope;
    current_scope = func->value.function->scope;
    active_sequence = func->value.function->sequence = new Sequence(func->value.function->scope);
    visitChildren(ctx->stmtBlock());
    current_scope = scope;
    active_sequence = nullptr;
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
    auto var = current_scope->declare_var(res.type, res.identifier);
    if (var != nullptr) {
        var->has_value = ctx->expr() != nullptr;
    }
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

static symbols::Function* create_predicate_function(SymbolTable& table, Scope& current_scope) {
    auto function = new symbols::Function;
    function->return_type = table.get_symbol(symbols::Primitive::boolean.id);
    function->scope = table.derive_scope(current_scope);
    function->sequence = new Sequence(function->scope);

    return function;
}

antlrcpp::Any ScopeVisitor::visitEventDecl(eelParser::EventDeclContext* ctx) {
    auto name = ctx->Identifier()->getText();
    auto event = current_scope->find(name);
    auto predicate = ctx->stmtBlock();

    if(event.is_nullptr()){
        // Create predicate less event.
        if(predicate == nullptr){
            current_scope->declare_event(ctx->Identifier()->getText());
        } else {
            // Create event with predicate
            auto function = create_predicate_function(*table, current_scope);

            auto event_symbol = current_scope->declare_event(name, function);
            function->type_id = fmt::format("{}_predicate", event_symbol->value.event->id);
            function->body = ctx->stmtBlock();
            if(!event_symbol.is_nullptr()) {
                this->current_event = event_symbol->value.event;
                this->active_sequence = function->sequence;
                visitChildren(ctx->stmtBlock());
                this->current_event = nullptr;
                this->active_sequence = nullptr;
            }
        }
    } else {
        // Name already used
        if(event->kind != Symbol_::Kind::Event){
            auto error = Error(Error::AlreadyDefined, ctx->Identifier()->getSymbol(), ctx, "");
            this->errors.push_back(error);
            return {};
        }

        auto& e = *(event->value.event);
        // if no predicate supplied or event has a predicate
        if(e.is_complete){
            auto error = Error(Error::DuplicateEvent, ctx->Identifier()->getSymbol(), ctx, "");
            this->errors.push_back(error);
            return {};
        } else {
            e.is_complete = true;
            if (predicate != nullptr) {
                e.has_predicate = true;
                e.predicate = create_predicate_function(*table, current_scope);
                e.predicate->type_id = fmt::format("{}_predicate", e.id);
                e.predicate->body = ctx->stmtBlock();

                this->current_event = &e;
                this->active_sequence = e.predicate->sequence;
                visitChildren(ctx->stmtBlock());
                this->current_event = nullptr;
                this->active_sequence = nullptr;
            }
        }
    }
    return {};
}

antlrcpp::Any ScopeVisitor::visitIntervalDecl(eelParser::IntervalDeclContext* ctx) {
    return eelBaseVisitor::visitIntervalDecl(ctx);
}

antlrcpp::Any ScopeVisitor::visitOnDecl(eelParser::OnDeclContext* ctx) {
    auto loc = visitors::get_source_location(ctx->start);
    auto& event = current_scope->declare_event_handle(ctx->fqn()->getText(), loc);
    auto& function = event.get_handle(loc);

    function.scope = table->derive_scope(current_scope);
    function.body = ctx->stmtBlock();
    this->active_sequence = function.sequence = new Sequence(function.scope);

    visitChildren(ctx->stmtBlock());
    this->active_sequence = nullptr;

    return {};
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
    if (this->active_sequence == nullptr)
        throw InternalError(InternalError::ScopeAnalysis, "Invalid visit to StmtBlock without active_sequence.");
    auto outer_scope = current_scope;
    auto inner_scope = table->derive_scope(outer_scope);

    current_scope = inner_scope;

    active_sequence->enter_block(inner_scope);
    visitChildren(ctx);
    active_sequence->leave_block();

    current_scope = outer_scope;
    return inner_scope;
}


std::any ScopeVisitor::visitAwaitStmt(eelParser::AwaitStmtContext* ctx) {
    if (this->active_sequence == nullptr)
        throw InternalError(InternalError::ScopeAnalysis, "Invalid visit to AwaitStmt without active_sequence.");

    active_sequence->yield(); // TODO mark event as awaited assuming ctx is an event.
    visitChildren(ctx);

    return {};
}
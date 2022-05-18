#include <Visitors/TypeVisitor.hpp>
#include <Visitors/utility.hpp>

/*
 * Helper functions
 */
std::string literal_str(TypeVisitor::Type::Kind value)  {
    std::string res;
    switch (value) {
        case TypeVisitor::Type::Kind::Undefined :
            res = "undefined";
            break;
        case TypeVisitor::Type::Kind::None:
            res = "None";
            break;
        case TypeVisitor::Type::Kind::Integer:
            res = "Integer";
            break;
        case TypeVisitor::Type::Kind::Float:
            res = "Float";
            break;
        case TypeVisitor::Type::Kind::Bool:
            res = "Bool";
            break;
        case TypeVisitor::Type::Kind::Char:
            res = "Char";
            break;
        case TypeVisitor::Type::Kind::String:
            res = "String";
            break;
        default:
            res = "ERROR";
    }
    return res;
}

std::string symbol_str(Symbol symbol) {
    switch (symbol->kind) {
        case Symbol_::Kind::None:
            return "none";
            break;
        case Symbol_::Kind::Variable:
            return symbol->value.variable->type->name;
            break;
        case Symbol_::Kind::Constant:
            return symbol->value.constant->type->name;
            break;
        case Symbol_::Kind::Function:
            if(symbol->value.function->has_return_type)
                return symbol->value.function->return_type->name;
            return "none";
            break;
        case Symbol_::Kind::Type:
            return symbol->value.type->type_source_name();
            break;
        default:
            return "none";
    }
    return symbol->value.type->type_source_name();
}

Error binary_expr(TypeVisitor::Type* left, TypeVisitor::Type* right, TypeVisitor::Type* expected, ParserRuleContext* ctx){
    if(expected->is_null()){
        if(!left->equals(right)){
            return Error(Error::TypeMisMatch, right->token, ctx, left->to_string());
        }
    } else {
        if(!left->equals(expected)){
            return Error(Error::TypeMisMatch, right->token, ctx, expected->to_string());
        }
    }
    left->token = ctx->start;
    return Error();
}

Error logical_expr(TypeVisitor::Type* type, ParserRuleContext* ctx){
    auto boolean = TypeVisitor::Type(TypeVisitor::Type::Kind::Bool, nullptr);
    if(type->is_null()){
        return Error(Error::TypeMisMatch, type->token, ctx, boolean.to_string());
    } else if(!type->equals(&boolean) ){
        return Error(Error::TypeMisMatch, type->token, ctx, boolean.to_string());
    }
    type->token = ctx->start;
    return Error();
}

Error binary_assign_expr(TypeVisitor::Type* var, TypeVisitor::Type* expr, ParserRuleContext* ctx){
    if (var->is_null() || var->is_literal) {
        return Error(Error::ExpectedVariable, var->token, ctx, "");
    } else if (var->symbol()->kind != Symbol_::Kind::Variable) {
        return Error(Error::ExpectedVariable, var->token, ctx, "");
    } else {
        if (!var->equals(expr)) {
            return Error(Error::TypeMisMatch, expr->token, ctx, var->to_string());
        }
    }
    var->token = ctx->start;
    return Error();
}

Error pin_stmt(TypeVisitor::Type* pin, TypeVisitor::Type* expr, ParserRuleContext* ctx, SymbolTable* table){
    auto digital = TypeVisitor::Type(table->get_symbol(symbols::Primitive::digital.id), nullptr);
    auto analog = TypeVisitor::Type(table->get_symbol(symbols::Primitive::analog.id), nullptr);
    auto u8 = TypeVisitor::Type(table->get_symbol(symbols::Primitive::u8.id), nullptr);
    if(pin->is_null()){
        return Error(Error::None, pin->token, ctx, "");
    } else if(!pin->equals(&analog) && !pin->equals(&digital)){
        return Error(Error::TypeMisMatch, pin->token, ctx, "digital or analog");
    } else {
        if(expr->is_null()){
            return Error(Error::TypeMisMatch, expr->token, ctx, "u8");
        } else if(!expr->equals(&u8)){
            return Error(Error::TypeMisMatch, expr->token, ctx, "u8");
        }
    }
    pin->token = ctx->start;
    return Error();
}

/*
 * Constructors
 */
TypeVisitor::Type::Type() {
    value = { .literal = Kind::None};
    token = nullptr;
    is_literal = true;
}

TypeVisitor::Type::Type(TypeVisitor::Type::Kind _value, Token* _token) {
    value = { .literal = _value};
    token = _token;
    is_literal = true;
}

TypeVisitor::Type::Type(Symbol _value, Token* _token) {
    if(_value.is_nullptr()){
        value = { .literal = Kind::None};
        is_literal = true;
    } else {
        value = { .symbol = _value};
        is_literal = false;
    }
    token = _token;
}

TypeVisitor::Type::Kind TypeVisitor::Type::literal() const {
    return value.literal;
}

Symbol TypeVisitor::Type::symbol() const {
    return value.symbol;
}

/// Returns the type as a std::string
std::string TypeVisitor::Type::to_string() const {
    if(this->is_literal){
        return literal_str(this->literal());
    }
    return symbol_str(this->symbol());
}

bool TypeVisitor::Type::is_null() const {
    return this->is_literal && this->literal() == Kind::None;
}

bool TypeVisitor::Type::equals(TypeVisitor::Type* other) {
    Type* kind = this->is_literal ? this : other;
    Type* symbol = this->is_literal ? other : this;
    if(kind->is_literal && !symbol->is_literal){
        if(symbol->symbol()->kind == Symbol_::Kind::Variable) {
            symbol->value.symbol = symbol->symbol()->value.variable->type;
        } else if(symbol->symbol()->kind == Symbol_::Kind::Constant) {
            symbol->value.symbol = symbol->symbol()->value.constant->type;
        }  else if(symbol->symbol()->kind == Symbol_::Kind::Event){
            return kind->literal() == Type::Kind::Bool;
        }
        switch (kind->literal()) {
            case Type::Kind::Integer:
                if(symbol->symbol()->id <= 7) return true;
                break;
            case Type::Kind::Float:
                if(symbol->symbol()->id == 8 || symbol->symbol()->id == 9) return true;
                break;
            case Type::Kind::Bool:
                if(symbol->symbol()->id == 10) return true;
            default: // TODO: missing primitives for strings and chars
                return false;
        }
    } if(this->is_literal && other->is_literal){
        return this->literal() == other->literal();
    } if(!this->is_literal && !other->is_literal){
        return this->to_string() == other->to_string();
    }
    return false;
}

TypeVisitor::TypeVisitor(SymbolTable* _table) {
    this->table = _table;
    this->current_scope = previous_scope = table->root_scope;
    this->current_function = nullptr;
    this->expected_type = Type();
    this->scope_index = 0;
}




antlrcpp::Any TypeVisitor::visitProgram(eelParser::ProgramContext* ctx) {
    return eelBaseVisitor::visitProgram(ctx);
}


/*
 * Top level declarations
 * */
antlrcpp::Any TypeVisitor::visitLoopDecl(eelParser::LoopDeclContext* ctx) {
    Symbol func = current_scope->find(visitors::builtin_loop_name);
    this->current_function = func->value.function;
    auto s = current_scope;
    this->current_scope = func->value.function->scope;
    visitChildren(ctx->stmtBlock());
    this->current_scope = s;
    this->current_function = nullptr;
    return {};
}

antlrcpp::Any TypeVisitor::visitSetupDecl(eelParser::SetupDeclContext* ctx) {
    Symbol func = current_scope->find(visitors::builtin_setup_name);
    this->current_function = func->value.function;
    auto s = current_scope;
    this->current_scope = func->value.function->scope;
    visitChildren(ctx->stmtBlock());
    this->current_scope = s;
    this->current_function = nullptr;
    return {};
}


/*
 * Variable declarations
 * */
antlrcpp::Any TypeVisitor::visitVariableDecl(eelParser::VariableDeclContext* ctx) {
    auto type = any_cast<Type>(visit(ctx->typedIdentifier()));

    if(type.is_literal && type.literal() == Type::Undefined){
        auto error = Error(Error::UndefinedType,type.token,ctx,"");
        this->errors.push_back(error);
    } else {
        if(nullptr != ctx->expr()) {
            this->expected_type = type; // Set expected type to be used inside expressions
            auto expr = any_cast<Type>(visit(ctx->expr()));
            this->expected_type = Type(); // Reset value

            if(expr.is_literal && expr.literal() == Type::Undefined){
                auto error = Error(Error::UndefinedType,expr.token,ctx,"");
                this->errors.push_back(error);
            } else if(!type.equals(&expr)){
                auto error = Error(Error::TypeMisMatch,expr.token,ctx,type.to_string());
                this->errors.push_back(error);
            }
        }
    }
    type.token = ctx->start;
    return type;
}

antlrcpp::Any TypeVisitor::visitStaticDecl(eelParser::StaticDeclContext* ctx) {
    auto type = any_cast<Type>(visit(ctx->typedIdentifier()));

    if(type.is_literal && type.literal() == Type::Undefined){
        auto error = Error(Error::UndefinedType,type.token,ctx,"");
        this->errors.push_back(error);
    } else {
        this->expected_type = type; // Set expected type to be used inside expressions
        auto expr = any_cast<Type>(visit(ctx->expr()));
        this->expected_type = Type(); // Reset value

        if(expr.is_literal && expr.literal() == Type::Undefined){
            auto error = Error(Error::UndefinedType,expr.token,ctx,"");
            this->errors.push_back(error);
        } else if(!type.equals(&expr)){
            auto error = Error(Error::TypeMisMatch,expr.token,ctx,type.to_string());
            this->errors.push_back(error);
        }
        type.token = ctx->start;
    }
    return type;
}

antlrcpp::Any TypeVisitor::visitConstDecl(eelParser::ConstDeclContext* ctx) {
    auto type = any_cast<Type>(visit(ctx->typedIdentifier()));

    if(type.is_literal && type.literal() == Type::Undefined){
        auto error = Error(Error::UndefinedType,type.token, ctx,"");
        this->errors.push_back(error);
    } else {
        this->expected_type = type; // Set expected type to be used inside expressions
        auto expr = any_cast<Type>(visit(ctx->expr()));
        this->expected_type = Type(); // Reset value

        if(expr.is_literal && expr.literal() == Type::Undefined){
            auto error = Error(Error::UndefinedType,expr.token, ctx,"");
            this->errors.push_back(error);
        } else if(!type.equals(&expr)){
            auto error = Error(Error::TypeMisMatch,expr.token,ctx,type.to_string());
            this->errors.push_back(error);
        }
        type.token = ctx->start;
    }
    return type;
}

antlrcpp::Any TypeVisitor::visitPinDecl(eelParser::PinDeclContext* ctx) {
    auto expr = std::any_cast<Type>(visit(ctx->expr()));
    auto u8 = Type(table->root_scope->find("u8"), nullptr);

    if(!u8.equals(&expr)){
        auto error = Error(Error::TypeMisMatch,expr.token,ctx,u8.to_string());
        this->errors.push_back(error);
    }
    return visitChildren(ctx);
}

antlrcpp::Any TypeVisitor::visitArrayDecl(eelParser::ArrayDeclContext* ctx) {
    return eelBaseVisitor::visitArrayDecl(ctx);
}

antlrcpp::Any TypeVisitor::visitReferenceDecl(eelParser::ReferenceDeclContext* ctx) {
    return eelBaseVisitor::visitReferenceDecl(ctx);
}

antlrcpp::Any TypeVisitor::visitPointerDecl(eelParser::PointerDeclContext* ctx) {
    return eelBaseVisitor::visitPointerDecl(ctx);
}

antlrcpp::Any TypeVisitor::visitTypedIdentifier(eelParser::TypedIdentifierContext* ctx) {
    auto type_name = ctx->type()->getText();
    auto type = current_scope->find(type_name);
    if (type.is_nullptr()){
        return Type(Type::Kind::Undefined, ctx->getStart());
    }
    if(type->kind != Symbol_::Kind::Type){
        return Type(Type::Kind::Undefined, ctx->getStart());
    }
    return Type(type, ctx->start);
}


/*
 *  Event Declarations
 * */

antlrcpp::Any TypeVisitor::visitEventDecl(eelParser::EventDeclContext* ctx) {
    auto event = current_scope->find(ctx->Identifier()->getText());
    if(!event.is_nullptr()){
        this->current_function = event->value.event->predicate;
        visitChildren(ctx);
        this->current_function = nullptr;
    }
    return Type();
}
antlrcpp::Any TypeVisitor::visitOnDecl(eelParser::OnDeclContext* ctx) {
    auto fqn = current_scope->find(ctx->fqn()->getText());

    if(fqn.is_nullptr()){
        auto error = Error(Error::Kind::TypeMisMatch, ctx->fqn()->start, ctx, "Event");
        this->errors.push_back(error);
    }
    auto event = Type(fqn, ctx->fqn()->start);

    if(event.is_literal) {
        auto error = Error(Error::Kind::TypeMisMatch, event.token, ctx, "Event");
        this->errors.push_back(error);
    } else if (event.symbol()->kind != Symbol_::Kind::Event){
        auto error = Error(Error::Kind::TypeMisMatch, event.token, ctx, "Event");
        this->errors.push_back(error);
    }
    return visitChildren(ctx);
}


/*
 * Literal Expressions
 * */
antlrcpp::Any TypeVisitor::visitIntegerLiteral(eelParser::IntegerLiteralContext* ctx) {
    return Type(Type::Kind::Integer, ctx->getStart());
}

antlrcpp::Any TypeVisitor::visitFloatLiteral(eelParser::FloatLiteralContext* ctx) {
    return Type(Type::Kind::Float, ctx->getStart());
}

antlrcpp::Any TypeVisitor::visitBoolLiteral(eelParser::BoolLiteralContext* ctx) {
    return Type(Type::Kind::Bool, ctx->getStart());
}

antlrcpp::Any TypeVisitor::visitCharLiteral(eelParser::CharLiteralContext* ctx) {
    return Type(Type::Kind::Char, ctx->getStart());
}

antlrcpp::Any TypeVisitor::visitStringLiteral(eelParser::StringLiteralContext* ctx) {
    return Type(Type::Kind::String, ctx->getStart());
}


/*
 * Function Expressions
 * */
antlrcpp::Any TypeVisitor::visitFnCallExpr(eelParser::FnCallExprContext* ctx) {
    return eelBaseVisitor::visitFnCallExpr(ctx);
}

antlrcpp::Any TypeVisitor::visitInstanceAssociatedFnCallExpr(eelParser::InstanceAssociatedFnCallExprContext* ctx) {
    return eelBaseVisitor::visitInstanceAssociatedFnCallExpr(ctx);
}

antlrcpp::Any TypeVisitor::visitExprList(eelParser::ExprListContext* ctx) {
    return eelBaseVisitor::visitExprList(ctx);
}


/*
 * Init Expressions
 * */
antlrcpp::Any TypeVisitor::visitStructExpr(eelParser::StructExprContext* ctx) {
    return eelBaseVisitor::visitStructExpr(ctx);
}

antlrcpp::Any TypeVisitor::visitArrayExpr(eelParser::ArrayExprContext* ctx) {
    return eelBaseVisitor::visitArrayExpr(ctx);
}

antlrcpp::Any TypeVisitor::visitPointerExpr(eelParser::PointerExprContext* ctx) {
    return eelBaseVisitor::visitPointerExpr(ctx);
}

antlrcpp::Any TypeVisitor::visitReferenceExpr(eelParser::ReferenceExprContext* ctx) {
    return eelBaseVisitor::visitReferenceExpr(ctx);
}

antlrcpp::Any TypeVisitor::visitFqnExpr(eelParser::FqnExprContext* ctx) {
    auto text = ctx->getText();
    auto token = current_scope->find(ctx->getText());
    if(token.is_nullptr())
        return Type(Type::Undefined, ctx->start);
    return Type(token, ctx->start);
}


/*
 * Math Expressions
 * */
antlrcpp::Any TypeVisitor::visitAdditiveExpr(eelParser::AdditiveExprContext* ctx) {
    auto left = any_cast<Type>(visit(ctx->left));
    auto right = any_cast<Type>(visit(ctx->right));
    auto error = binary_expr(&left, &right, &this->expected_type, ctx);
    if(error.kind != Error::None) {
        this->errors.push_back(error);
    }
    return left;
}

antlrcpp::Any TypeVisitor::visitScalingExpr(eelParser::ScalingExprContext* ctx) {
    auto left = any_cast<Type>(visit(ctx->left));
    auto right = any_cast<Type>(visit(ctx->right));
    auto error = binary_expr(&left, &right, &this->expected_type, ctx);
    if(error.kind != Error::None) {
        this->errors.push_back(error);
    }
    return left;
}

antlrcpp::Any TypeVisitor::visitShiftingExpr(eelParser::ShiftingExprContext* ctx) {
    auto left = any_cast<Type>(visit(ctx->left));
    auto right = any_cast<Type>(visit(ctx->right));
    auto error = binary_expr(&left, &right, &this->expected_type, ctx);
    if(error.kind != Error::None) {
        this->errors.push_back(error);
    }
    return left;
}

antlrcpp::Any TypeVisitor::visitComparisonExpr(eelParser::ComparisonExprContext* ctx) {
    auto left = any_cast<Type>(visit(ctx->left));
    auto right = any_cast<Type>(visit(ctx->right));
    auto error = binary_expr(&left, &right, &this->expected_type, ctx);
    if(error.kind != Error::None) {
        this->errors.push_back(error);
    }
    return Type(Type::Bool, left.token);
}

antlrcpp::Any TypeVisitor::visitAndExpr(eelParser::AndExprContext* ctx) {
    auto left = any_cast<Type>(visit(ctx->left));
    auto right = any_cast<Type>(visit(ctx->right));
    auto error = binary_expr(&left, &right, &this->expected_type, ctx);
    if(error.kind != Error::None) {
        this->errors.push_back(error);
    }
    return left;
}

antlrcpp::Any TypeVisitor::visitXorExpr(eelParser::XorExprContext* ctx) {
    auto left = any_cast<Type>(visit(ctx->left));
    auto right = any_cast<Type>(visit(ctx->right));
    auto error = binary_expr(&left, &right, &this->expected_type, ctx);
    if(error.kind != Error::None) {
        this->errors.push_back(error);
    }
    return left;
}

antlrcpp::Any TypeVisitor::visitOrExpr(eelParser::OrExprContext* ctx) {
    auto left = any_cast<Type>(visit(ctx->left));
    auto right = any_cast<Type>(visit(ctx->right));
    auto error = binary_expr(&left, &right, &this->expected_type, ctx);
    if(error.kind != Error::None) {
        this->errors.push_back(error);
    }
    return left;
}


/*
 * Logical Expressions
 */
antlrcpp::Any TypeVisitor::visitLAndExpr(eelParser::LAndExprContext* ctx) {
    auto left = any_cast<Type>(visit(ctx->left));
    auto right = any_cast<Type>(visit(ctx->right));
    auto left_error = logical_expr(&left, ctx);
    auto right_error = logical_expr(&right, ctx);
    if(left_error.kind != Error::None) {
        this->errors.push_back(left_error);
    }
    if(right_error.kind != Error::None) {
        this->errors.push_back(right_error);
    }
    return left;
}

antlrcpp::Any TypeVisitor::visitLOrExpr(eelParser::LOrExprContext* ctx) {
    auto left = any_cast<Type>(visit(ctx->left));
    auto right = any_cast<Type>(visit(ctx->right));
    auto left_error = logical_expr(&left, ctx);
    auto right_error = logical_expr(&right, ctx);
    if(left_error.kind != Error::None) {
        this->errors.push_back(left_error);
    }
    if(right_error.kind != Error::None) {
        this->errors.push_back(right_error);
    }
    return left;
}


/*
 * Assigment Expressions
 */
antlrcpp::Any TypeVisitor::visitAssignExpr(eelParser::AssignExprContext* ctx) {
    auto var = any_cast<Type>(visit(ctx->var));
    auto expr = any_cast<Type>(visit(ctx->right));
    auto error = binary_assign_expr(&var, &expr, ctx);
    if(error.kind != Error::None){
        this->errors.push_back(error);
    }
    return var;
}

antlrcpp::Any TypeVisitor::visitAdditiveAssignExpr(eelParser::AdditiveAssignExprContext* ctx) {
    auto var = any_cast<Type>(visit(ctx->var));
    auto expr = any_cast<Type>(visit(ctx->right));
    auto error = binary_assign_expr(&var, &expr, ctx);
    if(error.kind != Error::None){
        this->errors.push_back(error);
    }
    return var;
}

antlrcpp::Any TypeVisitor::visitScalingAssignExpr(eelParser::ScalingAssignExprContext* ctx) {
    auto var = any_cast<Type>(visit(ctx->var));
    auto expr = any_cast<Type>(visit(ctx->right));
    auto error = binary_assign_expr(&var, &expr, ctx);
    if(error.kind != Error::None){
        this->errors.push_back(error);
    }
    return var;
}

antlrcpp::Any TypeVisitor::visitShiftingAssignExpr(eelParser::ShiftingAssignExprContext* ctx) {
    auto var = any_cast<Type>(visit(ctx->var));
    auto expr = any_cast<Type>(visit(ctx->right));
    auto error = binary_assign_expr(&var, &expr, ctx);
    if(error.kind != Error::None){
        this->errors.push_back(error);
    }
    return var;
}

antlrcpp::Any TypeVisitor::visitOrAssignExpr(eelParser::OrAssignExprContext* ctx) {
    auto var = any_cast<Type>(visit(ctx->var));
    auto expr = any_cast<Type>(visit(ctx->right));
    auto error = binary_assign_expr(&var, &expr, ctx);
    if(error.kind != Error::None){
        this->errors.push_back(error);
    }
    return var;
}

antlrcpp::Any TypeVisitor::visitAndAssignExpr(eelParser::AndAssignExprContext* ctx) {
    auto var = any_cast<Type>(visit(ctx->var));
    auto expr = any_cast<Type>(visit(ctx->right));
    auto error = binary_assign_expr(&var, &expr, ctx);
    if(error.kind != Error::None){
        this->errors.push_back(error);
    }
    return var;
}

antlrcpp::Any TypeVisitor::visitXorAssignExpr(eelParser::XorAssignExprContext* ctx) {
    auto var = any_cast<Type>(visit(ctx->var));
    auto expr = any_cast<Type>(visit(ctx->right));
    auto error = binary_assign_expr(&var, &expr, ctx);
    if(error.kind != Error::None){
        this->errors.push_back(error);
    }
    return var;
}


/*
 * Other Expressions
 */
antlrcpp::Any TypeVisitor::visitReadPinExpr(eelParser::ReadPinExprContext* ctx) {
    auto digital = Type(table->root_scope->find("digital"), nullptr);
    auto analog = Type(table->root_scope->find("analog"), nullptr);
    auto pin = std::any_cast<Type>(visit(ctx->fqn()));
    if(pin.is_null()){
        auto error = Error(Error::None, pin.token, ctx, "");
        this->errors.push_back(error);
    } else if (!pin.equals(&analog) && !pin.equals(&digital)) {
        auto error = Error(Error::TypeMisMatch, pin.token, ctx, "digital or analog");
        this->errors.push_back(error);
    }
    return Type(table->root_scope->find("u8"), pin.token);
}


/*
 * Statements
 */
antlrcpp::Any TypeVisitor::visitSetPinValueStmt(eelParser::SetPinValueStmtContext* ctx) {
    auto pin = Type(current_scope->find(ctx->fqn()->getText()), ctx->fqn()->start);
    auto expr = std::any_cast<Type>(visit(ctx->expr()));
    auto error = pin_stmt(&pin, &expr, ctx, this->table);
    if(error.kind != Error::None){
        this->errors.push_back(error);
    }
    return pin;
}

antlrcpp::Any TypeVisitor::visitSetPinModeStmt(eelParser::SetPinModeStmtContext* ctx) {
    auto pin = Type(current_scope->find(ctx->fqn()->getText()), ctx->fqn()->start);
    auto expr = std::any_cast<Type>(visit(ctx->expr()));
    auto error = pin_stmt(&pin, &expr, ctx, this->table);
    if(error.kind != Error::None){
        this->errors.push_back(error);
    }
    return pin;
}

antlrcpp::Any TypeVisitor::visitSetPinNumberStmt(eelParser::SetPinNumberStmtContext* ctx) {
    auto pin = Type(current_scope->find(ctx->fqn()->getText()), ctx->fqn()->start);
    auto expr = std::any_cast<Type>(visit(ctx->expr()));
    auto error = pin_stmt(&pin, &expr, ctx, this->table);
    if(error.kind != Error::None){
        this->errors.push_back(error);
    }
    return pin;
}

antlrcpp::Any TypeVisitor::visitStmtBlock(eelParser::StmtBlockContext* ctx) {
    auto _previous_scope = previous_scope;
    previous_scope = current_scope;
    scope_index++;
    current_scope = table->get_scope(scope_index);
    visitChildren(ctx);
    current_scope = previous_scope;
    previous_scope = _previous_scope;
    return {};
}

antlrcpp::Any TypeVisitor::visitAwaitStmt(eelParser::AwaitStmtContext* ctx) {
    auto expr = any_cast<Type>(visit(ctx->expr()));
    auto boolean = Type(Type::Kind::Bool, nullptr);
    if(!expr.equals(&boolean)){
        auto error = Error(Error::Kind::TypeMisMatch, expr.token, ctx, boolean.to_string());
        this->errors.push_back(error);
    }
    return expr;
}

antlrcpp::Any TypeVisitor::visitReturnStmt(eelParser::ReturnStmtContext* ctx) {
    if(nullptr == ctx->expr()){
        if(this->current_function->has_return_type){
            auto return_type = Type(this->current_function->return_type, nullptr);
            auto error = Error(Error::Kind::InvalidReturnType, ctx->start, ctx, return_type.to_string());
            this->errors.push_back(error);
        }
    } else {
        auto expr = any_cast<Type>(visit(ctx->expr()));
        auto undefined = Type(Type::Kind::Undefined, nullptr);
        if(!this->current_function->has_return_type && !expr.equals(&undefined)){
            auto error = Error(Error::Kind::InvalidReturnType, expr.token, ctx, "return;");
            this->errors.push_back(error);
        } else {
            auto return_type = Type(this->current_function->return_type, nullptr);
            if(!expr.equals(&return_type)){
                auto error = Error(Error::Kind::InvalidReturnType, expr.token, ctx, return_type.to_string());
                this->errors.push_back(error);
            }
        }
        return expr;
    }
    return Type(Type::Kind::None, nullptr);
}

/*
 * Other
 */
antlrcpp::Any TypeVisitor::visitConditionBlock(eelParser::ConditionBlockContext* ctx) {
    Type expr = any_cast<Type>(visit(ctx->expr()));
    auto boolean = Type(Type::Kind::Bool, nullptr);

    if(expr.is_null()){
        auto error = Error(Error::None, expr.token, ctx, "");
        this->errors.push_back(error);
    } else if (!expr.equals(&boolean)){
        auto error = Error(Error::TypeMisMatch, expr.token, ctx, boolean.to_string());
        this->errors.push_back(error);
    }
    return expr;
}

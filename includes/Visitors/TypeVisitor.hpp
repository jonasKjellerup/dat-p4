#pragma once
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#pragma clang diagnostic ignored "-Wunused-variable"

#include <utility>

#include "eelBaseVisitor.h"
#include "antlr4-runtime.h"
#include "symbol_table.hpp"
#include "symbols/type.hpp"
#include "error.hpp"

using namespace eel;
using namespace antlr4;

class TypeVisitor : eelBaseVisitor {
private:
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

        explicit Type(){
            value = { .literal = Kind::None};
            token = nullptr;
            is_literal = true;
        }

        explicit Type(Kind _value, Token* _token){
            value = { .literal = _value};
            token = _token;
            is_literal = true;
        }

        explicit Type(Symbol _value, Token* _token){
            if(_value.is_nullptr()){
                value = { .literal = Kind::None};
                is_literal = true;
            } else {
                value = { .symbol = _value};
                is_literal = false;
            }
            token = _token;
        }

        [[nodiscard]] Kind literal() const {
            return value.literal;
        }

        [[nodiscard]] std::string literal_str() const {
            std::string res;
            switch (value.literal) {
                case Undefined:
                    res = "undefined";
                    break;
                case None:
                    res = "None";
                    break;
                case Integer:
                    res = "Integer";
                    break;
                case Float:
                    res = "Float";
                    break;
                case Bool:
                    res = "Bool";
                    break;
                case Char:
                    res = "Char";
                    break;
                case String:
                    res = "String";
                    break;
                default:
                    res = "ERROR";
            }
            return res;
        }

        std::string get_type(){
            if(is_literal){
                return literal_str();
            }
            return symbol_str();
        }

        [[nodiscard]] Symbol symbol() const {
            return value.symbol;
        }
        [[nodiscard]] bool is_null() const {
            return this->is_literal && this->literal() == Kind::None;
        }

        std::string symbol_str() {
            return value.symbol->value.type->type_source_name();
        }

        Value value {};
        Token* token;
        bool is_literal;
    };

    static bool type_equals(Type a, Type b){
        Type kind = a.is_literal ? a : b;
        Type symbol = a.is_literal ? b : a;
        if(kind.is_literal && !symbol.is_literal){
            if(symbol.symbol()->kind != Symbol_::Kind::Type) {
                symbol.value.symbol = symbol.symbol()->value.variable->type;
            }
            switch (kind.literal()) {
                case Type::Kind::Integer:
                    if(symbol.symbol()->id <= 7) return true;
                    break;
                case Type::Kind::Float:
                    if(symbol.symbol()->id == 8 || symbol.symbol()->id == 9) return true;
                    break;
                default: // TODO: missing primitives for strings and chars
                    return false;
            }
        } if(a.is_literal && b.is_literal){
            return a.literal() == b.literal();
        } if(!a.is_literal && !b.is_literal){
            return a.get_type() == b.get_type();
        }
        return false;
    }

    static std::string get_context_source(ParserRuleContext* ctx) {
        auto interval = misc::Interval(
                ctx->getStart()->getStartIndex(),
                ctx->getStop()->getStopIndex()
        );
        return ctx->start->getInputStream()->getText(interval);
    }

    static Error::Pos get_source_location(Token* token){
        return {
                token->getLine(),
                token->getCharPositionInLine()
        };
    }

    void new_error(Error::Kind kind, Token* token, ParserRuleContext* ctx, size_t offset, std::string expected){
        auto location = get_source_location(token);
        auto source = get_context_source(ctx);
        auto error = Error(
                kind,
                source,
                std::move(expected),
                location,
                offset
        );
        error.print(); // TODO: Remove
        this->errors.push_back(error);
    }

    Type binary_expr_check(tree::ParseTree* ctx_right, tree::ParseTree* ctx_left, ParserRuleContext* ctx){
        auto left = any_cast<Type>(visit(ctx_left));
        auto right = any_cast<Type>(visit(ctx_right));
        right.token->getLine();
        right.token->getCharPositionInLine();
        auto offset = right.token->getCharPositionInLine();
        if(this->expected_type.is_null()){
            if(!type_equals(left, right)){
                new_error(Error::TypeMisMatch, left.token, ctx, offset, right.get_type());
            }
        } else {
            if(!type_equals(left, this->expected_type)){
                new_error(Error::TypeMisMatch, left.token, ctx, offset, this->expected_type.get_type());
            }
        }
        return right;
    }

public:

    SymbolTable* table;
    Scope current_scope;
    Scope previous_scope;
    size_t scope_index;
    Type expected_type;
    std::vector<Error> errors;
    explicit TypeVisitor(SymbolTable* _table) {
        table = _table;
        current_scope = previous_scope = table->root_scope;
        expected_type = Type();
        scope_index = 0;
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
        auto type = any_cast<Type>(visit(ctx->typedIdentifier()));
        auto offset = ctx->start->getCharPositionInLine();
        if(type.is_literal && type.literal() == Type::Undefined){
            new_error(Error::UndefinedType,
                      type.token,
                      ctx,
                      offset,
                      "");
        } else {
            this->expected_type = type; // Set expected type to be used inside expressions
            auto expr = any_cast<Type>(visit(ctx->expr()));
            this->expected_type = Type(); // Reset value

            if(expr.is_literal && expr.literal() == Type::Undefined){
                new_error(Error::UndefinedType,
                          expr.token,
                          ctx,
                          offset,
                          "");
            } else if(!type_equals(type, expr)){
                new_error(Error::TypeMisMatch,
                          expr.token,
                          ctx,
                          offset,
                          type.get_type());
            }
            type.token = ctx->start;
        }
        return type;
    }

    antlrcpp::Any visitConstDecl (eelParser::ConstDeclContext* ctx) override {
        auto type = any_cast<Type>(visit(ctx->typedIdentifier()));
        auto offset = ctx->start->getCharPositionInLine();
        if(type.is_literal && type.literal() == Type::Undefined){
            new_error(Error::UndefinedType,
                      type.token,
                      ctx,
                      offset,
                      "");
        } else {
            this->expected_type = type; // Set expected type to be used inside expressions
            auto expr = any_cast<Type>(visit(ctx->expr()));
            this->expected_type = Type(); // Reset value

            if(expr.is_literal && expr.literal() == Type::Undefined){
                new_error(Error::UndefinedType,
                          expr.token,
                          ctx,
                          offset,
                          "");
            } else if(!type_equals(type, expr)){
                new_error(Error::TypeMisMatch,
                          expr.token,
                          ctx,
                          offset,
                          type.get_type());
            }
            type.token = ctx->start;
        }
        return type;
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
     * Literal Expressions
     * */
    antlrcpp::Any visitIntegerLiteral (eelParser::IntegerLiteralContext* ctx) override {
        return Type(Type::Kind::Integer, ctx->getStart());
    }
    antlrcpp::Any visitFloatLiteral (eelParser::FloatLiteralContext* ctx) override {
        return Type(Type::Kind::Float, ctx->getStart());
    }
    antlrcpp::Any visitBoolLiteral (eelParser::BoolLiteralContext* ctx) override {
        return Type(Type::Kind::Bool, ctx->getStart());
    }
    antlrcpp::Any visitCharLiteral (eelParser::CharLiteralContext* ctx) override {
        return Type(Type::Kind::Char, ctx->getStart());
    }
    antlrcpp::Any visitStringLiteral (eelParser::StringLiteralContext* ctx) override {
        return Type(Type::Kind::String, ctx->getStart());
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
        auto token = current_scope->find(ctx->getText());
        if(token.is_nullptr())
            return Type(Type::Undefined, ctx->start);
        else if(token->kind != Symbol_::Kind::Variable && token->kind != Symbol_::Kind::Constant)
            return Type(Type::NotAType, ctx->start);
        return Type(token, ctx->start);
    }

    /*
     * Math Expressions
     * */
    antlrcpp::Any visitScalingExpr (eelParser::ScalingExprContext* ctx) override {
        auto left = binary_expr_check(ctx->right, ctx->left, ctx);
        left.token = ctx->start;
        return left;
    }
    antlrcpp::Any visitAdditiveExpr (eelParser::AdditiveExprContext* ctx) override {
        auto left = binary_expr_check(ctx->right, ctx->left, ctx);
        left.token = ctx->start;
        return left;
    }
    antlrcpp::Any visitShiftingExpr (eelParser::ShiftingExprContext* ctx) override {
        auto left = binary_expr_check(ctx->right, ctx->left, ctx);
        left.token = ctx->start;
        return left;
    }
    antlrcpp::Any visitComparisonExpr (eelParser::ComparisonExprContext* ctx) override {
        auto left = binary_expr_check(ctx->right, ctx->left, ctx);
        left.token = ctx->start;
        return left;
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
        auto variable = any_cast<Type>(visit(ctx->right));
        auto offset = ctx->start->getCharPositionInLine();

        if(variable.is_null() || variable.is_literal) {
            new_error(Error::ExpectedVariable, variable.token, ctx, offset, "");
        } else if(variable.symbol()->kind != Symbol_::Kind::Variable){
            new_error(Error::ExpectedVariable, variable.token, ctx, offset, "");
        } else {
            auto expr = any_cast<Type>(visit(ctx->left));
            if(!type_equals(variable, expr)){
                new_error(Error::ExpectedVariable, expr.token, ctx, offset, "");
            }
        }
        return variable;
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
    antlrcpp::Any visitStmtBlock (eelParser::StmtBlockContext* ctx) override {
        auto _previous_scope = previous_scope;
        previous_scope = current_scope;
        current_scope = table->get_scope(++scope_index);
        visitChildren(ctx);
        scope_index--;
        current_scope = previous_scope;
        previous_scope = _previous_scope;
        return {};
    }
};
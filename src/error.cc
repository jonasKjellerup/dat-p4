#include "error.hpp"
#include <utility>
#include <iostream>

Error::Pos Error::get_pos() const {
    return this->location;
}



void print(Error* error, const std::string& message){
    std::cout << error->source << std::endl;
    std::cout << std::string(error->location.c - error->offset, ' ') << "^~ ";
    std::cout <<  message << error->expected;
    std::cout << " on Line: " << error->location.l << " Column: " << error->location.c;
    std::cout << std::endl;
}

void Error::print() {
    switch (this->kind) {
        case Error::TypeMisMatch:
            ::print(this,"Type mismatch expected: ");
            break;
        case Error::InvalidReturnType:
            ::print(this,"Invalid return type expected: ");
            break;
        case Error::UndefinedType:
            ::print(this,"Undefined type");
            break;
        case Error::ExpectedVariable:
            ::print(this, "Expected Variable");
            break;
        default:
            ::print(this,"Unknown error");
    }
}

Error::Error() {
    this->source = "";
    this->kind = Error::Kind::None;
    this->offset = 0;
}

Error::Error(Error::Kind _kind) {
    this->source = "";
    this->kind = _kind;
    this->offset = 0;
}

Error::Error(Error::Kind _kind, std::string _source, std::string _expected, Pos _location, size_t _offset) {
    this->offset = _offset;
    this->location = _location;
    this->source = std::move(_source);
    this->expected = std::move(_expected);
    this->kind = _kind;
}

static Error::Pos get_source_location(Token* token){
    return {
            token->getLine(),
            token->getCharPositionInLine()
    };
}

static std::string get_context_source(ParserRuleContext* ctx) {
    auto interval = misc::Interval(
            ctx->getStart()->getStartIndex(),
            ctx->getStop()->getStopIndex()
    );
    return ctx->start->getInputStream()->getText(interval);
}

Error::Error(Error::Kind kind, Token* token, ParserRuleContext* ctx, const std::string& expected){
    this->location = get_source_location(token);
    this->source = get_context_source(ctx);
    this->offset = ctx->start->getCharPositionInLine();
    this->kind = kind;
    this->expected = expected;
}


void Error::set_pos(Error::Pos _location) {
    this->location = _location;
}

void Error::set_expected(std::string _expected) {
    this->expected = std::move(_expected);
}

Error::Pos::Pos(std::size_t _line, std::size_t _char) {
    this->l = _line;
    this->c = _char;
}

Error::Pos::Pos() {
    this->l = 0;
    this->c = 0;
}

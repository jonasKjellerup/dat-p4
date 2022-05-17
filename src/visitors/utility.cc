#include <Visitors/utility.hpp>

using namespace eel;
using namespace eel::visitors;

Symbol resolve_fqn(Scope scope, eelParser::FqnContext* fqn) {
    // TODO: resolve namespace/object access properly
    //       this currently assumes direct access.
    return scope->find(fqn->getText());
}

std::string get_source_text(antlr4::ParserRuleContext* ctx) {
    auto interval = antlr4::misc::Interval(
            ctx->getStart()->getStartIndex(),
            ctx->getStop()->getStopIndex()
    );
    return ctx->start->getInputStream()->getText(interval);
}

Error::Pos get_source_location(antlr4::Token* token) {
    return {
        token->getLine(),
        token->getCharPositionInLine(),
    };
}
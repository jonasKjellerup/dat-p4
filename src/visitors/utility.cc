#include <Visitors/utility.hpp>

using namespace eel;
using namespace eel::visitors;

const char* visitors::builtin_setup_name = "__eel_setup";
const char* visitors::builtin_loop_name = "__eel_loop";

Symbol visitors::resolve_fqn(Scope scope, eelParser::FqnContext* fqn) {
    // TODO: resolve namespace/object access properly
    //       this currently assumes direct access.
    return scope->find(fqn->getText());
}

std::string visitors::get_source_text(antlr4::ParserRuleContext* ctx) {
    auto interval = antlr4::misc::Interval(
            ctx->getStart()->getStartIndex(),
            ctx->getStop()->getStopIndex()
    );
    return ctx->start->getInputStream()->getText(interval);
}

Error::Pos visitors::get_source_location(antlr4::Token* token) {
    return {
        token->getLine(),
        token->getCharPositionInLine(),
    };
}
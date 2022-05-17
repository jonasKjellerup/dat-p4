#pragma once

#include <eelBaseVisitor.h>
#include <antlr4-runtime.h>

#include <symbol_table.hpp>
#include <error.hpp>

namespace eel::visitors {

    using SourcePos = Error::Pos;

    /// \brief Resolve a fully-qualified name to a symbol
    Symbol resolve_fqn(Scope scope, eelParser::FqnContext* fqn);

    /// \brief Get the source text associated with a given context
    std::string get_source_text(antlr4::ParserRuleContext* ctx);

    /// \brief Get the source text location of a given token
    SourcePos get_source_location(antlr4::Token* token);

    extern const char* builtin_setup_name;
    extern const char* builtin_loop_name;

}
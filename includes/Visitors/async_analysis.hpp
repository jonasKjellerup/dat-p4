#pragma once

#include <eelBaseVisitor.h>
#include <antlr4-runtime.h>

#include <symbol_table.hpp>
#include <symbols/function.hpp>

namespace eel::visitors {
    using namespace eel;

    /// \brief Analysis of asynchronous code structures within the CST
    struct AsyncAnalysisVisitor : eelBaseVisitor {

        union Context {
            using Symbol_t = Symbol;
            enum struct Kind {
                None,
                Symbol,
                EventHandle,
                Block,
            };

            struct {} none {};
            Symbol_t symbol;
            symbols::Function* event_handle;
            symbols::Sequence* block;
        };

        SymbolTable& table;
        Context::Kind context_kind = Context::Kind::None;
        Context context {};

        explicit AsyncAnalysisVisitor(SymbolTable& table);

        std::any visitStmtBlock(eelParser::StmtBlockContext *ctx) override;
        std::any visitEventDecl(eelParser::EventDeclContext *ctx) override;
        std::any visitOnDecl(eelParser::OnDeclContext *ctx) override;
        std::any visitAwaitStmt(eelParser::AwaitStmtContext *ctx) override;


    };
}
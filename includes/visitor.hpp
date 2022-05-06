#pragma once

#include "eelParser.h"

namespace eel {
    struct Visitor {
        virtual void visit(eelParser::ExprContext*);
        virtual void visit(eelParser::VariableDeclContext*);
    };
}
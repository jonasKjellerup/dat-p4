#pragma once
#include <eelBaseVisitor.h>
#include <antlr4-runtime.h>
using namespace eel;

class ScopeVisitor : eelBaseVisitor {
public:
    /*
     * TODO: Might need to add some fields for storing additional context
     * */

    /*
     * This rule is here to act as a starting point for the traversal of the parse tree.
     * Here the default implementation of "return visitChildren(ctx);" to start the traversal.
     * */
    antlrcpp::Any visitProgram (eelParser::ProgramContext* ctx);

    /*
     * Declaration Section
     * */

    antlrcpp::Any visitStmtBlock (eelParser::StmtBlockContext* ctx);

    /*
     * Top level declarations
     * */
    antlrcpp::Any visitLoopDecl (eelParser::LoopDeclContext* ctx);
    antlrcpp::Any visitSetupDecl (eelParser::SetupDeclContext* ctx);
    antlrcpp::Any visitIncludeDirective (eelParser::IncludeDirectiveContext* ctx);
    antlrcpp::Any visitTraitImplBlock (eelParser::TraitImplBlockContext* ctx);
    antlrcpp::Any visitImplBlock (eelParser::ImplBlockContext* ctx);

    /*
     * Variable declarations
     * */
    antlrcpp::Any visitVariableDecl (eelParser::VariableDeclContext* ctx);
    antlrcpp::Any visitConstDecl (eelParser::ConstDeclContext* ctx);
    antlrcpp::Any visitStaticDecl (eelParser::StaticDeclContext* ctx);
    antlrcpp::Any visitPinDecl (eelParser::PinDeclContext* ctx);
    antlrcpp::Any visitArrayDecl (eelParser::ArrayDeclContext* ctx);
    antlrcpp::Any visitReferenceDecl (eelParser::ReferenceDeclContext* ctx);
    antlrcpp::Any visitPointerDecl (eelParser::PointerDeclContext* ctx);

    /*
     * Type declarations
     * */
    antlrcpp::Any visitStructDecl (eelParser::StructDeclContext* ctx);
    antlrcpp::Any visitUnionDecl (eelParser::UnionDeclContext* ctx);
    antlrcpp::Any visitUntaggedUnionDecl (eelParser::UntaggedUnionDeclContext* ctx);
    antlrcpp::Any visitEnumDecl (eelParser::EnumDeclContext* ctx);
    antlrcpp::Any visitEventDecl (eelParser::EventDeclContext* ctx);
    antlrcpp::Any visitIntervalDecl (eelParser::IntervalDeclContext* ctx);
    antlrcpp::Any visitOnDecl (eelParser::OnDeclContext* ctx);
    antlrcpp::Any visitTraitDecl (eelParser::TraitDeclContext* ctx);

    /*
     * Function declarations
     * */
    antlrcpp::Any visitFnDecl (eelParser::FnDeclContext* ctx);
    antlrcpp::Any visitParamList (eelParser::ParamListContext* ctx);
    antlrcpp::Any visitFnParam (eelParser::FnParamContext* ctx);

    /*
     * Associated Function declarations
     * */
    antlrcpp::Any visitInstanceAssociatedFn (eelParser::InstanceAssociatedFnContext* ctx);
    antlrcpp::Any visitPartialInstanceAssociatedFn (eelParser::PartialInstanceAssociatedFnContext* ctx);
    antlrcpp::Any visitTypeAssociatedFn (eelParser::TypeAssociatedFnContext* ctx);
    antlrcpp::Any visitPartialTypeAssociatedFn (eelParser::PartialTypeAssociatedFnContext* ctx);
    antlrcpp::Any visitInstanceAssocParamList (eelParser::InstanceAssocParamListContext* ctx);

    /*
     * Namespace declaration
     * */
    antlrcpp::Any visitNamespaceDecl (eelParser::NamespaceDeclContext* ctx);


    /*
     * Usage Section
     * */
    antlrcpp::Any visitFqn (eelParser::FqnContext* ctx);


    /*
     * TODO: Features missing in grammar
     * TODO: Add way to call regular and associated functions
     * TODO: Add way to access struct and union members
     * TODO: Improve context by assigning parse trees of rules to context variables
     * */
};

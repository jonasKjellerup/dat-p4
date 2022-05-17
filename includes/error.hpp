#pragma once
#include <cstddef>
#include <string>
#include "antlr4-runtime.h"

using namespace antlr4;

struct Error {
public:

    struct Pos {
        Pos();
        Pos(size_t l, size_t c);
        std::size_t l;
        std::size_t c;
    };
    enum Kind {
        None,
        TypeMisMatch,
        InvalidReturnType,
        DuplicateEvent,
        AlreadyDefined,
        ExpectedVariable,
        UndefinedType,
    };
    explicit Error();
    explicit Error(Error::Kind kind);
    explicit Error(Error::Kind kind, std::string source, std::string expected, Pos location, size_t offset);
    explicit Error(Kind kind, Token* token, ParserRuleContext* ctx, const std::string& expected);
    std::string source;
    std::string expected;
    size_t offset;
    Pos location;
    Kind kind;
    /// Sets the location of the error in the source code
    void set_pos(Pos location);
    void set_expected(std::string expected);
    [[nodiscard]] Pos get_pos() const;

    /// Prints an error message corresponding to the error kind.
    void print();

};

#pragma once
#include <cstddef>
#include <string>

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
        ExpectedVariable,
        UndefinedType,
    };
    explicit Error();
    explicit Error(Error::Kind kind);
    explicit Error(Error::Kind kind, std::string source, std::string expected, Pos location, size_t offset);
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

struct InternalError : std::exception {
public:
    enum Subsystem {
        Codegen = 0,
        SymbolTable,
    };

    InternalError(Subsystem src, const char* msg);
    InternalError(Subsystem src, std::string&& msg);
    void print() const;

    Subsystem src;
    std::string msg;
};

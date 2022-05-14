#pragma once

#include <symbol_table.hpp>

namespace eel::symbols {

    struct Type {
    public:

        enum struct Kind {
            Primitive,
            Struct,
            Union,
            UntaggedUnion,
            Enum,
            Trait,
        };

        explicit Type(Kind kind);

        /// \brief Determines the name used in the output code.
        [[nodiscard]] virtual const std::string& type_target_name() const = 0;
        /// \brief Determines the name used in the source code.
        [[nodiscard]] virtual const std::string& type_source_name() const = 0;

        Kind kind;

    };

    struct Primitive final : public Type {
    public:
        explicit Primitive(const std::string&& global_name) noexcept;
        Primitive(const std::string&& source_name, const std::string&& target_name) noexcept;

        static Primitive u8;
        static Primitive u16;
        static Primitive u32;
        static Primitive u64;

        static Primitive i8;
        static Primitive i16;
        static Primitive i32;
        static Primitive i64;

        static Primitive f32;
        static Primitive f64;

        static Primitive boolean;

        static Primitive digital;
        static Primitive analog;

        static void register_primitives(Scope scope);
        [[nodiscard]] const std::string& type_target_name() const override;
        [[nodiscard]] const std::string& type_source_name() const override;

        Symbol::Id id;
    private:
        std::string source_name;
        std::string target_name;
    };

    struct Struct : public Type { // TODO
    public:
        Struct();
        std::unordered_map<std::string, Symbol::Id> members;
    };

}
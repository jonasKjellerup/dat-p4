#pragma once

#include <vector>
#include <string>
#include <symbol_table.hpp>
#include <eelParser.h>

namespace eel {
    struct Sequence;
}

namespace eel::symbols {

    struct Function_ {
        Symbol return_type;
        std::vector<Symbol> parameters;

        [[nodiscard]] virtual const std::string& target_id() const = 0;
        [[nodiscard]] virtual bool is_async() const = 0;
        [[nodiscard]] bool has_return_type() const;
    };

    struct ExternalFunction : public Function_ {
        std::string _target_id;

        [[nodiscard]] const std::string& target_id() const override;
        [[nodiscard]] constexpr bool is_async() const override;
    };

    struct Function : public Function_ {
    public:
        Function() = default;
        Function(Scope scope, std::string&& name);

        Scope scope;
        Sequence* sequence = nullptr;
        std::string type_id;

        eel::eelParser::StmtBlockContext* body = nullptr;

        [[nodiscard]] const std::string& target_id() const override;
        [[nodiscard]] bool is_async() const override;

    };
}
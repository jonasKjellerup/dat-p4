#pragma once

#include <vector>

#include <symbols/function.hpp>
#include <symbol_table.hpp>

namespace eel::symbols {


    struct EventHandle {};

    struct Event {
    public:
        /// \brief Whether the event occurrence
        ///        is determined by a predicate.
        bool has_predicate;

        /// \brief Whether the symbol is complete.
        /// An incomplete event is created when
        /// event handler is created before the corresponding
        /// event has been declared.
        bool is_complete;

        /// \brief Whether the event has been awaited.
        bool is_awaited;

        Function* predicate;

        void add_handle(Scope scope, SymbolTable* table);

        [[nodiscard]] const std::vector<Function>& get_handles() const;


    private:
        std::vector<Function> event_handles;
    };
}
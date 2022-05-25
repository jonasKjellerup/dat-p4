#pragma once

#include <vector>
#include <unordered_map>

#include <symbols/function.hpp>
#include <symbol_table.hpp>
#include <Visitors/utility.hpp>

namespace eel::symbols {

    using eel::visitors::SourcePos;

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

        std::string id;

        void compute_id(Symbol symbol);

        void add_handle(Scope scope, SourcePos pos);

        Function& get_handle(SourcePos pos);

        [[nodiscard]] const std::unordered_map<size_t, Function>& get_handles() const;


    private:
        std::unordered_map<size_t , Function> event_handles;
    };

}
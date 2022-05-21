#pragma once

#include <vector>
#include <tuple>

#include <symbol_table.hpp>

namespace eel {
    struct SequencePoint {
        enum Kind {
            SyncPoint,
            AsyncPoint,
            YieldPoint,
        };

        explicit SequencePoint(Kind kind = SyncPoint);
        virtual ~SequencePoint();

        [[nodiscard]] bool is_async() const;

        Kind kind;
        SequencePoint* next;
    };

    struct Block final : public SequencePoint {
        Scope scope;

        Block* parent;
        SequencePoint* child;

        explicit Block(Scope scope, Block* parent = nullptr);

        void mark_async();

        ~Block() override;

    };

    struct Sequence {
        Block* start;
        Block* current_block;
        SequencePoint* current_point;

        explicit Sequence(Scope scope);
        ~Sequence();

        Sequence& enter_block(Scope scope);
        Sequence& leave_block();
        Sequence& yield();

        void reset();
        SequencePoint* next();

        [[nodiscard]] std::tuple<SequencePoint*, Block*> snapshot() const;
        void restore(const std::tuple<SequencePoint*, Block*>& snapshot);

        bool is_next_async();

    };
}


#pragma once

#include <vector>

#include <symbol_table.hpp>

namespace eel {
    struct SequencePoint {
        enum Kind {
            SyncPoint,
            AsyncPoint,
            YieldPoint,
        };

        explicit SequencePoint(Kind kind = SyncPoint);

        Kind kind;
        SequencePoint* next;

        virtual ~SequencePoint();
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

        Sequence& enter_block(Scope scope);
        Sequence& leave_block();
        Sequence& yield();

        void reset();
        SequencePoint* next();

        ~Sequence();
    };
}


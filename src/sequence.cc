#include <sequence.hpp>

using namespace eel;

SequencePoint::SequencePoint(Kind kind) : kind(kind), next(nullptr) {}

SequencePoint::~SequencePoint() {
    delete next;
}

Block::Block(Scope scope, Block* parent)
        : SequencePoint(), scope(scope), parent(parent), child(nullptr) {}

void Block::mark_async() {
    auto b = this;
    while (b != nullptr) {
        b->kind = AsyncPoint;
        b = b->parent;
    }
}

Block::~Block() {
    delete child;
    delete next;
}

Sequence::Sequence(Scope scope) {
    start = new Block(scope);
    current_block = start;
    current_point = start;
}

Sequence& Sequence::enter_block(Scope scope) {
    auto block = new Block(scope, current_block);

    if (current_block == current_point) {
        current_block->child = block;
    } else {
        current_point->next = block;
    }
    current_point = block;
    current_block = block;

    return *this;
}

Sequence& Sequence::leave_block() {
    current_point = current_block;
    current_block = current_block->parent;

    return *this;
}

Sequence& Sequence::yield() {
    auto point = new SequencePoint(SequencePoint::YieldPoint);
    if (current_block == current_point) {
        current_block->child = point;
    } else {
        current_point->next = point;
    }

    current_point = point;
    current_block->mark_async();

    return *this;
}

void Sequence::reset() {
    current_block = start;
    current_point = start;
}

SequencePoint* Sequence::next() {
    auto block = dynamic_cast<Block*>(current_point);
    if (block != nullptr && block->child != nullptr) {
        current_point = block->child;
    } else {
        current_point = current_point->next;
    }

    if (current_block->next != nullptr) {
        current_point = current_block->next;
    } else {
        current_point = current_block->parent;
    }

    return current_point;
}

Sequence::~Sequence() {
    delete start;
}
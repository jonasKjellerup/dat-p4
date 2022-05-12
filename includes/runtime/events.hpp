#pragma once

#include <runtime/platform.hpp>
#include <runtime/primitives.hpp>

/// \brief Wrapper structure for managing a series of bit flags.
/// Bits are stored in groups based on a unsigned integer type
/// (defaults to an 8-bit uint).
template<size_t count, typename G = u8>
struct StatusFlags {
    static_assert(count > 0);

    static constexpr size_t group_size = sizeof(G) * 8;
    static constexpr size_t group_count = count / group_size + ((count % group_size) > 0);

    G storage[group_count];

    // TODO consider force inlining these
    constexpr G get(size_t offset) const {
        size_t group = offset / group_size;
        G in_group_offset = offset % group_size;
        return storage[group] & (1 << in_group_offset);
    }

    constexpr void set(size_t offset, bool state) {
        size_t group = offset / group_size;
        G in_group_offset = offset % group_size;

        G isolation_mask = ~(1 << in_group_offset);
        G insert = state ? (1 << in_group_offset) : 0;

        storage[group] = (storage[group] & isolation_mask) | insert;
    }

};

/// \brief Marker type for marking a type as an AsyncFunction.
/// Asynchronous functions are generally expected to implement the
/// following functionality:
///     - `using State = ...;`
///     - `template<typename... Args> static int begin_invoke(State&, Args&&...);`
///        This function should initialise a `State` instance and return
///        with the result of calling `step`
///     - `static int step(State&);`
///        Should continue the execution of the function until the next yield.
///        Should return 1 on completion otherwise zero.
struct AsyncFunction {}; // TODO consider moving if we need more non-event related, function things

/// \brief Concept for generalising over AsyncFunctions
template<typename T>
concept IsAsyncFunction = std::is_base_of<AsyncFunction, T>::value;

/// \brief Empty struct for representing the lack of a predicate.
struct PredicateLess {};

/// \brief An event defined by its predicate function.
/// The predicate type can be any type that implements
/// `static bool invoke()`, is an AsyncFunction, or is
/// the `PredicateLess` type.
template<typename Predicate>
struct Event {
    bool check() __attribute((always_inline)) {
        return Predicate::invoke();
    }
};

template<IsAsyncFunction AsyncPredicate>
struct Event<AsyncPredicate> {
    using State = typename AsyncPredicate::State;
    State state;

    bool check() {
        bool is_complete;
        state->__return = &is_complete;

        if (AsyncPredicate::step(state))
            return is_complete;
        return false;
    }
};

template<>
struct Event<PredicateLess> {
    void check() __attribute__((always_inline)) {}
};


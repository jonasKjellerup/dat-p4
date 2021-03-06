
#pragma once

#include <runtime/platform.hpp>
#include <runtime/primitives.hpp>
#include <runtime/utilities.hpp>


/// \brief Wrapper structure for managing a series of bit flags.
/// Bits are stored in groups based on a unsigned integer type
/// (defaults to an 8-bit uint).
template<size_t count, typename G = u8>
struct StatusFlags {
    static_assert(count > 0);

    using GroupType = G;

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

template<typename T>
struct IsAsyncFunctionTest : public std::bool_constant<IsAsyncFunction<T>> {
};

/// \brief Empty struct for representing the lack of a predicate.
struct PredicateLess {};

template<typename Handle, typename Flags, typename States>
struct InvokeHandle {
    static void invoke(size_t&, Flags&, States&) {
        Handle::invoke();
    }
};

template<IsAsyncFunction AsyncHandle, typename Flags, typename States>
struct InvokeHandle<AsyncHandle, Flags, States> {
    static void invoke(size_t& i, Flags& flags, States& states) {
        auto& state = AsyncHandle::get_state(states);
        // If already running
        if (flags.get(i)) {
            auto result = AsyncHandle::step(state);
            if (result)
                flags.set(i, false);
        } else {
            flags.set(i, !AsyncHandle::begin_invoke(state));
        }

        i += 1;
    }
};

template<typename EventState, typename... EventHandles>
struct Event_ {
    /// \brief Counts the number of async handles passed as type parameters
    static constexpr size_t count_async_handles() {
        auto i = 0;

        // perform `i += IsAsyncFunction<EventHandles>` for each parameter in EventHandles
        using arrT = int[];
        static_cast<void>(arrT{(i += IsAsyncFunction<EventHandles>, 0)...});

        return i;
    }

    // One flag for the status of each async handles
    // + 1 flag for manual emit
    StatusFlags<count_async_handles() + 1> handle_status {};
    EventState states {};

    u8 incomplete_tasks = 0;
    u8 awaiting = 0;

    void emit() {
        handle_status.set(0, true);
    }

    [[nodiscard]] decltype(handle_status.get(0)) has_emit_flag() const {
        return handle_status.get(0);
    }

    void invoke_handles() {
        size_t i_status = 1;
        // This is just a hack to call invoke_handle for each EventHandle given
        using arrT = int[];
        static_cast<void>(arrT{(InvokeHandle<EventHandles, decltype(handle_status), decltype(states)>
                ::invoke(i_status, handle_status, states), 0)...});
    }
};

/// \brief An event defined by its predicate function.
/// The predicate type can be any type that implements
/// `static bool invoke()`, is an AsyncFunction, or is
/// the `PredicateLess` type.
template<typename Predicate, typename State, typename... EventHandles>
struct Event : Event_<State, EventHandles...> {
    bool check() __attribute((always_inline)) {
        return Predicate::invoke();
    }
};

template<IsAsyncFunction AsyncPredicate, typename HState, typename... EventHandles>
struct Event<AsyncPredicate, HState, EventHandles...> : Event_<HState, EventHandles...> {
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

template<typename State, typename... EventHandles>
struct Event<PredicateLess, State, EventHandles...> : Event_<State, EventHandles...> {
    [[nodiscard]] constexpr bool check() const { return false; }
};



template<typename Event>
void run_handles(Event& event) {
    if (event.has_emit_flag() || event.check()) {
        event.invoke_handles();
    }
}
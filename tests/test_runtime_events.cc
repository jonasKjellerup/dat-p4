#include <catch.hpp>

#define NO_ARDUINO
#include <runtime/events.hpp>

static_assert(StatusFlags<46>::group_count == 6);
static_assert(StatusFlags<448, u64>::group_count == 7);


TEST_CASE("getting flags result in correct values", "[StatusFlags]") {
    StatusFlags<32> flags {};

    // It is worth noting that offsets are not counted strictly left to right
    // but rather groups are counted left to right and in group offsets
    // are counter right to left (from the least significant bit to the most).

    // flags.storage = 0b10001111'00101101'11000011'00101010;
    flags.storage[0] = 0b10001111;
    flags.storage[1] = 0b00101101;
    flags.storage[2] = 0b11000011;
    flags.storage[3] = 0b00101010;

    REQUIRE(flags.get(11) > 0);
    REQUIRE(flags.get(28) == 0);
    REQUIRE(flags.get(7) > 0);
    REQUIRE(flags.get(4) == 0);
    REQUIRE(flags.get(8) > 0);
    REQUIRE(flags.get(1) > 0);

}

TEST_CASE("setting flags updates correct bits", "[StatusFlags]") {
    StatusFlags<32> flags {};

    // flags.storage = 0b10001111'00101101'11000011'00101010;
    flags.storage[0] = 0b10001111;
    flags.storage[1] = 0b00101101;
    flags.storage[2] = 0b11000011;
    flags.storage[3] = 0b00101010;

    flags.set(11, false);
    flags.set(28, true);
    flags.set(7, false);
    flags.set(4, true);
    flags.set(8, false);
    flags.set(1, false);

    REQUIRE(flags.storage[0] == 0b00011101);
    REQUIRE(flags.storage[1] == 0b00100100);
    REQUIRE(flags.storage[2] == 0b11000011);
    REQUIRE(flags.storage[3] == 0b00111010);
}

TEST_CASE("Event_::count_async_handles provides accurate count", "[Event]") {
    struct F1 {};
    struct F2 {};
    struct AF1 : AsyncFunction {};
    struct AF2 : AsyncFunction {};

    REQUIRE(Event_<F1, F2>::count_async_handles() == 0);
    REQUIRE(Event_<F1, AF1, F2>::count_async_handles() == 1);
    REQUIRE(Event_<F1, AF1, F2, AF2>::count_async_handles() == 2);
    REQUIRE(Event_<AF1, AF2>::count_async_handles() == 2);
}

/*
 * Test semi equivalent to example:
 *
 * event predicateless_event;
 *
 * bool x = false;
 *
 * on predicateless_event {
 *  x = true;
 * }
 *
 * i16 x = 0;
 *
 * loop {
 *  if (x++ == 1) {
 *      emit predicateless_event;
 *  }
 * }
 *
 */
TEST_CASE("predicate-less event is not dispatched until emitted", "[Event]") {
    static bool x = false;
    struct EventHandle {
        static void invoke() {
            x = true;
        }
    };
    Event<PredicateLess, EventHandle> event;

    run_handles(event);
    REQUIRE(x == false); // we expect that the handle was not invoked
    event.emit();
    run_handles(event);
    // since the event has been emitted the handle should have been invoked.
    REQUIRE(x == true);
}
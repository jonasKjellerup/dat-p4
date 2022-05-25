#pragma once

#include <stdint.h>
#include <runtime/platform.hpp>

// Integral types

// u8, u16, and u32 are provided by USBAPI.h
// which is included through Arduino.h
#ifndef USING_ARDUINO
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
#endif
using u64 = uint64_t;

static_assert(sizeof(u8) == 1);
static_assert(sizeof(u16) == 2);
static_assert(sizeof(u32) == 4);
static_assert(sizeof(u64) == 8);


using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

static_assert(sizeof(i8) == 1);
static_assert(sizeof(i16) == 2);
static_assert(sizeof(i32) == 4);
static_assert(sizeof(i64) == 8);

using usize = size_t;

using f32 = float;
static_assert(sizeof(f32) == 4, "f32 is not 32-bit (wrong compilation target?)");

// Depending on the options passed to avr-gcc double may be either
// 4 or 8 bytes in size. If 8 bytes in size we use double. If 4 bytes
// in size then we use long double. Either one of them will always be 8 bytes.
// ( see: https://gcc.gnu.org/wiki/avr-gcc )
using f64 = std::conditional<sizeof(double) == 8, double, long double>::type;
static_assert(sizeof(f64) == 8, "f64 is not 64-bit (wrong compilation target?)");

// Pin types

struct digital {
    static int read(uint8_t pin) __attribute__ ((always_inline)) {
        return digitalRead(pin);
    }

    static void write(uint8_t pin, uint8_t val) __attribute__ ((always_inline)) {
        return digitalWrite(pin, val);
    }
};

struct analog {
    static int read(uint8_t pin) __attribute__ ((always_inline)) {
        return analogRead(pin);
    }

    static void write(uint8_t pin, uint8_t val) __attribute__ ((always_inline)) {
        return analogWrite(pin, val);
    }
};

template <typename PinType>
struct pin {
    uint8_t pin_id;

    pin(uint8_t pin) { // NOLINT(google-explicit-constructor)
        pin_id = pin;
    }

    void set_mode(uint8_t mode) __attribute__ ((always_inline)) {
        pinMode(pin_id, mode);
    }

    int read() __attribute__ ((always_inline)) {
        return PinType::read(pin_id);
    }

    void write(uint8_t val) __attribute__ ((always_inline)) {
        PinType::write(pin_id, val);
    }
};

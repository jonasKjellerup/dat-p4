#pragma once

/*
 * This header manages inclusion of platform specific headers
 * based on the definition given to the compiler.
 *
 * The following definition are expected for the specified platforms:
 *  TARGET_AVR      - AVR/Arduino
 *  TARGET_AMD64    - x86_64 platform
 *  TARGET_ARM64    - 64bit arm platform
 */

#ifdef TARGET_AVR

// avr-libc does not include the <type_traits> header
// so we provide reimplemenations for some traits.
#include <runtime/type_traits.hpp>

#else

#include <type_traits>

#endif

// We include arduino libraries if explicitly told to
// or if not on AMD64 and not explicitly told not to.
#if defined(ARDUINO) || (!defined(TARGET_AMD64) && !defined(NO_ARDUINO))
#define USING_ARDUINO
#include <arduino/Arduino.h>
#endif

#if defined(NO_ARDUINO) || (defined(TARGET_AMD64) && !defined(ARDUINO))
#include <runtime/mock_arduino.hpp>
#endif
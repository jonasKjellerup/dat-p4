#pragma once

/*
 * This header includes mock declarations for some
 * arduino functions that are needed for the runtime
 * library.
 */

#include <stdint.h>

void pinMode(uint8_t pin, uint8_t mode);
void digitalWrite(uint8_t pin, uint8_t val);
int digitalRead(uint8_t pin);
int analogRead(uint8_t pin);
void analogWrite(uint8_t pin, int val);
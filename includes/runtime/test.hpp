#pragma once

#include <stdlib.h>

void assert_true(bool expr) {
    if (!expr)
        exit(EXIT_FAILURE);
}

void assert_false(bool expr) {
    assert_true(!(expr));
}
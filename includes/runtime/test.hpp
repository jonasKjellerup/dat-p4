#pragma once

#include <stdlib.h>
#include <stdio.h>

void assert_true(bool expr) {
    if (!expr)
        exit(EXIT_FAILURE);
}

void assert_false(bool expr) {
    assert_true(!(expr));
}

void fail(const char* reason) {
    printf("%s\n", reason);
    exit(EXIT_FAILURE);
}

void pass(int) {
    exit(EXIT_SUCCESS);
}
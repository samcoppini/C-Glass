#include "test/test.h"

#include <stdio.h>

static int total_tests = 0;
static int tests_passed = 0;

bool _assert(bool x, const char *filename, int line) {
    if (!x) {
        fprintf(stderr, "Test failed in %s on line %d.\n", filename, line);
    }
    total_tests++;
    if (x) {
        tests_passed++;
    }
    return x;
}

int test_status() {
    printf("%d out of %d tests passed.\n", tests_passed, total_tests);
    return total_tests != tests_passed;
}

#ifndef TEST_TEST_H
#define TEST_TEST_H

#include <stdbool.h>

void _assert(bool x, const char *filename, int line);

int test_status();

#define ASSERT_TRUE(x) _assert(x, __FILE__, __LINE__)
#define ASSERT_FALSE(x) _assert(!(x), __FILE__, __LINE__)
#define ASSERT_EQUAL(a, b) _assert((a) == (b), __FILE__, __LINE__)

#endif

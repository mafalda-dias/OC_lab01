#ifndef CHECK_H
#define CHECK_H
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
static unsigned checks, failures;
#define CHECK_EQ(label, actual, expected) do { \
    uint64_t got_ = (uint64_t)(actual), want_ = (uint64_t)(expected); \
    ++checks; \
    if (got_ != want_) { ++failures; \
        fprintf(stderr, "FAIL %s: got %" PRIu64 ", expected %" PRIu64 "\n", \
                (label), got_, want_); } \
} while (0)
static int finish(const char *name)
{
    printf("%s: %u/%u checks passed\n", name, checks - failures, checks);
    return failures ? 1 : 0;
}
#endif

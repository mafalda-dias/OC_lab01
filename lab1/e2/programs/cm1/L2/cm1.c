/* %%%% This part was added: POSIX aligned allocation declaration %%%% */
#define _POSIX_C_SOURCE 200112L
/* %%%% This part was kept as it was before %%%% */
#include <errno.h>
#include <papi.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h> // exit()
#include <string.h>

/* %%%% This part was updated with clearer names and a wider L1 range %%%%
 * Students edit MIN/MAX for L2. Both must be powers of two, MIN >= 4.
 * Values and strides are in BYTES; elements are uint8_t.
 * #ifndef also permits small validation runs through compiler -D options.
 */
#ifndef WORKING_SET_MIN
#define WORKING_SET_MIN (4ULL * 1024)
#endif
#ifndef WORKING_SET_MAX
#define WORKING_SET_MAX (1ULL * 1024 * 1024)
#endif
/* %%%% This part was added: bounded work and independent samples %%%%
 * TARGET_UPDATES is a target, not a strict cap: use whole passes and at
 * least MIN_PASSES. Large dense arrays therefore require more updates.
 * Samples reuse the allocation; each sample has its own uncounted warm-up.
 */
#ifndef TARGET_UPDATES
#define TARGET_UPDATES (1024ULL * 1024)
#endif
#ifndef MIN_PASSES
#define MIN_PASSES 4
#endif
#ifndef N_SAMPLES
#define N_SAMPLES 5
#endif
#ifndef EVENT_NAME
#define EVENT_NAME "PAPI_L1_DCM"
#endif

/* %%%% This part was updated with the actual PAPI error code %%%% */
static void check_papi(int rc, const char *operation) {
    if (rc != PAPI_OK) {
        fprintf(stderr, "PAPI %s failed (%d): %s\n", operation, rc,
                PAPI_strerror(rc));
        fprintf(stderr, "Check event support and counter permissions on a native Linux machine.\n");
        exit(EXIT_FAILURE);
    }
}
/* %%%% This part was added: validate parameters before allocation %%%% */
static int power_of_two(uint64_t n) { return n && !(n & (n - 1)); }

/* %%%% This part was updated with a separate, non-inlined update loop %%%%
 * The byte update and power-of-two stride pattern are retained.
 * volatile ensures each specified byte read and write remains observable;
 * loop/control variables can still be optimised into registers.
 * This is an intentionally scalar access experiment, not application code.
 */
#if defined(__GNUC__) || defined(__clang__)
__attribute__((noinline))
#endif
static void update_array(volatile uint8_t *array, size_t size,
                         size_t stride, uint64_t passes) {
    for (uint64_t repeat = 0; repeat < passes; ++repeat) {
        for (size_t index = 0; index < size; index += stride) {
            /* %%%% This part was kept as it was before %%%% */
            array[index] = array[index] + 1;
        }
    }
}

/* %%%% This part was updated with an optional event-name argument %%%% */
int main(int argc, char **argv) {
    if (argc > 2) {
        fprintf(stderr, "Usage: %s [PAPI_EVENT_NAME]\n", argv[0]);
        return EXIT_FAILURE;
    }
    const char *event_name = argc == 2 ? argv[1] : EVENT_NAME;
    /* %%%% This part was added: reject invalid/overflowing ranges %%%% */
    if (!power_of_two(WORKING_SET_MIN) || !power_of_two(WORKING_SET_MAX) ||
        WORKING_SET_MIN < 4 || WORKING_SET_MIN > WORKING_SET_MAX ||
        WORKING_SET_MAX > SIZE_MAX / 2 || TARGET_UPDATES < 1 ||
        MIN_PASSES < 1 || N_SAMPLES < 1) {
        fprintf(stderr, "Invalid array range or repetition parameters.\n");
        return EXIT_FAILURE;
    }
    /* %%%% This part was kept as it was before: library initialisation %%%% */
    int retval = PAPI_library_init(PAPI_VER_CURRENT);
    /* %%%% This part was updated with a version-mismatch diagnostic %%%% */
    if (retval != PAPI_VER_CURRENT) {
        fprintf(stderr, "PAPI initialisation failed: expected %d, received %d.\n",
                PAPI_VER_CURRENT, retval);
        return EXIT_FAILURE;
    }
    /* %%%% This part was added: explicit user-space counting and event check %%%% */
    check_papi(PAPI_set_domain(PAPI_DOM_USER), "set_domain");
    int event_code;
    char event_buffer[PAPI_MAX_STR_LEN];
    if (strlen(event_name) >= sizeof(event_buffer)) {
        fprintf(stderr, "Event name too long.\n");
        return EXIT_FAILURE;
    }
    strcpy(event_buffer, event_name);
    check_papi(PAPI_event_name_to_code(event_buffer, &event_code), "event_name_to_code");
    check_papi(PAPI_query_event(event_code), "query_event");
    /* %%%% This part was kept as it was before %%%% */
    int EventSet = PAPI_NULL;
    /* %%%% This part was updated with selectable event and checked calls %%%% */
    check_papi(PAPI_create_eventset(&EventSet), "create_eventset");
    check_papi(PAPI_add_event(EventSet, event_code), "add_event");

    /* %%%% This part was updated with aligned, explicitly touched storage %%%%
     * Alignment is 4096 bytes, not an assumption about hardware line size.
     * Touch pages before counting to exclude initial demand-zero faults.
     */
    void *storage = NULL;
    int alloc_rc = posix_memalign(&storage, 4096, (size_t)WORKING_SET_MAX);
    if (alloc_rc != 0) {
        fprintf(stderr, "Allocation failed: %s\n", strerror(alloc_rc));
        return EXIT_FAILURE;
    }
    memset(storage, 0, (size_t)WORKING_SET_MAX);
    volatile uint8_t *array = storage;
    /* %%%% This part was added: raw CSV and run metadata on separate streams %%%% */
    fprintf(stderr, "event=%s min=%llu max=%llu target=%llu min_passes=%d samples=%d\n",
            event_name, (unsigned long long)WORKING_SET_MIN,
            (unsigned long long)WORKING_SET_MAX, (unsigned long long)TARGET_UPDATES,
            MIN_PASSES, N_SAMPLES);
    puts("event,array_bytes,stride_bytes,sample,samples,passes,updates,events,events_per_update");
    /* %%%% This part was updated: same doubling sweep, renamed array variable %%%% */
    for (size_t size = WORKING_SET_MIN; size <= WORKING_SET_MAX; size *= 2) {
        fprintf(stderr, "Array: %zu bytes\n", size);
        for (size_t stride = 1; stride <= size / 2; stride *= 2) {
            /* %%%% This part was updated: work no longer scales as 200*stride passes %%%% */
            uint64_t per_pass = size / stride;
            uint64_t passes = TARGET_UPDATES / per_pass + (TARGET_UPDATES % per_pass != 0);
            if (passes < MIN_PASSES) passes = MIN_PASSES;
            if (passes > UINT64_MAX / per_pass) {
                fprintf(stderr, "Update count overflow.\n");
                return EXIT_FAILURE;
            }
            uint64_t updates = passes * per_pass;
            /* %%%% This part was added: repeated, independently counted samples %%%% */
            for (int sample = 1; sample <= N_SAMPLES; ++sample) {
                update_array(array, size, stride, 1);  /* uncounted warm-up */
                long long values[1];
                /* %%%% This part was updated: count only the kernel, with no timers %%%%
                 * PAPI_start resets counters; the earlier reset/read are redundant.
                 * Function-call and counter-boundary overhead is not subtracted.
                 */
                check_papi(PAPI_start(EventSet), "start");
                update_array(array, size, stride, passes);
                check_papi(PAPI_stop(EventSet, values), "stop");
                /* %%%% This part was updated: raw integer counts and double precision %%%% */
                if (values[0] < 0) {
                    fprintf(stderr, "Negative counter value; measurement rejected.\n");
                    return EXIT_FAILURE;
                }
                printf("%s,%zu,%zu,%d,%d,%llu,%llu,%lld,%.12g\n",
                       event_name, size, stride, sample, N_SAMPLES,
                       (unsigned long long)passes, (unsigned long long)updates,
                       values[0], (double)values[0] / (double)updates);
            }
        }
    }
    /* %%%% This part was added: resource cleanup and output error detection %%%% */
    check_papi(PAPI_cleanup_eventset(EventSet), "cleanup_eventset");
    check_papi(PAPI_destroy_eventset(&EventSet), "destroy_eventset");
    PAPI_shutdown();
    free(storage);
    if (fflush(stdout) == EOF || ferror(stdout)) return EXIT_FAILURE;
    return EXIT_SUCCESS;
}
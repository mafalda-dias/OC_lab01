#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define WORKING_SET_MIN (64 * 1024) // 4 KiB
#define WORKING_SET_MAX (64*1024* 1024) // 1 MiB
#define N_SAMPLES (10)
#define TARGET_UPDATES (256 * 1024)

// returns elapsed time in nanoseconds
long get_elapsed_ns(struct timespec const *start) {
    struct timespec end;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);

    return ((end.tv_sec - start->tv_sec) * 1000000000L) +
           (end.tv_nsec - start->tv_nsec);
}

int main() {
    uint8_t *array = calloc(WORKING_SET_MAX, sizeof(uint8_t));
    if (array == NULL) {
        perror("calloc");
        return 1;
    }

    fputs("working_set_bytes,stride_bytes,sample,updates,elapsed_ns,ns_per_update\n",
          stdout);

    for (size_t working_set_size = WORKING_SET_MIN; working_set_size <= WORKING_SET_MAX;
         working_set_size *= 2) {
        fprintf(stderr, "[LOG]: running with array of size %zu KiB\n",
                working_set_size >> 10);
        fflush(stderr);
        for (size_t stride = 1; stride <= working_set_size / 2; stride = 2 * stride) {
            size_t limit = working_set_size - stride + 1;
            size_t updates_per_pass = (limit + stride - 1) / stride;
            size_t n_repetitions =
                (TARGET_UPDATES + updates_per_pass - 1) / updates_per_pass;

            for (size_t sample = 1; sample <= N_SAMPLES; sample++) {
                /* warm up the cache */
                for (size_t index = 0; index < limit; index += stride) {
                    array[index] = array[index] + 1;
                }

                struct timespec start_time;
                clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_time);

                size_t n_iterations = 0;
                /* ********************************************************** */
                for (size_t repeat = 0; repeat < n_repetitions; repeat++) {
                    for (size_t index = 0; index < limit;
                         index += stride, n_iterations++) {
                        array[index] = array[index] + 1;
                    }
                }
                /* ********************************************************** */

                long const elapsed_ns = get_elapsed_ns(&start_time);
                double const ns_per_update =
                    (double)elapsed_ns / (double)n_iterations;

                fprintf(stdout, "%zu,%zu,%zu,%zu,%ld,%lf\n", working_set_size,
                        stride, sample, n_iterations, elapsed_ns,
                        ns_per_update);
            }
        }
    }

    free(array);

    return 0;
}

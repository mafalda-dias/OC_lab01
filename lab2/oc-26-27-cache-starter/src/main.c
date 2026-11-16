#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cache.h"
#include "memory.h"
#include "stats.h"
static int hex_value(const char *s, uint32_t max, uint32_t *value)
{
    char *end;
    if (s == NULL || s[0] != '0' || s[1] != 'x' || !isxdigit((unsigned char)s[2]))
        return 0;
    for (const char *p = s + 2; *p; ++p)
        if (!isxdigit((unsigned char)*p)) return 0;
    errno = 0;
    unsigned long n = strtoul(s + 2, &end, 16);
    if (errno || *end || n > max) return 0;
    *value = (uint32_t)n;
    return 1;
}
static void summary(Stats s)
{
    printf("reads=%" PRIu64 "\nwrites=%" PRIu64 "\n", s.reads, s.writes);
    printf("l1_hits=%" PRIu64 "\nl1_misses=%" PRIu64 "\n", s.l1_hits, s.l1_misses);
    printf("victim_hits=%" PRIu64 "\nvictim_misses=%" PRIu64 "\n", s.victim_hits, s.victim_misses);
    printf("memory_reads=%" PRIu64 "\nmemory_writes=%" PRIu64 "\n", s.memory_reads, s.memory_writes);
    printf("cycles=%" PRIu64 "\n", s.cycles);
}
int main(int argc, char **argv)
{
    CacheMode mode;
    if (argc != 3) {
        fprintf(stderr, "Usage: %s dm|sa2|victim trace-file\n", argv[0]);
        return EXIT_FAILURE;
    }
    if (!strcmp(argv[1], "dm")) mode = MODE_DM;
    else if (!strcmp(argv[1], "sa2")) mode = MODE_SA2;
    else if (!strcmp(argv[1], "victim")) mode = MODE_VICTIM;
    else { fprintf(stderr, "Invalid mode: %s\n", argv[1]); return EXIT_FAILURE; }
    FILE *f = fopen(argv[2], "r");
    if (!f) { perror(argv[2]); return EXIT_FAILURE; }
    Cache cache = {0};
    memory_init(); stats_reset(); cache_init(&cache, mode);
    char line[1024];
    unsigned long number = 0;
    while (fgets(line, sizeof line, f)) {
        ++number;
        if (!strchr(line, '\n') && !feof(f)) {
            fprintf(stderr, "Line %lu too long\n", number); fclose(f); return EXIT_FAILURE;
        }
        char *op = strtok(line, " \t\r\n");
        if (!op || op[0] == '#') continue;
        char *a = strtok(NULL, " \t\r\n");
        char *v = strtok(NULL, " \t\r\n");
        char *extra = strtok(NULL, " \t\r\n");
        uint32_t address = 0, value = 0;
        if (!strcmp(op, "R") && a && !v &&
            hex_value(a, MEMORY_SIZE - 1, &address)) {
            uint8_t result = cache_read(&cache, address);
            printf("R 0x%04" PRIX32 " 0x%02X\n", address, (unsigned)result);
        } else if (!strcmp(op, "W") && a && v && !extra &&
                   hex_value(a, MEMORY_SIZE - 1, &address) && hex_value(v, 255, &value)) {
            cache_write(&cache, address, (uint8_t)value);
        } else if (!strcmp(op, "F") && !a) cache_flush(&cache);
        else if (!strcmp(op, "Z") && !a) stats_reset();
        else { fprintf(stderr, "Invalid operation at line %lu\n", number); fclose(f); return EXIT_FAILURE; }
    }
    if (ferror(f)) { perror("Reading trace"); fclose(f); return EXIT_FAILURE; }
    fclose(f);
    summary(stats_get());
    return EXIT_SUCCESS;
}

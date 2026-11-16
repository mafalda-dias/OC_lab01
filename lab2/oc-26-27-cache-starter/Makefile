CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -O0 -g
CPPFLAGS =
BUILD = build
INCLUDES = -Isrc -Itests
CACHE_MODULES = src/cache_dm.c src/cache_sa2.c src/cache_victim.c
CORE = src/cache.c $(CACHE_MODULES) src/stats.c src/memory.c
HEADERS = $(wildcard src/*.h) tests/check.h
TINY_FLAGS = -DBLOCK_SIZE=16 -DL1_SIZE=32 -DVICTIM_BLOCKS=1

.PHONY: all clean test test-part1 test-part2 test-part3 test-part4 \
        test-accounting test-integration test-traces tiny test-matrix FORCE
all: $(BUILD)/cachesim
# FORCE deliberately rebuilds: changing CPPFLAGS cannot leave stale objects.
$(BUILD):
	mkdir -p $@
$(BUILD)/cachesim: src/main.c $(CORE) $(HEADERS) FORCE | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) src/main.c $(CORE) -o $@
$(BUILD)/test-cache: tests/test_cache.c tests/event_sink.c src/cache.c $(CACHE_MODULES) src/memory.c $(HEADERS) FORCE | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) tests/test_cache.c tests/event_sink.c src/cache.c $(CACHE_MODULES) src/memory.c -o $@
$(BUILD)/test-stats: tests/test_stats.c src/stats.c $(HEADERS) FORCE | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) tests/test_stats.c src/stats.c -o $@
$(BUILD)/test-integration: tests/test_integration.c $(CORE) $(HEADERS) FORCE | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) tests/test_integration.c $(CORE) -o $@
test-part1 test-part2 test-part3: $(BUILD)/test-cache
	$(BUILD)/test-cache $(patsubst test-part%,%,$@)
test-accounting: $(BUILD)/test-stats
	$(BUILD)/test-stats
test-integration: $(BUILD)/test-integration
	$(BUILD)/test-integration
test-part4: $(BUILD)/test-stats $(BUILD)/test-integration
	@status=0; $(BUILD)/test-stats || status=1; $(BUILD)/test-integration || status=1; exit $$status
# Run every group even if an earlier group fails.
test: $(BUILD)/test-cache $(BUILD)/test-stats $(BUILD)/test-integration
	@status=0; for p in 1 2 3; do $(BUILD)/test-cache $$p || status=1; done; \
	$(BUILD)/test-stats || status=1; $(BUILD)/test-integration || status=1; exit $$status
tiny:
	$(MAKE) BUILD=build/tiny CPPFLAGS='$(TINY_FLAGS)' all
test-traces: tiny
	sh tests/run_traces.sh build/tiny/cachesim
test-matrix:
	@status=0; for cfg in '8 32 1' '16 64 2' '32 256 4'; do \
	set -- $$cfg; $(MAKE) --no-print-directory BUILD=build/matrix-$$1-$$2-$$3 \
	CPPFLAGS="-DBLOCK_SIZE=$$1 -DL1_SIZE=$$2 -DVICTIM_BLOCKS=$$3" test || status=1; done; exit $$status
clean:
	rm -rf build
FORCE:

# OC 2026/2027 Cache Simulator

Student starter, version 0.5. See `oc-26-27-cache-assignment.pdf` for the specification.
The cache and accounting algorithms are intentionally unfinished. The starter compiles successfully.
Unused-parameter warnings are expected until the TODOs are implemented.
**Functional tests initially fail.**

## Requirements

GCC with C11 (2011 C standard) support, GNU Make, a POSIX shell, and `diff`.
You may develop on any OS with these tools. Grading uses x86-64 Linux and
the supplied Makefile; keep the supplied compiler flags unchanged.
The simulator uses only the standard C library. No Python or external packages
are needed to build or run the student project.

## Four parts, 5 points each

1. Direct-mapped cache: reads, writes, dirty eviction, and flush.
2. Two-way L1 with LRU.
3. Direct-mapped L1 with a victim cache.
4. Counters, simulated time, and reset.

Parts 2 and 3 are separate extensions of Part 1. Detailed interface and build
instructions below complement the shorter assignment document.

## Files you complete and submit

| Part | Source file | Functions |
| --- | --- | --- |
| 1 | `src/cache_dm.c` | `dm_read`, `dm_write`, `dm_flush` |
| 2 | `src/cache_sa2.c` | `sa2_read`, `sa2_write`, `sa2_flush` |
| 3 | `src/cache_victim.c` | `victim_read`, `victim_write`, `victim_flush` |
| 4 | `src/stats.c` | `stats_record`, `stats_reset` (`stats_get` is supplied) |

Submit exactly these four C files. No report is required. Keep helper functions
static within their corresponding file. Each cache module implements one mode
independently; do not call functions in another student cache module. You may
adapt your own helper logic across the files.

`src/cache.c` supplies common initialization and dispatch; do not modify it.
`src/cache_impl.h` declares the student module functions. The original public
interface in `src/cache.h` is used by the driver and tests. Headers, Makefile,
memory model, driver, tests, and dispatcher are restored during evaluation.

`src/main.c` is the supplied driver: it reads input operations, calls the
simulator, and prints read values and final counters. `src/memory.c` supplies
the memory model; `src/cache.c` initializes the cache and dispatches operations
to the selected mode.

## Build and run

```sh
make
./build/cachesim dm inputs/conflict.trace
./build/cachesim sa2 inputs/conflict.trace
./build/cachesim victim inputs/worked.trace
```

The mode names mean direct-mapped L1 (`dm`), two-way set-associative L1
(`sa2`), and direct-mapped L1 plus a victim buffer (`victim`).

Those commands use the default configuration (256-byte L1, 16-byte blocks,
2 victim blocks). Supplied `.out` files instead use the tiny configuration:

```sh
make tiny
./build/tiny/cachesim victim inputs/worked.trace
make test-traces
```

Tiny means 32-byte L1, 16-byte blocks, 1 victim block. It reproduces the
worked example in the assignment exactly. `make test-traces` compares stdout
for all 15 trace/mode combinations; stderr is retained only for debugging.

## Test one part at a time

```sh
make test-part1
make test-part2
make test-part3
make test-part4
make test-accounting
make test-integration
make test
make test-matrix
```

Parts 1-3 link a supplied event sink instead of your statistics code, and use
memory audit counters to check transfers. Some tests seed a valid cache state
directly to isolate a feature from bugs in initialization or earlier accesses.
`test-accounting` tests only `stats.c`, with known events; `test-integration`
checks the complete system in all modes. `test-part4` runs both.

`make test` runs all five test groups even if some fail, and returns a nonzero
status on any failure. The public checks are examples, not the entire marking
suite; their raw pass percentage is not your grade. The assignment gives the
five-point allocation for each part.

`make test-matrix` runs these tests for three configurations (block bytes,
L1 bytes, victim blocks): (8,32,1), (16,64,2), and (32,256,4).
Each configuration has its own build directory. Other valid configurations
can be selected without editing the distributed files:

```sh
make BUILD=build/custom CPPFLAGS='-DBLOCK_SIZE=8 -DL1_SIZE=64 -DVICTIM_BLOCKS=4'
./build/custom/cachesim victim inputs/conflict.trace
```

Do not compare custom/default results with the tiny expected outputs.
The Makefile deliberately rebuilds targets to avoid stale configuration values.
`make clean` removes the `build/` tree.

## Interface and event ownership

`cache.h` defines the fixed state layout and supplied public interface;
`cache_impl.h` declares the student functions for each mode. In `sa2`,
set s uses `l1[2*s]` and `l1[2*s+1]`. Other modes use `l1[index]`.
L1 stores a tag; victim entries store the full block number (address divided
by block size). Byte addresses must be aligned before calling memory functions.

Use `Cache.clock` and `last_used` for replacement timestamps. Larger values
mean more recent. Statistics reset must never alter these fields. Do not keep
replacement state in global variables: it belongs to the `Cache` object.

Emit exactly one `EVENT_CPU_READ` or `EVENT_CPU_WRITE` per processor access,
one L1 hit/miss event, and a victim hit/miss event only on an L1 miss in victim
mode. The memory functions emit their own events automatically: **do not
emit a second memory event in the student cache modules**. Flushing emits no CPU/lookup events.

`memory_peek`, `memory_read_count`, and `memory_write_count` are test-only
observers. Do not call them from your solution. They do not affect time or
statistics. Memory audit counters are reset only by `memory_init`.

## Input and output

Trace operations are `R 0xADDRESS`, `W 0xADDRESS 0xBYTE`, `F`, and `Z`.
Blank lines and full-line `#` comments (optional leading whitespace) are ignored.
There are no inline comments. Hex digits may be upper/lower case; the prefix
must be `0x`. Addresses are in [0,65535], bytes in [0,255].

Reads print `R 0xAAAA 0xVV` with uppercase digits and fixed widths. Writes,
flushes, and resets print nothing. At EOF, nine decimal counters are printed
as `name=value`, one per line, in the order shown in `expected/`. There is no
implicit final flush. Error messages go to stderr and return failure.

## Submission

Submit only `cache_dm.c`, `cache_sa2.c`, `cache_victim.c`, and `stats.c`.
No report, headers, build outputs, or additional source files are required.
The four parts are worth 5 points each. When investigating performance, keep
victim capacity separate from L1 capacity in comparisons.

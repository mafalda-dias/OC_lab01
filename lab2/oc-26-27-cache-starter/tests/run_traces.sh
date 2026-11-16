#!/bin/sh
# Run from the project root. The executable must use the tiny configuration.
set -u
exe=${1:-build/tiny/cachesim}
mkdir -p build/trace-results
status=0
for name in empty conflict worked reset lru; do
    for mode in dm sa2 victim; do
        out="build/trace-results/$name.$mode.out"
        err="build/trace-results/$name.$mode.log"
        if "$exe" "$mode" "inputs/$name.trace" >"$out" 2>"$err"; then
            if diff -u "expected/$name.$mode.out" "$out"; then
                printf 'PASS %s %s\n' "$name" "$mode"
            else
                printf 'FAIL %s %s (output differs)\n' "$name" "$mode"
                status=1
            fi
        else
            printf 'FAIL %s %s (execution failed; see %s)\n' "$name" "$mode" "$err"
            status=1
        fi
    done
done
exit "$status"

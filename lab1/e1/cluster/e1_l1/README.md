# Simple cache profiling assignment

## Dependencies

- A C11-compatible C compiler, such as GCC or Clang
- Python 3
- Python `matplotlib`

```bash
python3 -m pip install --user matplotlib
```

## Run everything

```bash
./run
```

The script builds the program, runs the benchmark, and creates the raw CSV,
formatted Markdown table, and PNG plot. Install `matplotlib` if needed:

To run each step separately:

```bash
make             # Build the program
make run         # Write raw measurements to results.csv
make table       # Create cache_table.md
make plots       # Create cache_table.md and cache_time.png

make clean       # Remove files and clean dir
```

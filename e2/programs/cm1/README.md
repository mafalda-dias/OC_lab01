<!-- %%%% This part was updated: generic student instructions and current paths %%%% -->
# Cache behaviour through hardware counters

<!-- %%%% This part was updated: setup %%%% -->
Use a native Linux computer with access to hardware performance counters.
The experiment requires a C compiler, make, Python 3 and Matplotlib. No graphical
session is needed. Use the same computer throughout each experiment and,
where possible, for the timing exercise too.

From the extracted kit root, build the bundled PAPI library:

```bash
cd e2/papi-7.2.0/src
bash configure
make -j4
./utils/papi_avail -e PAPI_L1_DCM
./utils/papi_avail -e PAPI_L2_DCM
cd ../../programs/cm1
```

No system-wide PAPI installation is required. If Matplotlib is not already
available, create and activate a Python environment:

```bash
python3 -m venv .venv
source .venv/bin/activate
python3 -m pip install matplotlib
```

<!-- %%%% This part was updated: L1 experiment %%%% -->
## L1

```bash
bash run L1 PAPI_L1_DCM
```

The default array-size range is 4 KiB to 1 MiB. Sizes and strides double.
Edit WORKING_SET_MIN and WORKING_SET_MAX in cm1.c if the range does not
cover the capacity transition. Use powers of two and keep the other
benchmark parameters unchanged.

<!-- %%%% This part was updated: L2 experiment and generic fallback %%%% -->
## L2

Edit WORKING_SET_MIN and WORKING_SET_MAX to investigate larger arrays.
As a starting point, use twice your estimated L1 capacity, rounded up to a
power of two, through 64 MiB. Adjust the range if necessary.

```bash
bash run L2 PAPI_L2_DCM
```

If this event is unavailable or produces an error, try:

```bash
bash run L2 'L2_RQSTS:DEMAND_DATA_RD_MISS'
```

The alternative event counts L2 demand data-read misses. Its availability
depends on the processor. Record the event used. If neither event works,
consult your lab instructor. For errors unrelated to the selected event,
such as compilation or missing Python packages, inspect the diagnostic logs.

To inspect the alternative event's description from this directory:

```bash
../../papi-7.2.0/src/utils/papi_native_avail -e 'L2_RQSTS:DEMAND_DATA_RD_MISS'
```

The runner does not automatically substitute events. A listed event may still
fail when measured. Negative counts are rejected, not converted to zero.

<!-- %%%% This part was updated: outputs and reruns %%%% -->
## Results

The first argument to run names the output directory; the optional second
argument selects the PAPI event, overriding EVENT_NAME in cm1.c.
The script rebuilds, measures and generates:

- results.csv: individual event counts, update counts and events per update.
- cache_table.md: median, minimum and maximum for each array-size/stride pair.
- cache_misses.png: median events per update against stride, with one curve
  per array size and automatic labels.
- event_details.txt: the description of the exact event selected.

Platform/build information, diagnostic logs and source/script copies are also
saved. Retain these for inspection; submit the files requested by the handout.

Rerunning replaces generated files in the selected output directory after
measurement and plotting succeed. Copy any results you wish to preserve first.
Unrelated files are retained. If measurement or plotting fails, previous results
remain intact and diagnostics are kept in a .tmp.* directory. Publishing the
successful files is not a single atomic operation. make clean removes only
build products.

To regenerate a table and plot without rerunning measurements:

```bash
python3 plot_results.py L1/results.csv --output-dir L1
```

<!-- %%%% This part was updated: concise measurement description %%%% -->
## Interpreting measurements

The benchmark updates one-byte array elements. Each array-size/stride pair has
five samples. Each sample has one uncounted warm-up pass, followed by at least
four measured passes and enough passes to reach a target of 2^20 updates.
The actual update count may exceed the target. The volatile array accesses
retain the specified byte reads and writes while loop control is optimised.

Events per update is the raw counter value divided by the measured number of
updates. It is not necessarily a miss probability: each update reads and writes,
and other accesses in the measured region may contribute. Hardware prefetching
and other effects influence the results. Interpret the selected event according
to its recorded definition. Compare curves at common strides to estimate cache
capacity; use several suitable array sizes when assessing line size.

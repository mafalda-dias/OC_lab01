#!/usr/bin/env python3
# %%%% This part was added %%%%
# Entire file replaces the shell/gnuplot pipeline. Its commented originals
# remain beside this file, and byte-for-byte copies are in e2/original/.

# %%%% This part was added: dependencies and headless PNG rendering %%%%
import argparse
import csv
import math
from collections import defaultdict
from pathlib import Path
from statistics import median
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from matplotlib.ticker import LogLocator, LogFormatterMathtext

# %%%% This part was added: validate the measured sweep before plotting %%%%
def read_results(path):
    groups = defaultdict(list)
    events = set()
    counts = set()
    with path.open(newline='') as handle:
        reader = csv.DictReader(handle)
        required = {'event', 'array_bytes', 'stride_bytes', 'sample', 'samples',
                    'passes', 'updates', 'events', 'events_per_update'}
        if not required.issubset(reader.fieldnames or []):
            raise ValueError('CSV columns do not match the revised benchmark.')
        for row in reader:
            n, s = int(row['array_bytes']), int(row['stride_bytes'])
            sample, samples = int(row['sample']), int(row['samples'])
            passes, updates, raw = (int(row[k]) for k in ('passes', 'updates', 'events'))
            if (n < 4 or n & (n-1) or s < 1 or s & (s-1) or s > n//2
                    or samples < 1 or not 1 <= sample <= samples
                    or passes < 1 or updates != passes * (n//s) or raw < 0):
                raise ValueError('Invalid sizes, counts or sample metadata.')
            ratio = raw / updates
            supplied = float(row['events_per_update'])
            if not math.isfinite(supplied) or not math.isclose(supplied, ratio, rel_tol=1e-9, abs_tol=1e-12):
                raise ValueError('Counter ratio disagrees with raw counts.')
            events.add(row['event'])
            counts.add(samples)
            groups[n, s].append((sample, ratio))
    if len(events) != 1 or len(counts) != 1:
        raise ValueError('Expected one event and one sample count in a non-empty CSV.')
    samples = counts.pop()
    sizes = sorted({n for n, _ in groups})
    if any(b != 2*a for a, b in zip(sizes, sizes[1:])):
        raise ValueError('Missing array sizes in sweep.')
    for n in sizes:
        s = 1
        while s <= n//2:
            values = groups.get((n, s), [])
            if sorted(i for i, _ in values) != list(range(1, samples+1)):
                raise ValueError(f'Missing or duplicate samples: array={n}, stride={s}.')
            s *= 2
    return events.pop(), {key: [value for _, value in rows] for key, rows in groups.items()}

# %%%% This part was added: automatic binary-unit legend labels %%%%
def size_label(n):
    return f'{n / (1024*1024):g} MiB' if n >= 1024*1024 else f'{n / 1024:g} KiB'

# %%%% This part was added: summary table and dynamically labelled plot %%%%
def main():
    parser = argparse.ArgumentParser(description='Summarise real PAPI benchmark CSV measurements.')
    parser.add_argument('csv', type=Path, nargs='?', default=Path('results.csv'))
    parser.add_argument('--output-dir', type=Path, default=Path('.'))
    args = parser.parse_args()
    event, groups = read_results(args.csv)
    args.output_dir.mkdir(parents=True, exist_ok=True)
    lines = [f'# {event}: events per update', '',
             '| Array (bytes) | Stride (bytes) | Samples | Median | Minimum | Maximum |',
             '|---:|---:|---:|---:|---:|---:|']
    for (n, s), values in sorted(groups.items()):
        lines.append(f'| {n} | {s} | {len(values)} | {median(values):.8g} | {min(values):.8g} | {max(values):.8g} |')
    (args.output_dir / 'cache_table.md').write_text('\n'.join(lines)+'\n')
    fig, ax = plt.subplots(figsize=(12, 6.5), layout='constrained')
    for n in sorted({n for n, _ in groups}):
        strides = sorted(s for size, s in groups if size == n)
        ax.plot(strides, [median(groups[n, s]) for s in strides], marker='o',
                markersize=3, label=size_label(n))
    ax.set_xscale('log', base=2)
    ax.xaxis.set_major_locator(LogLocator(base=2, numticks=30))
    ax.xaxis.set_major_formatter(LogFormatterMathtext(base=2))
    ax.set_xlabel('Stride (bytes)')
    ax.set_ylabel(f'{event}: events per update (median)')
    ax.set_title('Cache events by array size')
    ax.set_ylim(bottom=0)
    ax.grid(True, alpha=.25)
    ax.legend(title='Array size', ncol=2, fontsize=9)
    fig.savefig(args.output_dir / 'cache_misses.png', dpi=160)
    plt.close(fig)
    print(f'Wrote cache_table.md and cache_misses.png for {event}.')

# %%%% This part was added: fail clearly on incomplete/invalid data %%%%
if __name__ == '__main__':
    try:
        main()
    except (ValueError, OSError) as exc:
        raise SystemExit(f'Plotting failed: {exc}')

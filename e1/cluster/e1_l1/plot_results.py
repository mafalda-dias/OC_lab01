#!/usr/bin/env python3
import argparse
import csv
import statistics
from collections import defaultdict

import matplotlib.pyplot as plt


def format_size(byte_count):
    if byte_count >= 1024 * 1024:
        return f"{byte_count / (1024 * 1024):.2f} MiB"
    if byte_count < 1024:
        return f"{byte_count} B"
    return f"{byte_count / 1024:.1f} KiB"


def main():
    parser = argparse.ArgumentParser(description="Create cache timing results")
    parser.add_argument("csv_file", nargs="?", default="results.csv")
    parser.add_argument(
        "--output",
        default="cache_table.md",
        help="output table path (default: cache_table.md)",
    )
    parser.add_argument(
        "--plot",
        action="store_true",
        help="also create cache_time.png",
    )
    parser.add_argument(
        "--max-plot-size",
        type=int,
        default=None,
        metavar="BYTES",
        help="maximum working-set size shown in the plot (default: all data)",
    )
    args = parser.parse_args()
    samples = defaultdict(list)

    with open(args.csv_file, newline="", encoding="utf-8") as handle:
        for row in csv.DictReader(handle):
            cache_size = int(row["working_set_bytes"])
            stride = int(row["stride_bytes"])
            samples[(cache_size, stride)].append(float(row["ns_per_update"]))

    points = sorted(
        (cache_size, stride, statistics.median(values), min(values), max(values))
        for (cache_size, stride), values in samples.items()
    )
    if not points:
        raise SystemExit("No benchmark results found")

    headers = ["Working set", "Stride", "Median (ns/update)", "Minimum", "Maximum"]
    table_rows = []
    with open(args.output, "w", encoding="utf-8") as handle:
        for cache_size, stride, median, minimum, maximum in points:
            table_rows.append(
                [
                    format_size(cache_size),
                    format_size(stride),
                    f"{median:.2f}",
                    f"{minimum:.2f}",
                    f"{maximum:.2f}",
                ]
            )

        widths = [max(len(headers[index]), *(len(row[index]) for row in table_rows)) for index in range(len(headers))]

        def format_row(values):
            cells = [values[0].ljust(widths[0])]
            cells.extend(values[index].rjust(widths[index]) for index in range(1, len(values)))
            return "| " + " | ".join(cells) + " |\n"

        handle.write(format_row(headers))
        handle.write(
            "| "
            + " | ".join(
                [":" + "-" * (widths[0] - 1)]
                + ["-" * (width - 1) + ":" for width in widths[1:]]
            )
            + " |\n"
        )
        for row in table_rows:
            handle.write(format_row(row))

    if args.plot:
        if args.max_plot_size is None:
            plot_points = points
        else:
            plot_points = [
                point for point in points if point[0] <= args.max_plot_size
            ]
        if not plot_points:
            raise SystemExit("No results within the requested plot range")
        by_cache_size = defaultdict(list)
        for cache_size, stride, median, _, _ in plot_points:
            by_cache_size[cache_size].append((stride, median))

        plt.figure(figsize=(10, 6))
        for cache_size, values in sorted(by_cache_size.items()):
            x, y = zip(*values)
            plt.plot(
                x,
                y,
                marker="o",
                markersize=4,
                linewidth=1.5,
                label=format_size(cache_size),
            )
        plt.xscale("log", base=2)
        plt.xlabel("Stride (bytes)")
        plt.ylabel("Median time (ns/update)")
        max_plotted_size = max(point[0] for point in plot_points)
        plt.title(
            "Stride sweep by working-set size "
            f"(up to {format_size(max_plotted_size)})"
        )
        plt.grid(True, which="both", alpha=0.25)
        plt.legend(title="Working set", fontsize="small", ncol=2)
        plt.tight_layout()
        plt.savefig("cache_time.png", dpi=180)
        plt.close()


if __name__ == "__main__":
    main()

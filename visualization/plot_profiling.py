import argparse
import os
import re
from collections import defaultdict

import matplotlib.pyplot as plt
import numpy as np


def parse_arguments() -> argparse.Namespace:
    """Parse command-line arguments for plotting profiling results.
    """
    parser = argparse.ArgumentParser(
        description="Plot profiling runtime breakdown."
    )
    parser.add_argument(
        "-i",
        "--input_dir",
        type=str,
        default="../logs/profiling/trillium/perf-reports/",
        help="Directory containing input data files",
    )
    parser.add_argument(
        "-o",
        "--output_file",
        type=str,
        default=None,
        help="Path to save the output plot",
    )
    parser.add_argument(
        "-v",
        "--verbose",
        action="store_true",
        help="Enable verbose output",
    )
    return parser.parse_args()


def parse_perf_report(filename: str) -> dict:
    """Parse percentages from a performance report.
    """
    report = defaultdict(float)

    percentage_pattern = re.compile(r"(\d+(?:\.\d+)?)%")

    with open(filename, "r", encoding="utf-8") as file:
        for line in file:
            match = percentage_pattern.search(line)
            if not match:
                continue

            percent = float(match.group(1))
            line = line.strip()

            if line.startswith("Compute:"):
                report["Compute"] = percent
            elif line.startswith("MPI:"):
                report["MPI"] = percent
            elif line.startswith("I/O:"):
                report["I/O"] = percent
            elif line.startswith("Single-core code:"):
                report["Single-core code"] = percent
            elif line.startswith("OpenMP regions:"):
                report["OpenMP regions"] = percent
            elif line.startswith("Time in collective calls:"):
                report["collective"] = percent
            elif line.startswith("Time in point-to-point calls:"):
                report["point-to-point"] = percent
            elif line.startswith("Computation:"):
                report["computation"] = percent
            elif line.startswith("Synchronization:"):
                report["synchronization"] = percent

    return dict(report)


def compute_runtime_breakdown(report: dict) -> dict:
    """Convert hierarchical profiling percentages into normalized fractions.

    The final breakdown always sums to 1.0 and consists of:
        - Single-core code
        - OpenMP computation
        - OpenMP synchronization
        - MPI collective
        - MPI point-to-point
        - I/O
    """

    # ---------------------------------------------------------
    # Top-level runtime categories
    # ---------------------------------------------------------
    compute = report.get("Compute", 0.0)
    mpi = report.get("MPI", 0.0)
    io = report.get("I/O", 0.0)

    breakdown = {}

    # ---------------------------------------------------------
    # Compute
    # ---------------------------------------------------------
    # If OpenMP is not present, the entire Compute region is
    # single-core code.
    if "OpenMP regions" not in report:
        breakdown["Single-core code"] = compute / 100.0
        breakdown["OpenMP regions: computation"] = 0.0
        breakdown["OpenMP regions: synchronization"] = 0.0

    else:
        openmp_regions = report["OpenMP regions"]
        single_core = report.get("Single-core code", 0.0)
        computation = report.get("computation", 0.0)
        synchronization = report.get("synchronization", 0.0)

        breakdown["Single-core code"] = single_core * compute / 10000.0

        breakdown["OpenMP regions: computation"] = (
            computation
            * openmp_regions
            * compute
            / 1_000_000.0
        )

        breakdown["OpenMP regions: synchronization"] = (
            synchronization
            * openmp_regions
            * compute
            / 1_000_000.0
        )

    # ---------------------------------------------------------
    # MPI
    # ---------------------------------------------------------
    breakdown["MPI regions: collective"] = (
        report.get("collective", 0.0)
        * mpi
        / 10000.0
    )

    breakdown["MPI regions: point-to-point"] = (
        report.get("point-to-point", 0.0)
        * mpi
        / 10000.0
    )

    # ---------------------------------------------------------
    # I/O
    # ---------------------------------------------------------
    breakdown["I/O"] = io / 100.0

    # ---------------------------------------------------------
    # Normalize to exactly 1.0
    # ---------------------------------------------------------
    total = sum(breakdown.values())

    if total <= 0:
        raise ValueError("Runtime breakdown has zero total.")

    breakdown = {
        name: value / total
        for name, value in breakdown.items()
    }

    return breakdown


def plot(breakdowns: dict, output_file: str | None = None) -> None:
    """Plot normalized runtime breakdowns as a horizontal stacked bar chart.
    """

    settings = list(breakdowns.keys())

    segment_names = [
        "Single-core code",
        "OpenMP regions: computation",
        "OpenMP regions: synchronization",
        "MPI regions: collective",
        "MPI regions: point-to-point",
        "I/O",
    ]

    colors = ['#0C5DA5', '#00B945', '#F4D03F', '#FF9500',
              '#FF2C00', '#845B97', '#474747', '#9e9e9e']
    hatches = ['/', '-', 'x', '+', '.', '\\', 'o', '|']

    data = {
        name: np.array(
            [
                breakdowns[setting].get(name, 0.0)
                for setting in settings
            ]
        )
        for name in segment_names
    }

    fig, ax = plt.subplots(figsize=(10, 6))
    left = np.zeros(len(settings))

    for segment_name, color, hatch in zip(segment_names, colors, hatches):
        values = data[segment_name]
        bars = ax.barh(
            settings,
            values,
            left=left,
            label=segment_name,
            color=color,
            height=0.6,
            hatch=hatch
        )
        # ax.bar_label(bars, label_type="center",
        #              fmt=lambda x: f'{x:.2f}' if x != 0 else '')
        left += values

    ax.set_xlabel("Fraction of runtime")
    ax.set_xlim(0, 1)
    ax.legend(facecolor="white", framealpha=0.7, frameon=True)
    plt.tight_layout()

    if output_file:
        output_dir = os.path.dirname(os.path.abspath(output_file))
        os.makedirs(output_dir, exist_ok=True)

        fig.savefig(output_file, format="pdf")
        print(f"Saved plot to: {output_file}")
        plt.close(fig)
    else:
        plt.show()


def main():
    args = parse_arguments()
    prefix = args.input_dir

    reports = {
        "FFT-based with OpenMP": "omp-fft.txt",
        "Feedforward with OpenMP": "omp-qpff.txt",
        "FFT-based with off-node MPI": "mpi-fft.txt",
        "Feedforward with off-node MPI": "mpi-qpff.txt",
        "FFT-based with OpenMP+MPI": "hybrid-fft.txt",
        "Feedforward with OpenMP+MPI": "hybrid-qpff.txt",
    }

    breakdowns = {}

    for name, filename in reports.items():
        filepath = os.path.join(prefix, filename)
        if not os.path.isfile(filepath):
            raise FileNotFoundError(
                f"Performance report not found: {filepath}")
        report = parse_perf_report(filepath)
        breakdowns[name] = compute_runtime_breakdown(report)
        if args.verbose:
            print(f"\n{name}")
            print(f"\tReport: {filepath}")
            print(f"\tBreakdown: {breakdowns[name]}")

    plot(breakdowns, output_file=args.output_file)


if __name__ == "__main__":
    plt.style.use('science.mplstyle')
    main()

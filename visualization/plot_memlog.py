import argparse
from collections import defaultdict
import re
import os
import numbers

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.lines import Line2D

from plot_constants import COLOR_CYCLE_SIZE, NREPEATS, TRILLIUM_MEMLOG_BENCHMARKS as BENCHMARKS


CURRENT_COLOR = 0
CURRENT_PATTERN = 0

LINE_PATTERNS = [
    {"linestyle": "-", "marker": None},
    {"linestyle": "--", "marker": None},
    {"linestyle": ":", "marker": None},
    {"linestyle": "-.", "marker": None},
    {"linestyle": "-", "marker": "o"},
    {"linestyle": "-", "marker": "^"},
    {"linestyle": "-", "marker": "s"},
    {"linestyle": "-", "marker": "*"},
]


def get_color() -> str:
    """Get the current color name and move to the next color in the cycler.
    """
    global CURRENT_COLOR
    color_name = f'C{CURRENT_COLOR % COLOR_CYCLE_SIZE}'
    CURRENT_COLOR += 1
    return color_name


def get_line_pattern() -> str:
    """Get the current line/marker pattern and move to the next pattern.
    """
    global CURRENT_PATTERN
    pattern = LINE_PATTERNS[CURRENT_PATTERN % len(LINE_PATTERNS)]
    CURRENT_PATTERN += 1
    return pattern


def parse_arguments() -> argparse.Namespace:
    """Parse command-line arguments for plotting benchmark results.
    """
    parser = argparse.ArgumentParser(
        description="Plot memory usage from parallel benchmark results."
    )
    parser.add_argument('-i', '--input_dir', type=str,
                        help='Directory containing input data files')
    parser.add_argument('-o', '--output_file', type=str,
                        help='Path to save the output plot (default: none)')
    parser.add_argument('-l', '--parallel_level', type=str, choices=[
                        'omp', 'mpi', 'hybrid'], default='omp', help='Parallelization method used (default: %(default)s)')
    parser.add_argument('-s', '--scaling_type', type=str, choices=[
                        'strong', 'weak'], default='strong', help='Scaling analysis type (default: %(default)s)')
    parser.add_argument('-v', '--verbose', action='store_true',
                        help='Enable verbose output')
    return parser.parse_args()


def parse_etime_to_seconds(etime):
    """
    Parse etime in MM:SS or HH:MM:SS into seconds.
    """
    parts = [int(p) for p in etime.split(':')]
    if len(parts) == 2:
        m, s = parts
        return m * 60 + s
    elif len(parts) == 3:
        h, m, s = parts
        return h * 3600 + m * 60 + s
    else:
        raise ValueError(f'Unexpected etime format: {etime}')


def read_memlog_files(paths) -> pd.DataFrame:
    """Read memory log files given by paths into one DataFrame, summing the
    memory across different nodes. Used for off-node MPI and hybrid parallel.
    """
    dfs = []
    for path in paths:
        df = read_memlog_file(path)
        dfs.append(df)
    return pd.concat(dfs, axis=0).groupby('time_s').sum().reset_index()


def read_memlog_file(path) -> pd.DataFrame:
    """Read memory log file given by path into one DataFrame, summing the memory
    across different tasks. Used for OpenMP and in-node MPI.
    """
    df = pd.read_csv(path, sep=r'\s+',
                     names=["etime", "pid", "cpu", "mem", "rss"])
    df["time_s"] = df["etime"].apply(parse_etime_to_seconds)
    df["rss"] = df["rss"] / (10 ** 6)
    df = df[["time_s", "rss"]]
    df = df.groupby(["time_s"]).sum().reset_index()
    return df[["time_s", "rss"]]


def group_memlog_files(parallel_level: str, input_dir: str) -> dict:
    patterns = {
        "omp": [r"^memlog_(\d+)t_\d+r.txt$"],
        "mpi": [r"^memlog_(\d+)p_\d+r.txt$", r"^memlog_(\d+)p_\d+r_[a-zA-Z0-9]+.txt$"],
        "hybrid": [r"^memlog_(\d+)p_(\d+)t_\d+r_[a-zA-Z0-9]+.txt$"]
    }

    groups = defaultdict(list)

    for fname in os.listdir(input_dir):
        match_list = [re.search(p, fname) for p in patterns[parallel_level]]
        if not any(match_list):
            continue

        match = match_list[0] if match_list[0] else match_list[1]
        if parallel_level == 'omp' or parallel_level == 'mpi':
            key = int(match.group(1))
        else:
            processes = int(match.group(1))
            threads = int(match.group(2))
            key = (processes, threads)
        groups[key].append(os.path.join(input_dir, fname))

    return groups


def prepare_data(input_dir, parallel_level, verbose):
    """Read data from input_dir based on parallel_level and print the result if
    verbose.
    """
    df_list = []
    peak_rss = defaultdict(list)
    groups = group_memlog_files(parallel_level, input_dir)
    for task, paths in groups.items():
        df = read_memlog_files(paths)
        df['task'] = [task] * df.shape[0]
        df['rss'] = df['rss'] / NREPEATS
        df_list.append(df)
        peak_rss[task] = df['rss'].max()

    all_rss = pd.concat(df_list, axis=0)
    mean_rss = all_rss.groupby(['task', 'time_s']).mean()
    std_rss = all_rss.groupby(['task', 'time_s']).std()

    if verbose:
        print(f"Collected data from {input_dir}:")
        print("Peak RSS:")
        print(peak_rss)
        print("Mean RSS:")
        print(mean_rss)
        print("Standard deviation RSS:")
        print(std_rss)

    return sorted(all_rss['task'].unique()), all_rss, peak_rss, mean_rss, std_rss


def create_figure(parallel_level: str, input_dir: str | None) -> tuple[plt.Figure, plt.Axes]:
    """Create a figure based on type and parallel.
    """
    fig = plt.figure()
    if input_dir is not None:
        ax = fig.add_subplot()
        ax.set_xlabel("Time (s)")
        ax.set_ylabel("RSS (GB)")
        return fig, ax

    if parallel_level == "hybrid":
        ax = fig.add_subplot(projection="3d")
        ax.set_xlabel("Processes")
        ax.set_ylabel("Threads")
        ax.set_zlabel("RSS (GB)")
    else:
        ax = fig.add_subplot()
        ax.set_xlabel("Threads" if parallel_level == "omp" else "Processes")
        ax.set_ylabel("RSS (GB)")
    return fig, ax


def compute_limits(tasks_list: list, verbose: bool) -> dict:
    limits = defaultdict(list)

    if len(tasks_list) < 1:
        return limits

    xmin = min([sorted(tasks)[0] for tasks in tasks_list if len(
        tasks) > 0 and isinstance(tasks[0], numbers.Integral)], default=-1)
    xmax = max([sorted(tasks)[-1] for tasks in tasks_list if len(tasks)
                > 0 and isinstance(tasks[0], numbers.Integral)], default=-1)
    xpad = 0.08 * (xmax - xmin)
    limits["xlim"] = (xmin - xpad, xmax + xpad)

    if verbose:
        print(f'Limits of tasks:')
        print(f'\txlim=[{xmin},{xmax}]')

    if xmin == -1 or xmax == -1:
        xmin = min([sorted(tasks)[0][0] for tasks in tasks_list if len(
            tasks) > 0], default=-1)
        xmax = max([sorted(tasks)[-1][0] for tasks in tasks_list if len(tasks)
                    > 0], default=-1)
        ymin = min([sorted(tasks)[0][1] for tasks in tasks_list if len(
            tasks) > 0], default=-1)
        ymax = max([sorted(tasks)[-1][-1] for tasks in tasks_list if len(
                    tasks) > 0], default=-1)
        xpad = 0.08 * (xmax - xmin)
        ypad = 0.08 * (ymax - ymin)
        limits['xlim'] = [xmin - xpad, xmax + xpad]
        limits['ylim'] = [ymin - ypad, ymin + ypad]
        if verbose:
            print(f'Limits of tasks:')
            print(f'\txlim=[{xmin},{xmax}], ylim=[{ymin},{ymax}]')
    return limits


def set_axis_limit(ax: plt.Axes, xlim: list[int] | None = None, ylim: list[int] | None = None, zlim: list[int] | None = None) -> None:
    if xlim is not None and len(xlim) == 2:
        ax.set_xlim(left=xlim[0], right=xlim[1])
    if ylim is not None and len(ylim) == 2:
        ax.set_ylim(bottom=ylim[0], top=ylim[1])
    if zlim is not None and len(zlim) == 2:
        ax.set_zlim(bottom=zlim[0], top=zlim[1])


def plot_memory(ax: plt.Axes, parallel_level, verbose, input_dirs, labels):
    rss_list, tasks_list, peaks_list = [], [], []

    for input_dir in input_dirs:
        tasks, rss, peaks, _, _ = prepare_data(
            input_dir, parallel_level, verbose)
        rss_list.append(rss)
        tasks_list.append(tasks)
        peaks_list.append(peaks)

    limits = compute_limits(tasks_list, verbose)
    set_axis_limit(limits)

    patches = []
    if parallel_level == 'omp' or parallel_level == 'mpi':
        for tasks, peaks, lab in zip(tasks_list, peaks_list, labels):
            color = get_color()
            pattern = get_line_pattern()
            values = [peaks[t] for t in tasks]
            ax.plot(
                tasks, values, linestyle=pattern["linestyle"] or "", marker=pattern["marker"] or "")
            patches.append(Line2D([0], [0], color=color, linestyle=pattern["linestyle"] or "",
                                  marker=pattern["marker"] or "", label=lab, ))
    else:
        for tasks, peaks, lab in zip(tasks_list, peaks_list, labels):
            color = get_color()
            pattern = get_line_pattern()
            procs = np.array([t[0] for t in tasks])
            threads = np.array([t[1] for t in tasks])
            values = np.array([peaks[t] for t in tasks])
            # Connect same number of processes
            for p in np.unique(procs):
                mask = procs == p
                ax.plot(procs[mask], threads[mask], values[mask], color=color,
                        linestyle=pattern["linestyle"] or "", marker=pattern["marker"] or "")

            # Connect same number of threads
            for t in np.unique(threads):
                mask = threads == t
                ax.plot(procs[mask], threads[mask], values[mask], color=color,
                        linestyle=pattern["linestyle"] or "", marker=pattern["marker"] or "")

            patches.append(Line2D([0], [0], color=color, linestyle=pattern["linestyle"] or "",
                                  marker=pattern["marker"] or "", label=lab, ))
    ax.legend(handles=patches)


def get_task_label(parallel_level: str, task: int | tuple[int]) -> str:
    """Return human-readable string describing number and type of task based on
    parallel_level.
    """
    if parallel_level == 'omp':
        if task == 1:
            return '1 thread'
        return f'{task} threads'
    elif parallel_level == 'mpi':
        if task == 1:
            return '1 process'
        return f'{task} processes'
    else:
        s = ''
        if task[0] == 1:
            s += '1 process '
        else:
            s += f'{task[0]} processes '
        if task[1] == 1:
            s += '1 thread'
        else:
            s += f'{task[1]} threads'
        return s


def plot_timeseries(ax: plt.Axes, parallel_level, tasks, all_rss, peak_rss, mean_rss, std_rss):
    for t in tasks:
        color = get_color()

        # Plot actual data points
        all_df = all_rss[all_rss['task'] == t]
        ax.scatter(all_df['time_s'], all_df['rss'], color=color, s=1)

        # Plot average line
        mean_df = mean_rss.loc[t]
        ax.plot(mean_df.index, mean_df['rss'], color=color,
                label=get_task_label(parallel_level, t))

        # Plot standard deviation across runs
        std_df = std_rss.loc[t]
        ax.errorbar(
            mean_df.index, mean_df['rss'], yerr=std_df['rss'], color=color, capsize=4)

    ax.legend()


def main():
    args = parse_arguments()
    fig, ax = create_figure(args.parallel_level, args.input_dir)

    if args.input_dir is not None:
        tasks, all_rss, peak_rss, mean_rss, std_rss = prepare_data(
            args.input_dir, args.parallel_level, args.verbose)
        plot_timeseries(ax, args.parallel_level, tasks, all_rss,
                        peak_rss, mean_rss, std_rss)
    else:
        dirs = BENCHMARKS[args.scaling_type][args.parallel_level]["dirs"]
        labels = BENCHMARKS[args.scaling_type][args.parallel_level]["labels"]
        plot_memory(ax, args.parallel_level, args.verbose, dirs, labels)

    if args.output_file:
        plt.savefig(args.output_file, format="pdf")
    else:
        plt.show()


if __name__ == "__main__":
    plt.style.use('science.mplstyle')
    main()

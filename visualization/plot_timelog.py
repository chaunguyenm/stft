import argparse
import os
import re
from collections import defaultdict

import matplotlib.pyplot as plt
from matplotlib.patches import Patch
import numpy as np

from plot_constants import COLOR_CYCLE_SIZE, TRILLIUM_TIMELOG_BENCHMARKS as BENCHMARKS


CURRENT_COLOR = 0


def get_color() -> str:
    """Get the current color name and move to the next color in the cycler.
    """
    global CURRENT_COLOR
    color_name = f'C{CURRENT_COLOR % COLOR_CYCLE_SIZE}'
    CURRENT_COLOR += 1
    return color_name


def parse_filename(fname: str, parallel_level: str) -> int | tuple[int] | None:
    """Match fname against the correct syntax based on parallel_level and return
    the corresponding task count.
    """
    patterns = {
        "omp": r"^timelog_(\d+)t_\d+r.txt$",
        "mpi": r"^timelog_(\d+)p_\d+r.txt$",
        "hybrid": r"^timelog_(\d+)p_(\d+)t_\d+r.txt$",
    }

    match = re.search(patterns[parallel_level], fname)
    if not match:
        return None

    if parallel_level == "hybrid":
        return int(match.group(1)), int(match.group(2))
    return int(match.group(1))


def parse_arguments() -> argparse.Namespace:
    """Parse command-line arguments for plotting benchmark results.
    """
    parser = argparse.ArgumentParser(
        description="Plot runtime or speedup from parallel benchmark results."
    )
    parser.add_argument('-i', '--input_dir', type=str,
                        help='Directory containing input data files')
    parser.add_argument('-o', '--output_file', type=str,
                        help='Path to save the output plot (default: none)')
    parser.add_argument('-p', '--plot_type', type=str, choices=[
                        'runtime', 'speedup'], default='runtime', help='Type of plot to generate (default: %(default)s)')
    parser.add_argument('-l', '--parallel_level', type=str, choices=[
                        'omp', 'mpi', 'hybrid'], default='omp', help='Parallelization method used (default: %(default)s)')
    parser.add_argument('-s', '--scaling_type', type=str, choices=[
                        'strong', 'weak'], default='strong', help='Scaling analysis type (default: %(default)s)')
    parser.add_argument('-v', '--verbose', action='store_true',
                        help='Enable verbose output')
    return parser.parse_args()


def get_realtime(f: str) -> float:
    """Read a file f that records the output of 'time' and return the real runtime.
    """
    with open(f, 'r') as fr:
        for line in fr:
            if line.startswith("real"):
                return float(line.split()[1])
    raise ValueError(f"No 'real' time found in {f}")


def prepare_data(input_dir: str, parallel_level: str, verbose: bool) -> dict:
    """Read data from input_dir based on parallel_level and print the result if
    verbose.
    """
    if not os.path.exists(input_dir) or not os.path.isdir(input_dir):
        raise FileNotFoundError(f"{input_dir} not found.")

    data = defaultdict(list)
    for fname in os.listdir(input_dir):
        task = parse_filename(fname, parallel_level)
        if task is None:
            continue
        runtime = get_realtime(os.path.join(input_dir, fname))
        data[task].append(runtime)

    if verbose:
        print(f'Collected data from {input_dir}:')
        for t in sorted(data.keys()):
            print(f'\ttask={str(t):<8}\truntime={data[t]}')
    return data


def compute_limits(data: dict, verbose: bool) -> dict:
    limits = defaultdict(list)
    ks = sorted(data.keys())
    if len(ks) < 1:
        return limits
    if isinstance(ks[0], int):
        limits['xlim'] = [ks[0], ks[-1]]
        if verbose:
            print(f'Limits of tasks:')
            print(f'\txlim=[{ks[0]},{ks[-1]}]')
    else:
        limits['xlim'] = [ks[0][0], ks[-1][0]]
        limits['ylim'] = [ks[0][1], ks[-1][-1]]
        if verbose:
            print(f'Limits of tasks:')
            print(
                f'\txlim=[{ks[0][0]},{ks[-1][0]}], ylim=[{ks[0][1]},{ks[-1][-1]}]')
    return limits


def compute_stats(data: dict, verbose: bool) -> tuple[list[int | tuple[int, int]], np.ndarray, np.ndarray]:
    """
    Compute statistics for data, return means and standard deviations and
    print output if verbose.
    """
    tasks = sorted(data)
    means = np.array([np.mean(data[t]) for t in tasks])
    stds = np.array([np.std(data[t], ddof=1) for t in tasks])

    if verbose:
        print(f'Statistics:')
        for i, t in enumerate(tasks):
            print(
                f'\ttask={str(t):<8}\tmean={means[i]:.6f}\tstd={stds[i]:.6f}\truntime={data[t]}')
    return (tasks, means, stds)


def compute_speedup(tasks: list[int | tuple[int, int]], means: np.ndarray, base_task: int | tuple[int], verbose: bool) -> np.ndarray:
    """Compute speedup given average runtime per task count means and the
    baseline task count base_task.
    """
    base = means[tasks.index(base_task)]
    speedup = base / means
    if verbose:
        print('Speedup of means of runtime:')
        for i, t in enumerate(tasks):
            print(f'\ttask={str(t):<8}\tspeedup={speedup[i]}')
    return speedup


def create_figure(plot_type: str, parallel_level: str) -> tuple[plt.Figure, plt.Axes]:
    """Create a figure based on type and parallel.
    """
    fig = plt.figure()
    if parallel_level == "hybrid":
        ax = fig.add_subplot(projection="3d")
        ax.set_xlabel("Processes")
        ax.set_ylabel("Threads")
        ax.set_zlabel("Time (s)" if plot_type == "runtime" else "Speedup")
    else:
        ax = fig.add_subplot()
        ax.set_xlabel("Threads" if parallel_level == "omp" else "Processes")
        ax.set_ylabel("Time (s)" if plot_type == "runtime" else "Speedup")
    return fig, ax


def set_axis_limit(ax: plt.Axes, xlim: list[int] | None = None, ylim: list[int] | None = None, zlim: list[int] | None = None) -> None:
    if xlim is not None and len(xlim) == 2:
        ax.set_xlim(left=xlim[0], right=xlim[1])
    if ylim is not None and len(ylim) == 2:
        ax.set_ylim(bottom=ylim[0], top=ylim[1])
    if zlim is not None and len(zlim) == 2:
        ax.set_zlim(bottom=zlim[0], top=zlim[1])


def plot_ideal_2d(scaling_type: str, ax: plt.Axes, tasks: list[int]) -> None:
    """Plot the ideal speedup in 2D based on scaling_type.
    """
    if scaling_type == "strong":
        ax.plot(tasks, tasks, linestyle=":", color="lightgray")
    else:
        ax.plot(tasks, [1]*len(tasks), linestyle=":", color="lightgray")


def plot_one_2d(plot_type: str, ax: plt.Axes, color: str,
                data: dict, tasks: list[tuple[int, int]], means: np.ndarray,
                stds: np.ndarray, speedup: np.ndarray) -> None:
    """Plot a single run in 2D (for OpenMP and MPI parallel levels).
    """
    if plot_type == "runtime":
        # Plot scatter points for actual runtime
        for t in tasks:
            plt.scatter([t]*len(data[t]), data[t], color=color)
        # plt.errorbar(tasks, means, yerr=stds,
        #              capsize=4, color=f'C{color % 7}')
        # Plot box for statistical analysis
        box_data = [data[t] for t in tasks]
        plt.boxplot(box_data, positions=tasks, widths=0.6, patch_artist=True,
                    boxprops=dict(facecolor=color, alpha=0.3), medianprops=dict(color=color))
        # Plot average runtime
        plt.plot(tasks, means, color=color)
    else:
        # Plot actual speedup
        ax.plot(tasks, speedup, color=color)


def plot_all_2d(parallel_level: str, scaling_type: str, plot_type: str,
                verbose: bool, ax: plt.Axes, dirs: list[str], labels: list[str]) -> None:
    patches, tasks_list = [], []

    # Plot all input directories in the same plot and collect line colors for legend
    for directory, label in zip(dirs, labels):
        # Compute all necessary data
        data = prepare_data(directory, parallel_level, verbose)
        limits = compute_limits(data, verbose)
        tasks, means, stds = compute_stats(data, verbose)
        speedup = compute_speedup(tasks, means, 1, verbose)

        # Update data
        tasks_list.append(tasks)

        # Plot
        color = get_color()
        set_axis_limit(ax, xlim=limits['xlim'])
        plot_one_2d(plot_type, ax, color, data, tasks, means, stds, speedup)
        patches.append(Patch(facecolor=color, label=label))

    # Plot ideal speedup
    if plot_type == 'speedup':
        # Sanity check
        if len({tuple(tasks) for tasks in tasks_list}) != 1:
            print(
                f'WARNING: different thread/process values {tasks_list}, \
                    using the first element to plot ideal speedup!')
        plot_ideal_2d(scaling_type, ax, tasks_list[0])

    # Add legend
    ax.legend(handles=patches)


def plot_ideal_3d(scaling_type: str, ax: plt.Axes, tasks: list[tuple[int]], max_speedup: float) -> None:
    """Plot the ideal speedup in 3D based on scaling_type. Use max_speedup to
    set the view of the 3D figure.
    """
    procs = np.array([t[0] for t in tasks])
    threads = np.array([t[1] for t in tasks])
    x, y = np.meshgrid(np.unique(procs), np.unique(threads))
    if scaling_type == 'strong':
        z = np.clip(x * y, 0, max_speedup*2)
        ax.plot_surface(
            x, y, z, linestyle=':', color='lightgray', alpha=0.5)
    else:
        z = np.full_like(x, 1)
        ax.plot_surface(
            x, y, z, linestyle=':', color='lightgray', alpha=0.5)
    ax.set_zlim(0, max_speedup*2)


def plot_one_3d(plot_type: str, ax: plt.Axes, color: str,
                data: dict, tasks: list[tuple[int, int]], means: np.ndarray,
                stds: np.ndarray, speedup: np.ndarray) -> None:
    """Plot a single run in 3D (for hybrid parallel level).
    """
    procs = np.array([t[0] for t in tasks])
    threads = np.array([t[1] for t in tasks])

    # Plot runtime
    if plot_type == "runtime":
        # Plot scatter points for actual runtime
        for p in np.unique(procs):
            xs, ys, zs = [], [], []
            for t in np.unique(threads):
                xs.append([p]*len(data[(p, t)]))
                ys.append([t]*len(data[(p, t)]))
                zs.append(data[(p, t)])
            ax.scatter(xs, ys, zs, color=color)

        # Connect same number of processes
        for p in np.unique(procs):
            mask = procs == p
            ax.errorbar(procs[mask], threads[mask], means[mask],
                        yerr=stds[mask], capsize=4, color=color)

        # Connect same number of threads
        for t in np.unique(threads):
            mask = threads == t
            ax.errorbar(procs[mask], threads[mask], means[mask],
                        yerr=stds[mask], capsize=4, color=color)

    # Plot speedup
    else:
        # Connect same number of processes
        for p in np.unique(procs):
            mask = procs == p
            ax.plot(procs[mask], threads[mask], speedup[mask], color=color)

        # Connect same number of threads
        for t in np.unique(threads):
            mask = threads == t
            ax.plot(procs[mask], threads[mask], speedup[mask], color=color)


def plot_all_3d(parallel_level: str, scaling_type: str, plot_type: str,
                verbose: bool, ax: plt.Axes, dirs: list[str], labels: list[str]) -> None:
    patches, tasks_list = [], []
    max_speedup = -1

    # Plot all input directories in the same plot and collect line colors for legend
    for directory, label in zip(dirs, labels):
        # Compute all necessary data
        data = prepare_data(directory, parallel_level, verbose)
        limits = compute_limits(data, verbose)
        tasks, means, stds = compute_stats(data, verbose)
        speedup = compute_speedup(tasks, means, (1, 1), verbose)

        # Update data
        tasks_list.append(tasks)
        max_speedup = max(max_speedup, speedup.max())

        # Plot
        color = get_color()
        set_axis_limit(ax, xlim=limits['xlim'], ylim=limits['ylim'])
        plot_one_3d(plot_type, ax, color, data, tasks, means, stds, speedup)
        patches.append(Patch(facecolor=color, label=label))

    # Plot ideal speedup
    if plot_type == 'speedup':
        # Sanity check
        if len({tuple(tasks) for tasks in tasks_list}) != 1:
            print(
                f'WARNING: different (process, thread) values {tasks_list}, using the first element to plot ideal speedup!')
        plot_ideal_3d(scaling_type, ax, tasks_list[0], max_speedup)

    # Add legend
    ax.legend(handles=patches)


def main():
    args = parse_arguments()
    fig, ax = create_figure(args.plot_type, args.parallel_level)

    if args.input_dir:
        dirs = [args.input_dir]
        labels = ["Benchmark"]
    else:
        dirs = BENCHMARKS[args.scaling_type][args.parallel_level]["dirs"]
        labels = BENCHMARKS[args.scaling_type][args.parallel_level]["labels"]

    if args.parallel_level == "hybrid":
        plot_all_3d(args.parallel_level, args.scaling_type, args.plot_type,
                    args.verbose, ax, dirs, labels)
    else:
        plot_all_2d(args.parallel_level, args.scaling_type, args.plot_type,
                    args.verbose, ax, dirs, labels)

    if args.output_file:
        plt.savefig(args.output_file, format="pdf")
    else:
        plt.show()


if __name__ == "__main__":
    plt.style.use('science.mplstyle')
    main()

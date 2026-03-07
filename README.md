# Benchmarking Visualization 

Simple Python scripts that generate visualizations from benchmarking results.

## Getting Started

### Prerequisites

* [python>=3.11](https://www.python.org/downloads/)

### Installation

1. (Optional) Create a virtual environment
   ```sh
   python -m venv env
   ```

2. Install necessary Python packages
   ```sh
   source env/bin/activate
   pip install -r requirements.txt
   ```

## Usage

### Runtime

* Print all options
   ```sh
   python plot_timelog.py -h
   ```

* Plot comparison of different algorithms, e.g., show a runtime plot of all
  algorithms in shared-memory parallel and with weak scaling
   ```sh
   python plot_timelog.py -p runtime -l omp -s weak
   ```

* Plot individual run, e.g., show a speedup plot with hybrid
  distributed-shared-memory parallel and weak scaling
   ```sh
   python plot_timelog.py -p speedup -l hybrid -s weak -i ../logs/scaling/niagara/weak-scaling/1-number/timelog_2467
   ```

### Memory

* Print all options
   ```sh
   python plot_memlog.py -h
   ```

* Plot comparison of different algorithms, e.g., show a memory plot of all
  algorithms in distributed-memory parallel and with weak scaling
   ```sh
   python plot_memlog.py -l mpi -s weak
   ```

* Plot individual run, e.g., show memory usage over time with shared-memory parallel and strong scaling
   ```sh
   python plot_memlog.py -l omp -s strong -i ../logs/scaling/niagara/strong-scaling/memlog_12380
   ``` 

**Note:**

* `plot_constants.py` defines the input for comparison plots.

* Trillium is the default cluster for comparison plots. To change to Niagara
  cluster, modify the scripts to import the Niagara constants instead of
  Trillium constants from `plot_constants.py`.

  For `plot_timelog.py`:
  ```python
  from plot_constants import COLOR_CYCLE_SIZE, NIAGARA_TIMELOG_BENCHMARKS as BENCHMARKS
  ```

  For `plot_memlog.py`:
  ```python
  from plot_constants import COLOR_CYCLE_SIZE, NREPEATS, NIAGARA_MEMLOG_BENCHMARKS as BENCHMARKS
  ```

* For individual plot, the `-l` and `-s` options do not enforce the given parallel
  level or scaling type on the input directory `-i` but only exist to ensure
  correct plot type (2D or 3D) and task unit (thread, process, or
  thread+process).

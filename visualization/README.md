# Parallel Short-time Fourier Transforms

An implentation and analysis of several parallel short-time Fourier transform
algorithms.

## Description

This project studies the parallel implementations of algorithms that calculate short-time
Fourier transform, commonly used audio/signal processing. Two algorithms are
provided: a parallel FFT-based implementation and a quasi-parallel feedforward
implementation. Each algorithm is parallelized in three ways: shared-memory with
OpenMP, distributed-memory with MPI, and hybrid distributed-shared-memory with
OpenMP+MPI.

Benchmarking experimens are conducted on [the Niagara/Teach computing cluster](https://docs.scinet.utoronto.ca/index.php/Teach) and
[the Trillium computing
cluster](https://docs.alliancecan.ca/wiki/Trillium_Quickstart). All job scripts
for profiling and scaling analysis, together with their logs, and some
basic Python scripts to generate visualization from the job output are provided.

## Getting Started

Clone the project.

   ```sh
   git clone https://github.com/chaunguyenm/stft.git
   cd stft/src
   ```

### Running on a computing cluster

Load all the necessary modules on the cluster.

   ```sh
   source trilliumcluster
   ```

### Running locally

Build the Docker image and run a container. Mount the current directory
to have access to all the code and output on your local machine.

   ```sh
   docker build -t stft-image ./src
   docker run -v .:/project -it stft-image bash
   ```

## Usage

1. Compile the program. This command will output an executable `main`.

   ```sh
   make
   ```

Some options are available for compilation and can be used in combination with
each other:

* `DEBUG=1`: compile in debugging mode
* `WITH_OMP=1`: compile with OpenMP
* `WITH_MPI=1`: compile with MPI

You may need to recompile when switching between parallel modes:

   ```sh
   # Compile with OpenMP
   make WITH_OMP=1

   # Clean up
   make clean
   make distclean

   # Recompile with MPI
   make WITH_MPI=1
   ```

   2. Run the program, e.g., compute the short-time Fourier transform of the
      input signal `in.nc` using 4 MPI processes and the feedforward algorithm
      and write the result to `out.nc`
      ```sh
      mpirun -n 4 ./main -p mpi -a qpff -s -f in.nc -o out.nc 1048576 4096
      ```

`main` takes two arguments, `num-samples` and `window-size`, in this order. If
an input file is not provided with `-f` option, the program will generate a real
signal `signal` of size `num-samples` where `signal[i]=i`. If an input file is
provided, the signal specified in the file will be used.
It also requires options for the algorithm `-a` and the parallel scheme `-p`. 
For detailed usage of `main`, run `main -h`.

3. Verify the results against FFTW

   ```sh
   # Compile verifier
   make verifier WITH_OMP=1

   # Test
   ./verifier -f in.nc -o out.nc 1048576 4096
   ```

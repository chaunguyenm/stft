#!/bin/bash
#SBATCH --nodes=2
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=40
#SBATCH --time=00:15:00
#SBATCH --job-name profiling_mpi_fft_job
#SBATCH --output=profiling_mpi_fft_output_%j.txt
#SBATCH --mail-type=FAIL

cd $SLURM_SUBMIT_DIR

# load required modules
source trilliumsetup

make clean
make distclean
make WITH_MPI=1
map --mpi --output=profiling_mpi_fft_${SLURM_JOB_ID}.map --profile --export-functions=profiling_functions_mpi_fft_${SLURM_JOB_ID}.csv mpirun -n ${SLURM_JOB_NUM_NODES} ./main -p mpi -a fft -s 1048576 4096

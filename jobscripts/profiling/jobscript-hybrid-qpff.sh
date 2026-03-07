#!/bin/bash
#SBATCH --nodes=8
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=192
#SBATCH --time=00:15:00
#SBATCH --job-name profiling_hybrid_qpff_job
#SBATCH --output=profiling_hybrid_qpff_output_%j.txt
#SBATCH --mail-type=FAIL

cd $SLURM_SUBMIT_DIR

# Load required modules
source trilliumsetup

# Clean and compile
make clean
make distclean
make WITH_OMP=1 WITH_MPI=1

# Set number of threads
export OMP_NUM_THREADS=40

# Run program with profiling
map --mpi --output profiling_hybrid_qpff_${SLURM_JOB_ID}.map --profile --export-functions=profiling_functions_hybrid_qpff_${SLURM_JOB_ID}.csv mpirun -n ${SLURM_JOB_NUM_NODES} ./main -p mpi -a qpff -s 1048576 4096 

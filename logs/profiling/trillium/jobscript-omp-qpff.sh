#!/bin/bash
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=192
#SBATCH --time=00:15:00
#SBATCH --job-name profiling_omp_qpff_job
#SBATCH --output=profiling_omp_qpff_output_%j.txt
#SBATCH --mail-type=FAIL

cd $SLURM_SUBMIT_DIR

# load required modules
source trilliumsetup

export OMP_NUM_THREADS=40
make clean
make distclean
make WITH_OMP=1
map --output=profiling_omp_qpff_${SLURM_JOB_ID}.map --profile --export-functions=profiling_functions_omp_qpff_${SLURM_JOB_ID}.csv ./main -p omp -a qpff -s 1048576 4096

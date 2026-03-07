#!/bin/bash
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=40
#SBATCH --time=00:15:00
#SBATCH --job-name profiling_serial_fft_job
#SBATCH --output=profiling_serial_fft_output_%j.txt
#SBATCH --mail-type=FAIL

cd $SLURM_SUBMIT_DIR

# load required modules
source trilliumsetup 

make clean
make distclean
make
map --output profiling_serial_fft_${SLURM_JOB_ID}.map --profile --export-functions=profiling_functions_serial_fft_${SLURM_JOB_ID}.csv ./main -p omp -a fft -s 1048576 4096 

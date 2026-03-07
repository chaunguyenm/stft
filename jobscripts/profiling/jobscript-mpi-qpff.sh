#!/bin/bash
#SBATCH --nodes=8
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=192
#SBATCH --time=00:15:00
#SBATCH --job-name profiling_mpi_qpff_job
#SBATCH --output=profiling_mpi_qpff_output_%j.txt
#SBATCH --mail-type=FAIL

cd $SLURM_SUBMIT_DIR

# load required modules
source trilliumsetup

make clean
make distclean
make WITH_MPI=1
# mpirun -n ${SLURM_JOB_NUM_NODES} bash -c "NODE=\$(hostname | cut -d'.' -f1); free -h -s 5 > LOG_${SLURM_JOB_ID}_\${NODE}" &
# mpirun -n ${SLURM_JOB_NUM_NODES} ./main -p mpi -a qpff -o mpi-qpff-output.nc 1048576 4096
map --mpi --output=profiling_mpi_qpff_${SLURM_JOB_ID}.map --profile --export-functions=profiling_functions_mpi_qpff_${SLURM_JOB_ID}.csv mpirun -n ${SLURM_JOB_NUM_NODES} ./main -p mpi -a qpff -s 1048576 4096


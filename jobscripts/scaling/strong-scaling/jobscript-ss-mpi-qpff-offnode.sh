#!/bin/bash
#SBATCH --nodes=8
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=1
#SBATCH --time=00:45:00
#SBATCH --job-name=ss_mpi_qpff
#SBATCH --output=ss_mpi_qpff_output_%j.txt
#SBATCH --mail-type=FAIL

cd $SLURM_SUBMIT_DIR

# Load required modules
source trilliumsetup

# Clean and compile
make clean
make distclean
make WITH_MPI=1

# Set up parameters
NREPEAT=5
NUM_PROCS="1 2 4 8"

# Create output directories
TIMELOG_DIR="timelog_${SLURM_JOB_ID}"
MEMLOG_DIR="memlog_${SLURM_JOB_ID}"
mkdir -p $TIMELOG_DIR $MEMLOG_DIR

for np in $NUM_PROCS; do
    for r in $(seq 1 $NREPEAT); do
        TIMELOG_FILE="${TIMELOG_DIR}/timelog_${np}p_${r}r.txt"

        { time -p srun -N ${np} -n ${np} --ntasks-per-node=1 bash -c '
            NODE=$(hostname -s)
            ./main -p mpi -a qpff -s 1048576 4096 &
            PID=$!

            (
                while kill -0 $PID 2>/dev/null; do
                    ps -p $PID -o etime,pid,%cpu,%mem,rss --no-headers
                    sleep 1
                done
            ) > '"$MEMLOG_DIR"'/memlog_'"$np"'p_'"${r}"'r_${NODE}.txt &
            MEMLOG_PID=$!

            wait $PID
            wait $MEMLOG_PID
        ' 
        } 2> $TIMELOG_FILE

        sleep 2
    done

    sleep 5
done

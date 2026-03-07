#!/bin/bash
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=8
#SBATCH --cpus-per-task=1
#SBATCH --time=00:15:00
#SBATCH --job-name=ws_mpi_fft
#SBATCH --output=ws_mpi_fft_output_%j.txt
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
    WINDOW_SIZE=$(($np * 128))
    NUM_SAMPLES=$((1048576 + ($np - 1) *  128))

    for r in $(seq 1 $NREPEAT); do
        TIMELOG_FILE="${TIMELOG_DIR}/timelog_${np}p_${r}r.txt"
        MEMLOG_FILE="${MEMLOG_DIR}/memlog_${np}p_${r}r.txt"

        mpirun -n $np ./main -p mpi -a fft -s $NUM_SAMPLES $WINDOW_SIZE &
        PID=$!

        (
            while kill -0 $PID 2>/dev/null; do
                ps --ppid $PID -o etime,pid,%cpu,%mem,rss --no-headers
                sleep 1
            done
        ) > $MEMLOG_FILE &

        { time -p wait $PID; } 2> $TIMELOG_FILE

        # Wait before starting next run
        sleep 2
    done

    # Wait before running with new process count
    sleep 5
done

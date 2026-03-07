#!/bin/bash
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=192
#SBATCH --time=00:30:00
#SBATCH --job-name=ws_omp_fft
#SBATCH --output=ws_omp_fft_output_%j.txt
#SBATCH --mail-type=FAIL

cd $SLURM_SUBMIT_DIR

# Load required modules
source trilliumsetup

# Clean and compile
make clean
make distclean
make WITH_OMP=1

# Set up parameters
NREPEAT=5
NUM_THREADS="1 2 4 8 16 32 64 128"

# Create output directories
TIMELOG_DIR="timelog_${SLURM_JOB_ID}"
MEMLOG_DIR="memlog_${SLURM_JOB_ID}"
mkdir -p $TIMELOG_DIR $MEMLOG_DIR

for nt in $NUM_THREADS; do
    export OMP_NUM_THREADS=$nt
    WINDOW_SIZE=$(($nt * 128))
    NUM_SAMPLES=$((1048576 + ($nt - 1) *  128))

    for r in $(seq 1 $NREPEAT); do
        TIMELOG_FILE="${TIMELOG_DIR}/timelog_${nt}t_${r}r.txt"
        MEMLOG_FILE="${MEMLOG_DIR}/memlog_${nt}t_${r}r.txt"

        ./main -p omp -a fft -s $NUM_SAMPLES $WINDOW_SIZE &
        PID=$!
        
        (
            while kill -0 "$PID" 2>/dev/null; do
                ps -p "$PID" -o etime,pid,%cpu,%mem,rss --no-headers
                sleep 1
            done
        ) > $MEMLOG_FILE &

        { time -p wait "$PID"; } 2> $TIMELOG_FILE

        # Wait before starting next run
        sleep 2
    done

    # Wait before running with new process count
    sleep 5
done

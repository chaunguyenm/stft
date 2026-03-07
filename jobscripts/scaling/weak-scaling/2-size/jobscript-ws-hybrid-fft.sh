#!/bin/bash
#SBATCH --nodes=8
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=192
#SBATCH --time=00:30:00
#SBATCH --job-name=ws_hybrid_fft
#SBATCH --output=ws_hybrid_fft_output_%j.txt
#SBATCH --mail-type=FAIL

cd $SLURM_SUBMIT_DIR

# Load required modules
source trilliumsetup

# Clean and compile
make clean
make distclean
make WITH_OMP=1 WITH_MPI=1

# Set up parameters
NREPEAT=5
NUM_PROCS="1 2 4 8"
NUM_THREADS="1 2 4 8 16 32 64 128"

# Create output directories
TIMELOG_DIR="timelog_${SLURM_JOB_ID}"
MEMLOG_DIR="memlog_${SLURM_JOB_ID}"
mkdir -p $TIMELOG_DIR $MEMLOG_DIR

for np in $NUM_PROCS; do
    for nt in $NUM_THREADS; do
        WINDOW_SIZE=$(($np * $nt * 64))
        NUM_SAMPLES=$((131072 + ($np * $nt - 1) *  64))

        export OMP_NUM_THREADS=$nt
        for r in $(seq 1 $NREPEAT); do
            TIMELOG_FILE="${TIMELOG_DIR}/timelog_${np}p_${nt}t_${r}r.txt"

            { time -p srun -N ${np} -n ${np} --ntasks-per-node=1 bash -c '
                NODE=$(hostname -s)
                ./main -p hybrid -a fft -s '"$NUM_SAMPLES"' '"$WINDOW_SIZE"' &
                PID=$!

                (
                    while kill -0 $PID 2>/dev/null; do
                        ps -p $PID -o etime,pid,%cpu,%mem,rss --no-headers
                        sleep 1
                    done
                ) > '"$MEMLOG_DIR"'/memlog_'"$np"'p_'"$nt"'t_'"$r"'r_${NODE}.txt &
                MEMLOG_PID=$!

                wait $PID
                wait $MEMLOG_PID
            ' 
            } 2> $TIMELOG_FILE

            # Wait before starting next run
            sleep 2
        done

        # Wait before running with new thread count
        sleep 5
    done

    # Wait before running with new process count
    sleep 5
done
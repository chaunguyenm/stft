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
    WINDOW_SIZE=4096
    NUM_SAMPLES=$(($nt * 65536 - ($nt - 1) *  $WINDOW_SIZE + ($nt - 1)))

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


# #!/bin/bash
# #SBATCH --nodes=1
# #SBATCH --ntasks-per-node=1
# #SBATCH --cpus-per-task=192
# #SBATCH --time=00:15:00
# #SBATCH --job-name ws_omp_fft_job
# #SBATCH --output=ws_omp_fft_output_%j.txt
# #SBATCH --mail-type=FAIL

# cd $SLURM_SUBMIT_DIR

# # Load required modules
# source trilliumsetup

# # Clean and compile
# make clean
# make distclean
# make WITH_OMP=1

# # Run scaling analysis
# for i in 1 2 4 8 16 32 64 128; do
#     export OMP_NUM_THREADS=$i
#     window_size=4096
#     num_samples=$(($i * 65536 - ($i - 1) *  $window_size + ($i - 1)))
#     # { time -p ./main -p omp -a fft -s $num_samples $window_size; } 2> "output_$i.txt"

#     ./main -p omp -a fft -s $num_samples $window_size &
#     PID=$!
    
#     (
#         while kill -0 "$PID" 2>/dev/null; do
#             ps -p "$PID" -o etime,pid,%cpu,%mem,rss --no-headers
#             sleep 1
#         done
#     ) >> "memlog_${SLURM_JOB_ID}_${i}.txt" &

#     { time -p wait $PID; } 2> "timelog_${SLURM_JOB_ID}_${i}.txt"
# done

# # Prepare runtime output for plotting
# timelogs=$(find . -maxdepth 1 -name "timelog_${SLURM_JOB_ID}_*.txt" -print | sort -t_ -k3 -n)

# for timelog in $timelogs; do
#     nthreads=$(echo $timelog | sed 's/.*_\([0-9]\+\)\.txt/\1/')
#     runtime=$(grep "real" "$timelog" | sed 's/real //')

#     if [ "$nthreads" -eq 1 ]; then
#         runtime1=$runtime
#         echo "$nthreads $runtime 1" > "timelog_${SLURM_JOB_ID}.txt"
#     else
#         speedup=$(echo "$runtime1 / $runtime" | bc -l)
#         echo "$nthreads $runtime $speedup" >> "timelog_${SLURM_JOB_ID}.txt"
#     fi
# done

# rm -f timelog_${SLURM_JOB_ID}_*.txt

# # Prepare memory output for plotting
# # memlogs=$(find . -maxdepth 1 -name "memlog_${SLURM_JOB_ID}_*.txt" -print | sort -t_ -k3 -n)

# # for memlog in $memlogs; do
# #     nthreads=$(echo $memlog | sed -n 's/.*_\([0-9]\+\)t_.*/\1/p')
# #     memused=$(awk '/^Mem:/ { printf "%s ", $3 } END { print "" }' $memlog)
# #     echo "$nthreads $memused" >> "memlog_${SLURM_JOB_ID}.txt"
# # done

# # rm -f memlog_${SLURM_JOB_ID}_*.txt

# mkdir memlog_${SLURM_JOB_ID}
# mv memlog_${SLURM_JOB_ID}_*.txt memlog_${SLURM_JOB_ID}

# # Plot runtime and speedup
# # module load gnuplot
# # gnuplot -e "dir='.'" plot.gnu

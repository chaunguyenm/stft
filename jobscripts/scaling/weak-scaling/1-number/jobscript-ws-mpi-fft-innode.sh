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
    WINDOW_SIZE=4096
    NUM_SAMPLES=$(($np * 32768 - ($np - 1) *  $WINDOW_SIZE + ($np - 1)))

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


# #!/bin/bash
# #SBATCH --nodes=1
# #SBATCH --ntasks-per-node=8
# #SBATCH --cpus-per-task=24
# #SBATCH --time=00:15:00
# #SBATCH --job-name ws_mpi_fft
# #SBATCH --output=ws_mpi_fft_output_%j.txt
# #SBATCH --mail-type=FAIL

# cd $SLURM_SUBMIT_DIR

# # Load required modules
# source trilliumsetup

# # Clean and compile
# make clean
# make distclean
# make WITH_MPI=1

# # Run strong scaling analysis and output memory usage and runtime
# for i in 1 2 4 8; do
#     # free -w -b -s 5 > "memlog_${SLURM_JOB_ID}_${i}.txt" &
#     # free_pid=$!
#     # { time -p mpirun -n "$i" ./main -p mpi -a qpff -s 1048576 4096; } 2> "timelog_${SLURM_JOB_ID}_$i.txt"
#     # kill $free_pid
#     window_size=4096
#     num_samples=$(($i * 32768 - ($i - 1) *  $window_size + ($i - 1)))

#     mpirun -n $i ./main -p mpi -a fft -s $num_samples $window_size &
#     PID=$!

#     (
#         while kill -0 $PID 2>/dev/null; do
#             ps --ppid $PID -o etime,pid,%cpu,%mem,rss --no-headers
#             sleep 1
#         done
#     ) >> memlog_${SLURM_JOB_ID}_${i}.txt &

#     { time -p wait $PID; } 2> timelog_${SLURM_JOB_ID}_${i}.txt
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

# # Plot
# # module load gnuplot
# # gnuplot -e "dir='.'" plot.gnu

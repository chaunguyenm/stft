#!/bin/bash
#SBATCH --nodes=8
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=192
#SBATCH --time=01:15:00
#SBATCH --job-name=ws_hybrid_qpff
#SBATCH --output=ws_hybrid_qpff_output_%j.txt
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
        WINDOW_SIZE=4096
        NUM_SAMPLES=$(($np * $nt * 8192 - ($np * $nt - 1) *  $WINDOW_SIZE + ($np * $nt - 1)))

        export OMP_NUM_THREADS=$nt
        for r in $(seq 1 $NREPEAT); do
            TIMELOG_FILE="${TIMELOG_DIR}/timelog_${np}p_${nt}t_${r}r.txt"

            { time -p srun -N ${np} -n ${np} --ntasks-per-node=1 bash -c '
                NODE=$(hostname -s)
                ./main -p hybrid -a qpff -s '"$NUM_SAMPLES"' '"$WINDOW_SIZE"' &
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


# #!/bin/bash
# #SBATCH --nodes=8
# #SBATCH --ntasks-per-node=1
# #SBATCH --cpus-per-task=192
# #SBATCH --time=00:15:00
# #SBATCH --job-name ws_hybrid_qpff
# #SBATCH --output=ws_hybrid_qpff_output_%j.txt
# #SBATCH --mail-type=FAIL

# cd $SLURM_SUBMIT_DIR

# # Load required modules
# source trilliumsetup

# # Clean and compile
# make clean
# make distclean
# make WITH_MPI=1 WITH_OMP=1

# mkdir -p memlog_${SLURM_JOB_ID}
# mkdir -p timelog_${SLURM_JOB_ID}

# NUM_PROCS="1 2 4 8"
# NUM_THREADS="1 2 4 8 16 32 64 128"

# # Run strong scaling analysis and output memory usage and runtime
# window_size=4096

# for i in $NUM_PROCS; do
#     for j in $NUM_THREADS; do
#         num_samples=$(($i * $j * 8192 - ($i * $j - 1) *  $window_size + ($i * $j - 1)))
#         export OMP_NUM_THREADS=$j
#         { time -p srun --nodes=${i} --ntasks-per-node=1 bash -c '
#             NODE=$(hostname -s)
#             ./main -p mpi -a qpff -s '"$num_samples"' '"$window_size"' &
#             PID=$!

#             (
#                 while kill -0 $PID 2>/dev/null; do
#                     ps -p $PID -o etime,pid,%cpu,%mem,rss --no-headers
#                     sleep 1
#                 done
#             ) >> memlog_${SLURM_JOB_ID}/memlog_${SLURM_JOB_ID}_${NODE}_'"$i"'p_'"$j"'t.txt &
#             PS_PID=$!

#             wait $PID
#             wait $PS_PID
#         ' 
#         } 2> timelog_${SLURM_JOB_ID}/timelog_${SLURM_JOB_ID}_${i}p_${j}t.txt
#     done
# done

# # Prepare runtime output for plotting
# cd timelog_${SLURM_JOB_ID}
# for i in $NUM_PROCS; do
#     timelogs=$(find . -maxdepth 1 -name "timelog_${SLURM_JOB_ID}_${i}p_*t.txt" -print | sort -t_ -k4,4 -n)

#     for timelog in $timelogs; do
#         nthreads=$(echo $timelog | sed 's/.*_\([0-9]\+\)p_\([0-9]\+\)t\.txt/\2/')
#         runtime=$(grep "real" "$timelog" | sed 's/real //')

#         if [ "$nthreads" -eq 1 ]; then
#             runtime1=$runtime
#             echo "$nthreads $runtime 1" > "timelog_${SLURM_JOB_ID}_${i}p.txt"
#         else
#             speedup=$(echo "$runtime1 / $runtime" | bc -l)
#             echo "$nthreads $runtime $speedup" >> "timelog_${SLURM_JOB_ID}_${i}p.txt"
#         fi
#     done
# done

# for i in $NUM_PROCS; do    
#     while read p; do
#         echo "$i $p" >> "timelog_${SLURM_JOB_ID}.txt"
#     done <"timelog_${SLURM_JOB_ID}_${i}p.txt"
# done

# # rm -f timelog_${SLURM_JOB_ID}_*.txt

# # Prepare memory output for plotting
# # NODES="$(scontrol show hostnames "$SLURM_JOB_NODELIST")"
# # for NODE in $NODES; do
# #     memlogs=$(find . -maxdepth 1 -name "memlog_${SLURM_JOB_ID}_${NODE}_*.txt" -print | sort -t_ -k4 -n)
# #     for memlog in $memlogs; do
# #         nthreads=$(echo $memlog | sed -n 's/.*_\([0-9]\+\)t_.*/\1/p')
# #         memused=$(awk '/^Mem:/ { printf "%s ", $3 } END { print "" }' $memlog)
# #         echo "$nthreads $memused" >> "memlog_${SLURM_JOB_ID}_${NODE}.txt"
# #     done

# #     rm -f memlog_${SLURM_JOB_ID}_${NODE}_*.txt
# # done

# # Plot runtime and speedup
# # module load gnuplot
# # gnuplot -e "dir='.'" plot.gnu

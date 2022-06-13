#! /usr/bin/env bash 

declare -a LOOP=(0 1 2 3 4 5 6 7 8 9 10)

sysctl -w kernel.sched_rt_runtime_us=-1 

TEST="disparity"
SIZE="250000"

sudo insmod ../../patched_memguard/memguard.ko g_hw_counter_id=0x17
sleep 2

mkdir -p res/${TEST}

sudo taskset -c 2 ../../benchmarks/hesoc-mark/cuda/cudainterf -d $SIZE -s --iterations=200055 --mode=memset | grep "Memset BW" | awk '{ print $4 }'  >> bw_memset.txt & PID_TO_KILL0=$!
sleep 5

PID_TO_KILL=$(pgrep cudainterf)

# sudo echo 0 > /sys/kernel/debug/tracing/trace
sudo kill -s SIGUSR1 $PID_TO_KILL


CPU=4
GPU=19
 
# CPU
sudo /bin/busybox devmem 0x70019320 32 0x${CPU}

# GPU
sudo /bin/busybox devmem 0x700193ac 32 0x${GPU}
sudo /bin/busybox devmem 0x700193e8 32 0x${GPU}

sudo taskset -c 0 ../../benchmarks/sd-vbs/benchmarks/${TEST}/data/fullhd/${TEST} ../../benchmarks/sd-vbs/benchmarks/${TEST}/data/fullhd/. | grep "Cycles elapsed" >> cycles_CPU_${CPU}_GPU_${GPU}.txt & PID_TO_WAIT=$! 
wait $PID_TO_WAIT

# sudo cat /sys/kernel/debug/tracing/trace > trace_${INTERF}.txt

sudo kill -s SIGUSR2 $PID_TO_KILL
wait $PID_TO_KILL0

sleep 2

sudo kill -9 $PID_TO_KILL > /dev/null 2>&1
sudo killall -9 cudainterf > /dev/null 2>&1

# python3 splitftrace.py trace_${INTERF}.txt


mv *.txt res/${TEST}


# sudo rmmod memguard

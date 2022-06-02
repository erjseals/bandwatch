#! /usr/bin/env bash 

SIZE="250000"

mkdir -p res/hesoc_isolated

sysctl -w kernel.sched_rt_runtime_us=-1 
	
sudo insmod ../../patched_memguard/memguard.ko g_hw_counter_id=0x17
sleep 2

sudo echo 0 > /sys/kernel/debug/memguard/throttle
sleep 2

sudo taskset -c 2 ../../benchmarks/hesoc-mark/cuda/cudainterf -s -d $SIZE --iterations=20055 --mode=memset & PID_TO_KILL0=$!

PID_TO_KILL=$(pgrep cudainterf)
sleep .5

sudo echo 0 > /sys/kernel/debug/tracing/trace

sleep 5
sudo cat /sys/kernel/debug/tracing/trace > trace_memset.txt
sudo rmmod memguard

# Attempt clean close
sudo kill -s SIGUSR2 $PID_TO_KILL
sleep 1

sudo kill -9 $PID_TO_KILL

python3 splitftrace.py trace_memset.txt

echo "done"
sleep 10





sudo insmod ../../patched_memguard/memguard.ko g_hw_counter_id=0x17
sleep 2

sudo echo 0 > /sys/kernel/debug/memguard/throttle
sleep 2

sudo taskset -c 2 ../../benchmarks/hesoc-mark/cuda/cudainterf -s -d $SIZE --iterations=20055 --mode=memcpy & PID_TO_KILL0=$!

PID_TO_KILL=$(pgrep cudainterf)
sleep .5

sudo echo 0 > /sys/kernel/debug/tracing/trace

sleep 5
sudo cat /sys/kernel/debug/tracing/trace > trace_memcpy.txt
sudo rmmod memguard

# Attempt clean close
sudo kill -s SIGUSR2 $PID_TO_KILL
sleep 1

sudo kill -9 $PID_TO_KILL

python3 splitftrace.py trace_memcpy.txt


mv *.txt res/hesoc_isolated
echo "done"

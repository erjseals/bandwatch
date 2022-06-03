#! /usr/bin/env bash 

SIZE="250000"

mkdir -p res/hesoc_isolated

sysctl -w kernel.sched_rt_runtime_us=-1 
	
sudo insmod ../../patched_memguard/memguard.ko g_hw_counter_id=0x17
sleep 2

sudo echo 0 > /sys/kernel/debug/memguard/throttle
sleep 2

sudo taskset -c 2 ../../benchmarks/hesoc-mark/cuda/cudainterf -s -d $SIZE --iterations=20055 --mode=memset & PID_TO_KILL0=$!

sleep 3
PID_TO_KILL=$(pgrep cudainterf)

sudo kill -s SIGUSR1 $PID_TO_KILL

sudo echo 0 > /sys/kernel/debug/tracing/trace

sleep 5
sudo cat /sys/kernel/debug/tracing/trace > trace_memset_sync.txt
sudo kill -s SIGUSR2 $PID_TO_KILL

sudo rmmod memguard

sudo kill -9 $PID_TO_KILL
sudo killall -9 cudainterf

python3 splitftrace.py trace_memset_sync.txt

sleep 10


sudo insmod ../../patched_memguard/memguard.ko g_hw_counter_id=0x17
sleep 2

sudo echo 0 > /sys/kernel/debug/memguard/throttle
sleep 2

sudo taskset -c 2 ../../benchmarks/hesoc-mark/cuda/cudainterf -s -d $SIZE --iterations=20055 --mode=memcpy & PID_TO_KILL0=$!

sleep 3
PID_TO_KILL=$(pgrep cudainterf)

sudo kill -s SIGUSR1 $PID_TO_KILL
sudo echo 0 > /sys/kernel/debug/tracing/trace

sleep 5
sudo cat /sys/kernel/debug/tracing/trace > trace_memcpy_sync.txt
sudo kill -s SIGUSR2 $PID_TO_KILL
sudo rmmod memguard

sudo kill -9 $PID_TO_KILL
sudo killall -9 cudainterf

python3 splitftrace.py trace_memcpy_sync.txt


sudo insmod ../../patched_memguard/memguard.ko g_hw_counter_id=0x17
sleep 2

sudo echo 0 > /sys/kernel/debug/memguard/throttle
sleep 2

sudo taskset -c 2 ../../benchmarks/hesoc-mark/cuda/cudainterf -d $SIZE --iterations=20055 --mode=memset & PID_TO_KILL0=$!

sleep 3
PID_TO_KILL=$(pgrep cudainterf)

sudo echo 0 > /sys/kernel/debug/tracing/trace

sleep 5
sudo cat /sys/kernel/debug/tracing/trace > trace_memset.txt
sudo rmmod memguard

sudo kill -9 $PID_TO_KILL
sudo killall -9 cudainterf

python3 splitftrace.py trace_memset.txt

sleep 10


sudo insmod ../../patched_memguard/memguard.ko g_hw_counter_id=0x17
sleep 2

sudo echo 0 > /sys/kernel/debug/memguard/throttle
sleep 2

sudo taskset -c 2 ../../benchmarks/hesoc-mark/cuda/cudainterf -d $SIZE --iterations=20055 --mode=memcpy & PID_TO_KILL0=$!

sleep 3
PID_TO_KILL=$(pgrep cudainterf)

sudo echo 0 > /sys/kernel/debug/tracing/trace

sleep 5
sudo cat /sys/kernel/debug/tracing/trace > trace_memcpy.txt
sudo rmmod memguard

sudo kill -9 $PID_TO_KILL
sudo killall -9 cudainterf

python3 splitftrace.py trace_memcpy.txt


mv *.txt res/hesoc_isolated

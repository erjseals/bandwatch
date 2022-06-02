#! /usr/bin/env bash 

sysctl -w kernel.sched_rt_runtime_us=-1 

TEST="disparity"
SIZE="250000"

mkdir -p res/${TEST}/memcpy

sudo insmod ../../patched_memguard/memguard.ko g_hw_counter_id=0x17
sleep 2

sudo echo 0 > /sys/kernel/debug/memguard/throttle
sleep 2

sudo taskset -c 2 ../../benchmarks/hesoc-mark/cuda/cudainterf -d $SIZE -s --iterations=20055 --mode=memcpy &
sleep .5

PID_TO_KILL=$(pgrep cudainterf)

sudo kill -s SIGUSR1 $PID_TO_KILL
sudo echo 0 > /sys/kernel/debug/tracing/trace

sudo taskset -c 0 ../../benchmarks/sd-vbs/benchmarks/${TEST}/data/fullhd/${TEST} ../../benchmarks/sd-vbs/benchmarks/${TEST}/data/fullhd/. | grep "Cycles elapsed" >> cycles_dynamic_memcpy.txt & PID_TO_WAIT=$! 
wait $PID_TO_WAIT

sudo cat /sys/kernel/debug/tracing/trace > trace_dynamic_memcpy.txt
sudo kill -s SIGUSR2 $PID_TO_KILL

sudo rmmod memguard

sudo kill -9 $PID_TO_KILL

python3 splitftrace.py trace_dynamic_memcpy.txt

mv *.txt res/${TEST}/memcpy

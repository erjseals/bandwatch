#! /usr/bin/env bash 

sysctl -w kernel.sched_rt_runtime_us=-1 

TEST="disparity"

mkdir -p res/${TEST}/memset
mkdir -p res/${TEST}/memcpy

	
sudo insmod ../../patched_memguard/memguard.ko g_hw_counter_id=0x17
sleep 2

sudo echo 0 > /sys/kernel/debug/memguard/throttle
sleep 2

sudo taskset -c 2 ../../benchmarks/hesoc-mark/cuda/cudainterf -d 102400 --iterations=20055 --mode=memset & PID_TO_KILL0=$!
sleep .5

sudo kill -10 $PID_TO_KILL0

sudo echo 0 > /sys/kernel/debug/tracing/trace

sudo taskset -c 0 ../../benchmarks/sd-vbs/benchmarks/${TEST}/data/fullhd/${TEST} ../../benchmarks/sd-vbs/benchmarks/${TEST}/data/fullhd/. | grep "Cycles elapsed" >> cycles_dynamic_memset.txt & PID_TO_WAIT=$! 
wait $PID_TO_WAIT

sudo cat /sys/kernel/debug/tracing/trace > trace_dynamic_memset.txt

sudo kill -12 $PID_TO_KILL0

sudo rmmod memguard

sleep 2

sudo kill -9 $PID_TO_KILL0 
sudo killall -q cudainterf

python3 splitftrace.py trace_dynamic_memset.txt

mv *.txt res/${TEST}/memset 


echo "Delay, let system refresh"
sleep 10


sudo insmod ../../patched_memguard/memguard.ko g_hw_counter_id=0x17
sleep 2

sudo echo 0 > /sys/kernel/debug/memguard/throttle
sleep 2

sudo taskset -c 2 ../../benchmarks/hesoc-mark/cuda/cudainterf -d 102400 --iterations=20055 --mode=memcpy & PID_TO_KILL0=$!
sleep .5

sudo echo 0 > /sys/kernel/debug/tracing/trace

sudo taskset -c 0 ../../benchmarks/sd-vbs/benchmarks/${TEST}/data/fullhd/${TEST} ../../benchmarks/sd-vbs/benchmarks/${TEST}/data/fullhd/. | grep "Cycles elapsed" >> cycles_dynamic_memcpy.txt & PID_TO_WAIT=$! 
wait $PID_TO_WAIT
sudo cat /sys/kernel/debug/tracing/trace > trace_dynamic_memcpy.txt
sudo rmmod memguard

sudo kill -9 $PID_TO_KILL0 
sudo killall -q cudainterf

python3 splitftrace.py trace_dynamic_memcpy.txt

mv *.txt res/${TEST}/memcpy

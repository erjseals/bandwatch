#! /usr/bin/env bash 

sysctl -w kernel.sched_rt_runtime_us=-1 

# (cd ../patched_memguard && make)


	
sudo insmod ../patched_memguard/memguard.ko g_hw_counter_id=0x17
sleep 2

sudo echo 0 > /sys/kernel/debug/memguard/throttle
sleep 2

sudo taskset -c 2 ../benchmarks/hesoc-mark/cuda/cudainterf -d 102400 --iterations=20055 --mode=memset & PID_TO_KILL0=$!
sleep .5

sudo echo 0 > /sys/kernel/debug/tracing/trace

sleep 5
sudo cat /sys/kernel/debug/tracing/trace > trace_memset.txt
sudo rmmod memguard

sudo kill -9 $PID_TO_KILL0 
sudo killall -q cudainterf

python3 splitftrace.py trace_memset.txt

echo "done"
sleep 10





sudo insmod ../patched_memguard/memguard.ko g_hw_counter_id=0x17
sleep 2

sudo echo 0 > /sys/kernel/debug/memguard/throttle
sleep 2

sudo taskset -c 2 ../benchmarks/hesoc-mark/cuda/cudainterf -d 102400 --iterations=20055 --mode=memcpy & PID_TO_KILL0=$!
sleep .5

sudo echo 0 > /sys/kernel/debug/tracing/trace

sleep 5
sudo cat /sys/kernel/debug/tracing/trace > trace_memcpy.txt
sudo rmmod memguard

sudo kill -9 $PID_TO_KILL0 
sudo killall -q cudainterf

python3 splitftrace.py trace_memcpy.txt


mv *.txt tests_hesocIsol
echo "done"

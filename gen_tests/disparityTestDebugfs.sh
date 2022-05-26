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

sudo taskset -c 0 ../benchmarks/sd-vbs/benchmarks/disparity/data/fullhd/disparity ../benchmarks/sd-vbs/benchmarks/disparity/data/fullhd/. | grep "Cycles elapsed" >> cycles_dynamic_memset.txt & PID_TO_WAIT=$! 
wait $PID_TO_WAIT
sudo cat /sys/kernel/debug/tracing/trace > trace_dynamic_memset.txt
sudo rmmod memguard

sudo kill -9 $PID_TO_KILL0 
sudo killall -q cudainterf

python3 splitftraceThrottle.py trace_dynamic_memset.txt

echo "done"


mv *.txt tests_dynamic 
echo "done"

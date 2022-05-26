#! /usr/bin/env bash 

sysctl -w kernel.sched_rt_runtime_us=-1 


	
sudo insmod ../patched_memguard/memguard.ko g_hw_counter_id=0x17
sleep 2

#sudo echo 0 > /sys/kernel/debug/memguard/throttle
#sleep 2

sudo echo 0 > /sys/kernel/debug/tracing/trace
sudo taskset -c 2 ../benchmarks/hesoc-mark/cuda/cudameasure 51200000 50 dryrun & PID_TO_KILL0=$!

sudo taskset -c 0 ../benchmarks/sd-vbs/benchmarks/disparity/data/fullhd/disparity ../benchmarks/sd-vbs/benchmarks/disparity/data/fullhd/. | grep "Cycles elapsed" >> cycles_GPU_perf.txt & PID_TO_WAIT=$! 

wait $PID_TO_WAIT
wait $PID_TO_KILL0

sudo cat /sys/kernel/debug/tracing/trace > trace_GPU_perf.txt
sudo rmmod memguard

#sudo kill -9 $PID_TO_KILL0 
#sudo killall -q cudainterf

python3 splitftrace.py trace_GPU_perf.txt

mv *.txt tests_dynamic 
echo "done"

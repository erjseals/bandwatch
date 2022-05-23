#! /usr/bin/env bash 

sysctl -w kernel.sched_rt_runtime_us=-1 

sudo echo 0 > /sys/kernel/debug/tracing/trace

sudo taskset -c 0   ../../membench/membench | grep HESOCMARK & PID_TO_WAIT=$! 
wait $PID_TO_WAIT

sudo cat /sys/kernel/debug/tracing/trace

echo "done"

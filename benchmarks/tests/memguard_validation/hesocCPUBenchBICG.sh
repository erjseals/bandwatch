#!/bin/bash

sysctl -w kernel.sched_rt_runtime_us=-1

sudo echo 0 > /sys/kernel/debug/tracing/trace

sudo taskset -c 0 ~/dev/hesoc-mark/cpubench/cpubench -v --datacount=8192 --iterations=50 --test=BICG & PID_TO_WAIT=$!
wait $PID_TO_WAIT

sudo rmmod memguard



# With configuration `echo mb 400 100 100 100 > limit`
# 
#
#

#!/bin/bash

sysctl -w kernel.sched_rt_runtime_us=-1

#sudo taskset -c 1 ~/dev/hesoc-mark/membench/meminterf -v --size=156 --iterations=855 --test=memset & PID_TO_WAIT=$!
#sudo taskset -c 2 ~/dev/hesoc-mark/membench/meminterf -v --size=156 --iterations=855 --test=memset & PID_TO_WAIT=$!
#sudo taskset -c 3 ~/dev/hesoc-mark/membench/meminterf -v --size=156 --iterations=855 --test=memset & PID_TO_WAIT=$!

sudo taskset -c 0 ~/dev/hesoc-mark/membench/membench >> oldenabledNoInterf.txt & PID_TO_WAIT=$!
wait $PID_TO_WAIT



# With configuration `echo mb 400 100 100 100 > limit`
# 
#
#

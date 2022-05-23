#!/bin/bash

sysctl -w kernel.sched_rt_runtime_us=-1

sudo taskset -c 3 ./cudainterf -v -d 102400 --iterations=20055 --test=memset & PID_TO_WAIT3=$!
# sudo taskset -c 2 ./../membench/meminterf -v --size=156 --iterations=855 --test=memset & PID_TO_WAIT2=$!
# sudo taskset -c 0 ./../membench/membench -r & PID_TO_WAIT3=$!

while ps -p $PID_TO_WAIT3 > /dev/null ; do
	sleep .5
done;

# killall -9 meminterf
killall -9 cudainterf

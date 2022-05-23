#!/bin/bash

sysctl -w kernel.sched_rt_runtime_us=-1

sudo taskset -c 0 ./cudainterf -v -d 102400 --iterations=2055 --test=memset & PID_TO_WAIT=$!
sudo taskset -c 1 ../../membench/membench 

sleep .1
echo 1 > /sys/kernel/debug/actmon/actmon
sleep .1

while ps -p $PID_TO_WAIT > /dev/null ; do
	echo 1 > /sys/kernel/debug/actmon/mcall_count
	cat /sys/kernel/debug/actmon/MCALL_AVG 
	sleep .1
done;


#!/bin/bash

sysctl -w kernel.sched_rt_runtime_us=-1

sudo taskset -c 0 ./meminterf -v --size=156 --iterations=855 --test=memset & PID_TO_WAIT=$!
sudo taskset -c 2 ./meminterf -v --size=156 --iterations=855 --test=memset & PID_TO_WAIT=$!

sleep .1

while ps -p $PID_TO_WAIT > /dev/null ; do
	sleep .5
done;


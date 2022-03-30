#!/bin/bash

sysctl -w kernel.sched_rt_runtime_us=-1

sleep .1

while true ; do 
	echo 1 > /sys/kernel/debug/actmon/actmon
	cat /sys/kernel/debug/actmon/MCALL_AVG 
	sleep .1
done;


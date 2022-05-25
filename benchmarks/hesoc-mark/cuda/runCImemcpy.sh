#!/bin/bash

sysctl -w kernel.sched_rt_runtime_us=-1

sudo taskset -c 0 ./cudainterf -v -d 102400 --iterations=20055 --mode=memcpy & PID_TO_WAIT=$!

#! /usr/bin/env bash 

sysctl -w kernel.sched_rt_runtime_us=-1 

sudo taskset -c 2 ../../benchmarks/hesoc-mark/cuda/cudainterf -d 102400 --iterations=8055 --mode=memset &
sleep .5

PID_TO_KILL=$(pgrep cudainterf)
echo $PID_TO_KILL

sudo kill -s SIGUSR1 $PID_TO_KILL

sleep .5

sudo kill -s SIGUSR2 $PID_TO_KILL

sleep .5
sudo kill -9 $PID_TO_KILL
#sudo killall -q cudainterf

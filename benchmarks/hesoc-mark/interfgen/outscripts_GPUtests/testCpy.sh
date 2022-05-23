#! /usr/bin/env bash 

sysctl -w kernel.sched_rt_runtime_us=-1 

sudo taskset -c 2 ../../cuda/cudainterf -v -d 102400 --iterations=2055 --mode=memcpy & PID_TO_KILL0=$!
sleep 1
sudo taskset -c 0   ../../membench/membench  | grep HESOCMARK & PID_TO_WAIT=$! 
wait $PID_TO_WAIT

echo "done"
sudo kill -9 $PID_TO_KILL0 
sudo killall -q cudainterf

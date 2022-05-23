#! /usr/bin/env bash 

sleep 5
sysctl -w kernel.sched_rt_runtime_us=-1 

sudo taskset -c 2 ../../cuda/cudainterf -v -d 102400 --iterations=2055 --mode=memset & PID_TO_KILL0=$!
sleep 1 

# Random Accesses
#sudo taskset -c 0 ../../membench/membench -r & PID_TO_WAIT=$! 

# Sequential Accesses
# sudo taskset -c 0 ../../membench/membench & PID_TO_WAIT=$! 

# wait $PID_TO_WAIT

wait $PID_TO_KILL0

sudo kill -9 $PID_TO_KILL0 
sudo killall -q cudainterf

echo "done"

#! /usr/bin/env bash 

sleep 5

sysctl -w kernel.sched_rt_runtime_us=-1 

for i in {1..1} 
do 
sudo taskset -c 1 ../../membench/meminterf -v --size=128 --test=memcpy --iterations=10000 & PID_TO_KILL0=$!
sudo taskset -c 2 ../../membench/meminterf -v --size=128 --test=memcpy --iterations=10000 & PID_TO_KILL1=$!
sudo taskset -c 3 ../../membench/meminterf -v --size=128 --test=memcpy --iterations=10000 & PID_TO_KILL2=$!
sleep 10 
sudo taskset -c 0   ../../cuda/cudameasure 51200000 50 dryrun  >> t1-GPU_CPUs_CPUs_CPUs_.sh_$i.txt &  PID_TO_WAIT=$! 
wait $PID_TO_WAIT

echo "done"
sudo kill -9 $PID_TO_KILL0 
sudo kill -9 $PID_TO_KILL1 
sudo kill -9 $PID_TO_KILL2 
sudo killall -q meminterf

wait 
done 

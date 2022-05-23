#! /usr/bin/env bash 

declare -a THROTTLE_AMOUNT=(0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31)

sleep 5
sysctl -w kernel.sched_rt_runtime_us=-1 

for i in "${THROTTLE_AMOUNT[@]}"
do 
	echo "Throttle Amount ${i}"

	sudo taskset -c 2 ../../cuda/cudainterf -v -d 102400 --iterations=20055 --mode=memset & PID_TO_KILL0=$!
	sleep 1 
	sudo echo ${i} > /sys/kernel/debug/throttle/throttle
	sudo taskset -c 0   ../../membench/membench -r | grep HESOCMARK >>  rnd/rnd_$i.txt &  PID_TO_WAIT=$! 
	wait $PID_TO_WAIT

	echo "done"
	sudo kill -9 $PID_TO_KILL0 
	sudo killall -q cudainterf

done 

sudo echo 0 > /sys/kernel/debug/throttle/throttle
echo "done"

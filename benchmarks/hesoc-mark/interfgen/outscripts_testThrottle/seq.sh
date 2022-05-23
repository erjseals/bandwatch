#! /usr/bin/env bash 

declare -a THROTTLE_AMOUNT=(0 1 2 3 4 5 6 7 8)

sleep 1
sysctl -w kernel.sched_rt_runtime_us=-1 

for i in  "${THROTTLE_AMOUNT[@]}"
do 
	echo "Throttle Amount ${i}"

	sudo echo ${i} > /sys/kernel/debug/bandWatch/throttle
	sleep 1
	sudo taskset -c 2 ../../cuda/cudameasure 51200000 50 dryrun & PID_TO_WAIT0=$!
	# sudo taskset -c 0   ../../membench/membench  | grep HESOCMARK >> seq/seq_$i.txt & PID_TO_WAIT=$! 
	wait $PID_TO_WAIT

	echo "done"
	sudo kill -9 $PID_TO_KILL0 
	sudo killall -q cudainterf

done 

sudo echo 0 > /sys/kernel/debug/throttle/throttle
echo "done"

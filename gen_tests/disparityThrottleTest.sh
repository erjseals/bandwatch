#! /usr/bin/env bash 

declare -a THROTTLE_AMOUNT=(0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16)

sysctl -w kernel.sched_rt_runtime_us=-1 

for i in  "${THROTTLE_AMOUNT[@]}"
do 
	echo "Throttle Amount ${i}"

	sudo taskset -c 2 ../benchmarks/hesoc-mark/cuda/cudainterf -d 102400 --iterations=2055 --mode=memset & PID_TO_KILL0=$!
	sleep 1 
	sudo echo ${i} > /sys/kernel/debug/memguard/throttle
	sleep 1
	sudo taskset -c 0 ../benchmarks/sd-vbs/benchmarks/disparity/data/fullhd/disparity ../benchmarks/sd-vbs/benchmarks/disparity/data/fullhd/. | grep "Cycles elapsed" >> cycles_$i.txt & PID_TO_WAIT=$! 
	wait $PID_TO_WAIT

	echo "done"
	sudo kill -9 $PID_TO_KILL0 
	sudo killall -q cudainterf

done 

sudo echo 0 > /sys/kernel/debug/memguard/throttle
echo "done"

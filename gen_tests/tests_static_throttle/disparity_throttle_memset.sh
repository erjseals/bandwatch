#! /usr/bin/env bash 

declare -a THROTTLE_AMOUNT=(0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16)

TEST="disparity"

mkdir -p res 
mkdir -p res/${TEST}/memset

sysctl -w kernel.sched_rt_runtime_us=-1 

for i in  "${THROTTLE_AMOUNT[@]}"
do 
	
	sudo insmod ../../patched_memguard/memguard.ko g_hw_counter_id=0x17
	sleep 2

	echo "Memset, Throttle Amount ${i}"
	sudo echo ${i} > /sys/kernel/debug/memguard/throttle
	sleep 2

	sudo taskset -c 2 ../../benchmarks/hesoc-mark/cuda/cudainterf -d 102400 -s --iterations=20055 --mode=memset & PID_TO_KILL0=$!
	sleep .5

  PID_TO_KILL=$(pgrep cudainterf)

  sudo kill -s SIGUSR1 $PID_TO_KILL
	sudo echo 0 > /sys/kernel/debug/tracing/trace

	sudo taskset -c 0 ../../benchmarks/sd-vbs/benchmarks/${TEST}/data/fullhd/${TEST} ../../benchmarks/sd-vbs/benchmarks/${TEST}/data/fullhd/. | grep "Cycles elapsed" >> cycles_memset.txt & PID_TO_WAIT=$! 
	wait $PID_TO_WAIT

	sudo cat /sys/kernel/debug/tracing/trace > trace_memset_$i.txt
  sudo kill -s SIGUSR2 $PID_TO_KILL

	sudo rmmod memguard

	sudo kill -9 $PID_TO_KILL

	python3 splitftrace_static_throttle.py trace_memset_$i.txt

	echo "done"
	sleep 10
done 

mv *.txt res/${TEST}/memset

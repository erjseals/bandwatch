#! /usr/bin/env bash 

declare -a THROTTLE_AMOUNT=(0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31)

sleep 5
sysctl -w kernel.sched_rt_runtime_us=-1 

for i in "${THROTTLE_AMOUNT[@]}"
do
	sudo echo ${i} > /sys/kernel/debug/throttle/throttle
	sleep 1
	sudo taskset -c 0 ../../cuda/cudameasure 51200000 50 dryrun >> base/throttle${i}.txt
	wait $PID_TO_WAIT
done

sudo echo 0 > /sys/kernel/debug/throttle/throttle
echo "done"

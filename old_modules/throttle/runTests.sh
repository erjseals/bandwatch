#!/bin/bash

sysctl -w kernel.sched_rt_runtime_us=-1

for throttle in {0..11..1}
do
	rm dir/throttle${throttle}.txt
	echo $throttle > /sys/kernel/debug/throttle/throttle
	sleep 1
	sudo taskset -c 0 ~/dev/hesoc-mark/cuda/cudameasure 51200000 50 dryrun >> dir/throttle${throttle}.txt & PID_TO_WAIT=$!
	wait $PID_TO_WAIT
	echo "Throttle ${throttle} complete"
done

sudo echo 0 > /sys/kernel/debug/throttle/throttle
echo All done

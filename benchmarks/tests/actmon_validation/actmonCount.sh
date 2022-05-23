#!/bin/bash

#
# With configuration `echo mb 400 100 100 100 > limit`
#

sysctl -w kernel.sched_rt_runtime_us=-1

sudo taskset -c 1 ~/dev/hesoc-mark/membench/meminterf -v --size=156 --iterations=8550 --test=memset & PID_TO_KILL1=$!
sudo taskset -c 2 ~/dev/hesoc-mark/membench/meminterf -v --size=156 --iterations=8550 --test=memset & PID_TO_KILL2=$!
sudo taskset -c 3 ~/dev/hesoc-mark/membench/meminterf -v --size=156 --iterations=8550 --test=memset & PID_TO_KILL3=$!

sudo taskset -c 0 ~/dev/hesoc-mark/membench/membench >> fullInterf.txt & PID_TO_WAIT=$!

wait $PID_TO_WAIT
sudo kill $PID_TO_KILL1
sudo kill $PID_TO_KILL2
sudo kill $PID_TO_KILL3
sudo killall -9 meminterf

sleep 2

sudo taskset -c 2 ~/dev/hesoc-mark/membench/meminterf -v --size=156 --iterations=8550 --test=memset & PID_TO_KILL4=$!
sudo taskset -c 3 ~/dev/hesoc-mark/membench/meminterf -v --size=156 --iterations=8550 --test=memset & PID_TO_KILL5=$!

sudo taskset -c 0 ~/dev/hesoc-mark/membench/membench >> noL1Interf.txt & PID_TO_WAIT1=$!

wait $PID_TO_WAIT1
sudo kill $PID_TO_KILL4
sudo kill $PID_TO_KILL5
sudo killall -9 meminterf

sleep 2

sudo taskset -c 1 ~/dev/hesoc-mark/membench/meminterf -v --size=156 --iterations=8550 --test=memset & PID_TO_KILL6=$!

sudo taskset -c 0 ~/dev/hesoc-mark/membench/membench >> L1Interf.txt & PID_TO_WAIT2=$!

wait $PID_TO_WAIT2
sudo kill $PID_TO_KILL6
sudo killall -9 meminterf

sleep 2

sudo taskset -c 0 ~/dev/hesoc-mark/membench/membench >> noInterf.txt & PID_TO_WAIT3=$!
wait $PID_TO_WAIT3


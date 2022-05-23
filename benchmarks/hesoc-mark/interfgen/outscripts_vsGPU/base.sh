#! /usr/bin/env bash 

sysctl -w kernel.sched_rt_runtime_us=-1 

sudo taskset -c 0 ../../cuda/cudameasure 51200000 50 dryrun >> base.txt

echo "done"

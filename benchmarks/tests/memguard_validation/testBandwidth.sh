#!/bin/bash

sudo echo 0 > /sys/kernel/debug/tracing/trace

sudo bandwidth & PID_TO_WAIT=$!
wait $PID_TO_WAIT

sudo rmmod memguard

# With configuration `echo mb 400 100 100 100 > limit`
# 
#
#

#!/bin/bash

echo 511 > /sys/kernel/debug/throttle/limit
echo 0 > /sys/kernel/debug/throttle/throttle


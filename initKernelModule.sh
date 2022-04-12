#!/bin/bash

echo 511 > /sys/kernel/debug/bandWatch/limit
echo 0 > /sys/kernel/debug/bandWatch/throttle

echo 1 > /sys/kernel/debug/bandWatch/actmon


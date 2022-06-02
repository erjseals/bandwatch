#! /usr/bin/env bash 

TEST="bandwidth"
SIZE="250000"
INTERF=" "

mkdir -p res/${TEST}_baselines

sysctl -w kernel.sched_rt_runtime_us=-1

TEST="bandwidth0"
INTERF="isolated"
echo "$TEST $INTERF"

sudo insmod ../../patched_memguard/memguard.ko g_hw_counter_id=0x17
sleep 2

sudo echo 0 > /sys/kernel/debug/tracing/trace

bandwidth -c 0 -t 1000 &
sleep 5

sudo cat /sys/kernel/debug/tracing/trace > trace_${TEST}_${INTERF}.txt 
sudo killall -9 bandwidth
sudo rmmod memguard

python3 splitftrace.py trace_${TEST}_${INTERF}.txt

sleep 10




TEST="bandwidth123"
INTERF="isolated"
echo "$TEST $INTERF"

sudo insmod ../../patched_memguard/memguard.ko g_hw_counter_id=0x17
sleep 2

sudo echo 0 > /sys/kernel/debug/tracing/trace

for c in 1 2 3; do bandwidth -c $c -t 1000 & done;
sleep 5

sudo cat /sys/kernel/debug/tracing/trace > trace_${TEST}_${INTERF}.txt 
sudo killall -9 bandwidth
sudo rmmod memguard

python3 splitftrace.py trace_${TEST}_${INTERF}.txt

sleep 10




TEST="bandwidth01"
INTERF="isolated"
echo "$TEST $INTERF"

sudo insmod ../../patched_memguard/memguard.ko g_hw_counter_id=0x17
sleep 2

sudo echo 0 > /sys/kernel/debug/tracing/trace

for c in 0 1; do bandwidth -c $c -t 1000 & done;
sleep 5

sudo cat /sys/kernel/debug/tracing/trace > trace_${TEST}_${INTERF}.txt 
sudo killall -9 bandwidth
sudo rmmod memguard

python3 splitftrace.py trace_${TEST}_${INTERF}.txt

sleep 10




TEST="bandwidth0"
INTERF="memcpy"
echo "$TEST against $INTERF"

sudo insmod ../../patched_memguard/memguard.ko g_hw_counter_id=0x17
sleep 2

sudo taskset -c 2 ../../benchmarks/hesoc-mark/cuda/cudainterf -s -d $SIZE -i 20000 -m memcpy & PID_TO_KILL0=$!

sleep 3
PID_TO_KILL=$(pgrep cudainterf)

sudo kill -s SIGUSR1 $PID_TO_KILL
sudo echo 0 > /sys/kernel/debug/tracing/trace

bandwidth -c 0 -t 1000 > ${TEST}_${INTERF}.txt &
sleep 5

sudo cat /sys/kernel/debug/tracing/trace > trace_${TEST}_vs_${INTERF}.txt 
sudo kill -s SIGUSR2 $PID_TO_KILL
sudo killall -9 bandwidth
sudo rmmod memguard

sudo kill -9 $PID_TO_KILL
sudo killall -9 cudainterf

python3 splitftrace.py trace_${TEST}_vs_${INTERF}.txt

sleep 10



TEST="bandwidth0"
INTERF="memset"
echo "$TEST against $INTERF"

sudo insmod ../../patched_memguard/memguard.ko g_hw_counter_id=0x17
sleep 2

sudo taskset -c 2 ../../benchmarks/hesoc-mark/cuda/cudainterf -s -d $SIZE -i 20000 -m memset & PID_TO_KILL0=$!

sleep 3
PID_TO_KILL=$(pgrep cudainterf)

sudo kill -s SIGUSR1 $PID_TO_KILL
sudo echo 0 > /sys/kernel/debug/tracing/trace

bandwidth -c 0 -t 1000 > ${TEST}_${INTERF}.txt &
sleep 5

sudo cat /sys/kernel/debug/tracing/trace > trace_${TEST}_vs_${INTERF}.txt 
sudo kill -s SIGUSR2 $PID_TO_KILL
sudo killall -9 bandwidth
sudo rmmod memguard

sudo kill -9 $PID_TO_KILL
sudo killall -9 cudainterf

python3 splitftrace.py trace_${TEST}_vs_${INTERF}.txt

sleep 10




TEST="bandwidth123"
INTERF="memcpy"
echo "$TEST against $INTERF"

sudo insmod ../../patched_memguard/memguard.ko g_hw_counter_id=0x17
sleep 2

sudo taskset -c 2 ../../benchmarks/hesoc-mark/cuda/cudainterf -s -d $SIZE -i 20000 -m memcpy & PID_TO_KILL0=$!

sleep 3
PID_TO_KILL=$(pgrep cudainterf)

sudo kill -s SIGUSR1 $PID_TO_KILL
sudo echo 0 > /sys/kernel/debug/tracing/trace

for c in 1 2 3; do bandwidth -c $c -t 1000 & done;
sleep 5

sudo cat /sys/kernel/debug/tracing/trace > trace_${TEST}_vs_${INTERF}.txt 
sudo kill -s SIGUSR2 $PID_TO_KILL
sudo killall -9 bandwidth
sudo rmmod memguard

sudo kill -9 $PID_TO_KILL
sudo killall -9 cudainterf

python3 splitftrace.py trace_${TEST}_vs_${INTERF}.txt

sleep 10



TEST="bandwidth123"
INTERF="memset"
echo "$TEST against $INTERF"

sudo insmod ../../patched_memguard/memguard.ko g_hw_counter_id=0x17
sleep 2

sudo taskset -c 2 ../../benchmarks/hesoc-mark/cuda/cudainterf -s -d $SIZE -i 20000 -m memset & PID_TO_KILL0=$!

sleep 3
PID_TO_KILL=$(pgrep cudainterf)

sudo kill -s SIGUSR1 $PID_TO_KILL
sudo echo 0 > /sys/kernel/debug/tracing/trace

for c in 1 2 3; do bandwidth -c $c -t 1000 & done;
sleep 5

sudo cat /sys/kernel/debug/tracing/trace > trace_${TEST}_vs_${INTERF}.txt 
sudo kill -s SIGUSR2 $PID_TO_KILL
sudo killall -9 bandwidth
sudo rmmod memguard

sudo kill -9 $PID_TO_KILL
sudo killall -9 cudainterf

python3 splitftrace.py trace_${TEST}_vs_${INTERF}.txt

sleep 10


TEST="bandwidth"

mv *.txt res/${TEST}_baselines

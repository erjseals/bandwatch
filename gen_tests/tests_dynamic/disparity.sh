#! /usr/bin/env bash 

declare -a LOOP=(0 1 2 3 4 5 6 7 8 9 10)

sysctl -w kernel.sched_rt_runtime_us=-1 

TEST="disparity"
SIZE="250000"

mkdir -p res/${TEST}

if [[ 1 == 1 ]]
then
  INTERF="isolated"
  echo "$TEST $INTERF"
    
  sudo insmod ../../patched_memguard/memguard.ko g_hw_counter_id=0x17
  sleep 2

  sudo echo 0 > /sys/kernel/debug/memguard/throttle
  sleep 2

  sudo echo 0 > /sys/kernel/debug/tracing/trace

  sudo taskset -c 0 ../../benchmarks/sd-vbs/benchmarks/${TEST}/data/fullhd/${TEST} ../../benchmarks/sd-vbs/benchmarks/${TEST}/data/fullhd/. | grep "Cycles elapsed" >> cycles_${INTERF}.txt & PID_TO_WAIT=$! 
  wait $PID_TO_WAIT

  sudo cat /sys/kernel/debug/tracing/trace > trace_${INTERF}.txt

  sudo rmmod memguard

  sleep 2

  python3 splitftrace.py trace_${INTERF}.txt

  sleep 10
fi

if [[ 1 == 1 ]]
then
  INTERF="memset"
  echo "$TEST against $INTERF"

  sudo insmod ../../patched_memguard/memguard.ko g_hw_counter_id=0x17
  sleep 2

  sudo echo 0 > /sys/kernel/debug/memguard/throttle
  sleep 2

  sudo taskset -c 2 ../../benchmarks/hesoc-mark/cuda/cudainterf -d $SIZE -s --iterations=20055 --mode=memset & 
  sleep 5

  PID_TO_KILL=$(pgrep cudainterf)

  sudo kill -s SIGUSR1 $PID_TO_KILL
  sudo echo 0 > /sys/kernel/debug/tracing/trace

  sudo taskset -c 0 ../../benchmarks/sd-vbs/benchmarks/${TEST}/data/fullhd/${TEST} ../../benchmarks/sd-vbs/benchmarks/${TEST}/data/fullhd/. | grep "Cycles elapsed" >> cycles_${INTERF}.txt & PID_TO_WAIT=$! 
  wait $PID_TO_WAIT

  sudo cat /sys/kernel/debug/tracing/trace > trace_${INTERF}.txt
  sudo kill -s SIGUSR2 $PID_TO_KILL

  sudo rmmod memguard

  sleep 2

  sudo kill -9 $PID_TO_KILL > /dev/null 2>&1
  sudo killall -9 cudainterf > /dev/null 2>&1

  python3 splitftrace.py trace_${INTERF}.txt

  sleep 10
fi





if [[ 1 == 1 ]]
then
  INTERF="memcpy"
  echo "$TEST against $INTERF"

  sudo insmod ../../patched_memguard/memguard.ko g_hw_counter_id=0x17
  sleep 2

  sudo echo 0 > /sys/kernel/debug/memguard/throttle
  sleep 2

  sudo taskset -c 2 ../../benchmarks/hesoc-mark/cuda/cudainterf -d $SIZE -s --iterations=20055 --mode=memset & 
  sleep 5

  PID_TO_KILL=$(pgrep cudainterf)

  sudo kill -s SIGUSR1 $PID_TO_KILL
  sudo echo 0 > /sys/kernel/debug/tracing/trace

  sudo taskset -c 0 ../../benchmarks/sd-vbs/benchmarks/${TEST}/data/fullhd/${TEST} ../../benchmarks/sd-vbs/benchmarks/${TEST}/data/fullhd/. | grep "Cycles elapsed" >> cycles_${INTERF}.txt & PID_TO_WAIT=$! 
  wait $PID_TO_WAIT

  sudo cat /sys/kernel/debug/tracing/trace > trace_${INTERF}.txt
  sudo kill -s SIGUSR2 $PID_TO_KILL

  sudo rmmod memguard

  sleep 2

  sudo kill -9 $PID_TO_KILL > /dev/null 2>&1
  sudo killall -9 cudainterf > /dev/null 2>&1

  python3 splitftrace.py trace_${INTERF}.txt

  sleep 10
fi


if [[ 1 == 1 ]]
then

  INTERF="bandwidth_read"
  echo "$TEST against $INTERF"

  sudo insmod ../../patched_memguard/memguard.ko g_hw_counter_id=0x17
  sleep 2

  sudo echo 0 > /sys/kernel/debug/memguard/throttle
  sleep 2

  for c in 1 2 3; do bandwidth -c $c -t 1000 & done

  sudo echo 0 > /sys/kernel/debug/tracing/trace

  taskset -c 0 ../../benchmarks/sd-vbs/benchmarks/${TEST}/data/fullhd/${TEST} ../../benchmarks/sd-vbs/benchmarks/${TEST}/data/fullhd/. | grep "Cycles elapsed" > cycles_${TEST}_vs_${INTERF}.txt & PID_TO_WAIT=$!

  wait $PID_TO_WAIT
  sudo cat /sys/kernel/debug/tracing/trace > trace_${TEST}_vs_${INTERF}.txt 
  sudo rmmod memguard

  sudo killall -9 bandwidth > /dev/null 2>&1

  python3 splitftrace.py trace_${TEST}_vs_${INTERF}.txt

  sleep 10




  INTERF="bandwidth_write"
  echo "$TEST against $INTERF"

  sudo insmod ../../patched_memguard/memguard.ko g_hw_counter_id=0x17
  sleep 2

  for c in 1 2 3; do bandwidth -a write -c $c -t 1000 & done

  sudo echo 0 > /sys/kernel/debug/tracing/trace

  taskset -c 0 ../../benchmarks/sd-vbs/benchmarks/${TEST}/data/fullhd/${TEST} ../../benchmarks/sd-vbs/benchmarks/${TEST}/data/fullhd/. | grep "Cycles elapsed" > cycles_${TEST}_vs_${INTERF}.txt & PID_TO_WAIT=$!

  wait $PID_TO_WAIT
  sudo cat /sys/kernel/debug/tracing/trace > trace_${TEST}_vs_${INTERF}.txt 
  sudo rmmod memguard

  sudo killall -9 bandwidth > /dev/null 2>&1

  python3 splitftrace.py trace_${TEST}_vs_${INTERF}.txt

  sleep 10




  INTERF="bandwidth_read_memcpy"
  echo "$TEST against $INTERF"

  sudo insmod ../../patched_memguard/memguard.ko g_hw_counter_id=0x17
  sleep 2

  for c in 1 2 3; do bandwidth -c $c -t 1000 & done
  sudo taskset -c 2 ../../benchmarks/hesoc-mark/cuda/cudainterf -s -d $SIZE -i 20000 -m memcpy & PID_TO_KILL0=$!

  sleep 5
  PID_TO_KILL=$(pgrep cudainterf)

  sudo kill -s SIGUSR1 $PID_TO_KILL
  sudo echo 0 > /sys/kernel/debug/tracing/trace

  taskset -c 0 ../../benchmarks/sd-vbs/benchmarks/${TEST}/data/fullhd/${TEST} ../../benchmarks/sd-vbs/benchmarks/${TEST}/data/fullhd/. | grep "Cycles elapsed" > cycles_${TEST}_vs_${INTERF}.txt & PID_TO_WAIT=$!

  wait $PID_TO_WAIT
  sudo cat /sys/kernel/debug/tracing/trace > trace_${TEST}_vs_${INTERF}.txt 
  sudo kill -s SIGUSR2 $PID_TO_KILL
  sudo rmmod memguard

  sudo killall -9 bandwidth > /dev/null 2>&1
  sudo kill -9 $PID_TO_KILL  > /dev/null 2>&1
  sudo killall -9 cudainterf > /dev/null 2>&1

  python3 splitftrace.py trace_${TEST}_vs_${INTERF}.txt

  sleep 10





  INTERF="bandwidth_write_memset"
  echo "$TEST against $INTERF"

  sudo insmod ../../patched_memguard/memguard.ko g_hw_counter_id=0x17
  sleep 2

  for c in 1 2 3; do bandwidth -a write -c $c -t 1000 & done 
  sudo taskset -c 2 ../../benchmarks/hesoc-mark/cuda/cudainterf -s -d $SIZE -i 20000 -m memset & PID_TO_KILL0=$!

  sleep 5
  PID_TO_KILL=$(pgrep cudainterf)

  sudo kill -s SIGUSR1 $PID_TO_KILL
  sudo echo 0 > /sys/kernel/debug/tracing/trace

  taskset -c 0 ../../benchmarks/sd-vbs/benchmarks/${TEST}/data/fullhd/${TEST} ../../benchmarks/sd-vbs/benchmarks/${TEST}/data/fullhd/. | grep "Cycles elapsed" > cycles_${TEST}_vs_${INTERF}.txt & PID_TO_WAIT=$!

  wait $PID_TO_WAIT
  sudo cat /sys/kernel/debug/tracing/trace > trace_${TEST}_vs_${INTERF}.txt 
  sudo kill -s SIGUSR2 $PID_TO_KILL
  sudo rmmod memguard

  sudo killall -9 bandwidth > /dev/null 2>&1
  sudo kill -9 $PID_TO_KILL > /dev/null 2>&1
  sudo killall -9 cudainterf > /dev/null 2>&1

  python3 splitftrace.py trace_${TEST}_vs_${INTERF}.txt
fi

mv *.txt res/${TEST}


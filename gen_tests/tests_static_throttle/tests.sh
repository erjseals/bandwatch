#! /usr/bin/env bash 

declare -a THROTTLE_AMOUNT=(0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16)

declare -a LOOP=(0)

TEST="disparity"
SIZE="250000"

sudo insmod ../../patched_memguard/memguard.ko g_hw_counter_id=0x17
sleep 2

for j in "${LOOP[@]}"
do
  if [[ $j == 0 ]]
  then 
    TEST="disparity"
  fi
  if [[ $j == 1 ]]
  then 
    TEST="mser"
  fi
  if [[ $j == 2 ]]
  then 
    TEST="sift"
  fi
  if [[ $j == 3 ]]
  then 
    TEST="texture_synthesis"
  fi
  if [[ $j == 4 ]]
  then 
    TEST="tracking"
  fi

  mkdir -p res/${TEST}/memset
  mkdir -p res/${TEST}/memcpy

  sysctl -w kernel.sched_rt_runtime_us=-1 

  for i in  "${THROTTLE_AMOUNT[@]}"
  do 

    echo "Memset, Throttle Amount ${i}"
    sudo echo ${i} > /sys/kernel/debug/memguard/throttle
    sleep 2

    sudo taskset -c 2 ../../benchmarks/hesoc-mark/cuda/cudainterf -d $SIZE -s --iterations=20055 --mode=memset | grep "Memset BW" | awk '{ print $4 }' >> bw_memset.txt & PID_TO_KILL0=$!
    sleep 5

    PID_TO_KILL=$(pgrep cudainterf)

    sudo echo 0 > /sys/kernel/debug/tracing/trace

    sudo kill -s SIGUSR1 $PID_TO_KILL
    #sudo taskset -c 0 ../../benchmarks/sd-vbs/benchmarks/${TEST}/data/fullhd/${TEST} ../../benchmarks/sd-vbs/benchmarks/${TEST}/data/fullhd/. | grep "Cycles elapsed" >> cycles_memset.txt & PID_TO_WAIT=$! 
    #wait $PID_TO_WAIT
    sleep 5
    sudo kill -s SIGUSR2 $PID_TO_KILL

    sudo cat /sys/kernel/debug/tracing/trace > trace_memset_$i.txt

    
    wait $PID_TO_KILL0

    sudo kill -9 $PID_TO_KILL
    sudo killall -9 cudainterf

    python3 splitftrace_static_throttle.py trace_memset_$i.txt

    sleep 10
  done 

  mv *.txt res/${TEST}/memset

  for i in  "${THROTTLE_AMOUNT[@]}"
  do 
    echo "Memcpy, Throttle Amount ${i}"
    sudo echo ${i} > /sys/kernel/debug/memguard/throttle
    sleep 2

    sudo taskset -c 2 ../../benchmarks/hesoc-mark/cuda/cudainterf -s -d $SIZE --iterations=20055 --mode=memcpy | grep "Memcpy BW" | awk '{ print $4 }' >> bw_memcpy.txt  & PID_TO_KILL0=$!
    sleep 5

    PID_TO_KILL=$(pgrep cudainterf)

    sudo kill -s SIGUSR1 $PID_TO_KILL
    sudo echo 0 > /sys/kernel/debug/tracing/trace

    sudo taskset -c 0 ../../benchmarks/sd-vbs/benchmarks/${TEST}/data/fullhd/${TEST} ../../benchmarks/sd-vbs/benchmarks/${TEST}/data/fullhd/. | grep "Cycles elapsed" >> cycles_memcpy.txt & PID_TO_WAIT=$! 
    wait $PID_TO_WAIT

    sudo cat /sys/kernel/debug/tracing/trace > trace_memcpy_$i.txt
    sudo kill -s SIGUSR2 $PID_TO_KILL

    sleep 1

    sudo kill -9 $PID_TO_KILL
    sudo killall -9 cudainterf

    python3 splitftrace_static_throttle.py trace_memcpy_$i.txt

    sleep 5
  done 

  mv *.txt res/${TEST}/memcpy
done
sudo rmmod memguard

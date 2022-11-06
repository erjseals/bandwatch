#! /usr/bin/env bash 

declare -a LOOP=(3)

declare -a THROTTLE=(0 1)

declare -a STATIC=(0)

sysctl -w kernel.sched_rt_runtime_us=-1 

TEST="tracking"
SIZE="250000"

isolated=11
memcpy=11
memset=11
bandwidth=11
bandwidth_heavy=1

sudo insmod ../../patched_memguard/memguard.ko g_hw_counter_id=0x17
sleep 2


for i in "${LOOP[@]}"
do
  if [[ $i == 0 ]]
  then
    TEST="disparity"
  fi
  if [[ $i == 1 ]]
  then
    TEST="mser"
  fi
  if [[ $i == 2 ]]
  then
    TEST="sift"
  fi
  if [[ $i == 3 ]]
  then
    TEST="texture_synthesis"
  fi
  if [[ $i == 4 ]]
  then
    TEST="tracking"
  fi

  mkdir -p res/bwGPU/${TEST}

  if [[ $isolated == 1 ]]
  then
    INTERF="isolated"
    echo "$TEST $INTERF"
      
    sudo echo 0 > /sys/kernel/debug/memguard/throttle
    sleep 2

    sudo echo 0 > /sys/kernel/debug/tracing/trace

    sudo taskset -c 0 ../../benchmarks/sd-vbs/benchmarks/${TEST}/data/fullhd/${TEST} ../../benchmarks/sd-vbs/benchmarks/${TEST}/data/fullhd/. | grep "Cycles elapsed" >> cycles_${INTERF}.txt & PID_TO_WAIT=$! 
    wait $PID_TO_WAIT

    sudo cat /sys/kernel/debug/tracing/trace > trace_${INTERF}.txt

    sleep 2

    python3 splitftrace.py trace_${INTERF}.txt

    sleep 10
  fi


  if [[ $memset == 1 ]]
  then
    INTERF="memset"
    echo "$TEST against $INTERF"

    sudo echo 0 > /sys/kernel/debug/memguard/throttle
    sleep 2

    sudo taskset -c 2 ../../benchmarks/hesoc-mark/cuda/cudainterf -d $SIZE -s --iterations=20055 --mode=memset | grep "Memset BW" | awk '{ print $4 }'  >> bw_${INTERF}.txt & PID_TO_KILL0=$!
    sleep 5

    PID_TO_KILL=$(pgrep cudainterf)

    sudo echo 0 > /sys/kernel/debug/tracing/trace
    sudo kill -s SIGUSR1 $PID_TO_KILL

    sudo taskset -c 0 ../../benchmarks/sd-vbs/benchmarks/${TEST}/data/fullhd/${TEST} ../../benchmarks/sd-vbs/benchmarks/${TEST}/data/fullhd/. | grep "Cycles elapsed" >> cycles_${INTERF}.txt & PID_TO_WAIT=$! 
    wait $PID_TO_WAIT
    sudo kill -s SIGUSR2 $PID_TO_KILL

    sudo cat /sys/kernel/debug/tracing/trace > trace_${INTERF}.txt

    wait $PID_TO_KILL0

    sleep 2

    sudo kill -9 $PID_TO_KILL > /dev/null 2>&1
    sudo killall -9 cudainterf > /dev/null 2>&1

    python3 splitftrace.py trace_${INTERF}.txt

    sleep 10
  fi


  if [[ $memcpy == 1 ]]
  then
    INTERF="memcpy"
    echo "$TEST against $INTERF"

    sudo echo 0 > /sys/kernel/debug/memguard/throttle
    sleep 2

    sudo taskset -c 2 ../../benchmarks/hesoc-mark/cuda/cudainterf -d $SIZE -s --iterations=20055 --mode=memcpy | grep "Memcpy BW" | awk '{ print $4 }'  >> bw_${INTERF}.txt & PID_TO_KILL0=$!
    sleep 5

    PID_TO_KILL=$(pgrep cudainterf)

    sudo echo 0 > /sys/kernel/debug/tracing/trace
    sudo kill -s SIGUSR1 $PID_TO_KILL

    sudo taskset -c 0 ../../benchmarks/sd-vbs/benchmarks/${TEST}/data/fullhd/${TEST} ../../benchmarks/sd-vbs/benchmarks/${TEST}/data/fullhd/. | grep "Cycles elapsed" >> cycles_${INTERF}.txt & PID_TO_WAIT=$! 
    wait $PID_TO_WAIT
    sudo kill -s SIGUSR2 $PID_TO_KILL

    sudo cat /sys/kernel/debug/tracing/trace > trace_${INTERF}.txt

    wait $PID_TO_KILL0

    sleep 2

    sudo kill -9 $PID_TO_KILL > /dev/null 2>&1
    sudo killall -9 cudainterf > /dev/null 2>&1

    python3 splitftrace.py trace_${INTERF}.txt

    sleep 10
  fi

  for j in "${STATIC[@]}"
  do
    if [[ $j == 0 ]]
    then
      sudo echo mb 100000  75 75 75> /sys/kernel/debug/memguard/limit
    fi
    if [[ $j == 1 ]]
    then
      sudo echo mb 100000 100 100 100 > /sys/kernel/debug/memguard/limit
    fi
    if [[ $j == 2 ]]
    then
      sudo echo mb 100000 125 125 125 > /sys/kernel/debug/memguard/limit
    fi

    if [[ $bandwidth == 1 ]]
    then
      INTERF="bandwidth_read"
      echo "$TEST against $INTERF"

      sudo echo 0 > /sys/kernel/debug/memguard/throttle
      sleep 2


      sudo echo 0 > /sys/kernel/debug/tracing/trace

      for c in 1 2 3; do bandwidth -c $c -t 1000 >> bw_${INTERF}.txt & done

      taskset -c 0 ../../benchmarks/sd-vbs/benchmarks/${TEST}/data/fullhd/${TEST} ../../benchmarks/sd-vbs/benchmarks/${TEST}/data/fullhd/. | grep "Cycles elapsed" >> cycles_${TEST}_vs_${INTERF}.txt & PID_TO_WAIT=$!

      wait $PID_TO_WAIT
      sudo killall -2 bandwidth > /dev/null 2>&1

      sudo cat /sys/kernel/debug/tracing/trace > trace_${TEST}_vs_${INTERF}.txt 

      sleep 1

      sudo killall -9 bandwidth > /dev/null 2>&1

      python3 splitftrace.py trace_${TEST}_vs_${INTERF}.txt

      sleep 5


      INTERF="bandwidth_write"
      echo "$TEST against $INTERF"


      sleep 5

      sudo echo 0 > /sys/kernel/debug/tracing/trace
      for c in 1 2 3; do bandwidth -a write -c $c -t 1000 >> bw_${INTERF}.txt & done

      taskset -c 0 ../../benchmarks/sd-vbs/benchmarks/${TEST}/data/fullhd/${TEST} ../../benchmarks/sd-vbs/benchmarks/${TEST}/data/fullhd/. | grep "Cycles elapsed" >> cycles_${TEST}_vs_${INTERF}.txt & PID_TO_WAIT=$!

      wait $PID_TO_WAIT
      sudo killall -2 bandwidth > /dev/null 2>&1

      sudo cat /sys/kernel/debug/tracing/trace > trace_${TEST}_vs_${INTERF}.txt 

      sleep 1

      sudo killall -9 bandwidth > /dev/null 2>&1

      python3 splitftrace.py trace_${TEST}_vs_${INTERF}.txt
    fi
  done


  if [[ $bandwidth_heavy == 1 ]]
  then
    for j in "${STATIC[@]}"
    do
      if [[ $j == 0 ]]
      then
        sudo echo mb 100000 10000 10000 10000 > /sys/kernel/debug/memguard/limit
      fi

      for x in "${THROTTLE[@]}"
      do

        if [[ $x == 0 ]]
        then
          sudo echo 0 > /sys/kernel/debug/memguard/throttle
        fi
        if [[ $x == 1 ]]
        then
          sudo echo 1 > /sys/kernel/debug/memguard/throttle
        fi
        if [[ $x == 2 ]]
        then
          sudo echo 2 > /sys/kernel/debug/memguard/throttle
        fi
        if [[ $x == 3 ]]
        then
          sudo echo 3 > /sys/kernel/debug/memguard/throttle
        fi

        echo "STATIC $j and THROTTLE $x"
        echo " "

        INTERF="bandwidth_read_memcpy"
        echo "$TEST against $INTERF"

        sudo taskset -c 2 ../../benchmarks/hesoc-mark/cuda/cudainterf -s -d $SIZE -i 20000 -m memcpy | grep "Memcpy BW" | awk '{ print $4 }'  >> bw_${INTERF}_gpu.txt & PID_TO_KILL0=$!

        sleep 5
        PID_TO_KILL=$(pgrep cudainterf)

        sudo echo 0 > /sys/kernel/debug/tracing/trace
        for c in 1 2 3; do bandwidth -c $c -t 1000 | grep "CPU" >> bw_${INTERF}_cpu.txt & done
        sudo kill -s SIGUSR1 $PID_TO_KILL

        taskset -c 0 ../../benchmarks/sd-vbs/benchmarks/${TEST}/data/fullhd/${TEST} ../../benchmarks/sd-vbs/benchmarks/${TEST}/data/fullhd/. | grep "Cycles elapsed" >> cycles_${TEST}_vs_${INTERF}.txt & PID_TO_WAIT=$!

        wait $PID_TO_WAIT
        sudo kill -s SIGUSR2 $PID_TO_KILL
        sudo killall -2 bandwidth
        sudo cat /sys/kernel/debug/tracing/trace > trace_${TEST}_vs_${INTERF}.txt 

        sleep 1

        wait $PID_TO_KILL0

        sudo killall -9 bandwidth > /dev/null 2>&1
        sudo kill -9 $PID_TO_KILL  > /dev/null 2>&1
        sudo killall -9 cudainterf > /dev/null 2>&1

        python3 splitftrace.py trace_${TEST}_vs_${INTERF}.txt

        echo " " >> bw_${INTERF}_cpu.txt

        sleep 10


        INTERF="bandwidth_write_memset"
        echo "$TEST against $INTERF"

        sudo taskset -c 2 ../../benchmarks/hesoc-mark/cuda/cudainterf -s -d $SIZE -i 20000 -m memset | grep "Memset BW" | awk '{ print $4 }'  >> bw_${INTERF}_gpu.txt & PID_TO_KILL0=$!

        sleep 5
        PID_TO_KILL=$(pgrep cudainterf)

        for c in 1 2 3; do bandwidth -a write -c $c -t 1000 | grep "CPU" >> bw_${INTERF}_cpu.txt & done 
        sudo echo 0 > /sys/kernel/debug/tracing/trace
        sudo kill -s SIGUSR1 $PID_TO_KILL

        taskset -c 0 ../../benchmarks/sd-vbs/benchmarks/${TEST}/data/fullhd/${TEST} ../../benchmarks/sd-vbs/benchmarks/${TEST}/data/fullhd/. | grep "Cycles elapsed" >> cycles_${TEST}_vs_${INTERF}.txt & PID_TO_WAIT=$!

        wait $PID_TO_WAIT
        sudo kill -s SIGUSR2 $PID_TO_KILL
        sudo killall -2 bandwidth
        sudo cat /sys/kernel/debug/tracing/trace > trace_${TEST}_vs_${INTERF}.txt 

        sleep 1

        wait $PID_TO_KILL0
        
        sudo killall -9 bandwidth > /dev/null 2>&1
        sudo kill -9 $PID_TO_KILL > /dev/null 2>&1
        sudo killall -9 cudainterf > /dev/null 2>&1

        python3 splitftrace.py trace_${TEST}_vs_${INTERF}.txt

        echo " " >> bw_${INTERF}_gpu.txt
      done
      
    done
  fi
  mv *.txt res/bwGPU/${TEST}
done

sudo rmmod memguard

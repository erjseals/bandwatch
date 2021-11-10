#!/bin/bash

for limit in {0..512..16}
do
  touch "prof-${limit}-10.txt"
  for throttle in {0..32..4}
  do
    echo $limit > /sys/kernel/debug/throttle/limit
    echo $throttle > /sys/kernel/debug/throttle/throttle

    /usr/local/cuda/bin/nvprof -o ~/dev/linux-kernerl/profile/prof-${limit}-${throttle}.nvvp 
  done
done

echo All done

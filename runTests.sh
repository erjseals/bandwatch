#!/bin/bash

for limit in {0..512..16}
do
  for throttle in {0..32..4}
  do
    echo $limit > /sys/kernel/debug/throttle/limit
    echo $throttle > /sys/kernel/debug/throttle/throttle

    /usr/local/cuda/bin/nvprof -o ~/dev/linux-kernel/profile/prof-${limit}-${throttle}.nvvp ~/Documents/NVIDIA_CUDA-10.2_Samples/0_Simple/matrixMul/./matrixMul
  done
done

echo All done

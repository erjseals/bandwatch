#!/bin/bash

  for throttle in {0..31..1}
  do
    echo $throttle > /sys/kernel/debug/throttle/throttle

    /usr/local/cuda/bin/nvprof -o ~/dev/tests/profile/prof-${throttle}.nvvp ~/dev/hesoc-mark/cuda/./cudameasure 102400000 10 dryrun 
  done

echo All done

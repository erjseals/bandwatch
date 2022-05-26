#! /usr/bin/env bash 

declare -a THROTTLE_AMOUNT=(0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16)

for i in  "${THROTTLE_AMOUNT[@]}"
do 
  gnuplot -c gnuplot.scr ../tests_staticThrottleMemset/parsed_trace_memset_$i.txt > staticTests/memSetStatic/memset_$i.pdf
  gnuplot -c gnuplot.scr ../tests_staticThrottleMemcpy/parsed_trace_memcpy_$i.txt > staticTests/memCpyStatic/memcpy_$i.pdf
done


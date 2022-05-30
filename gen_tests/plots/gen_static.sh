#! /usr/bin/env bash 

declare -a THROTTLE_AMOUNT=(0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16)

mkdir -p static_tests/memset
mkdir -p static_tests/memcpy

for i in  "${THROTTLE_AMOUNT[@]}"
do 
  gnuplot -c gnuplot.scr ../static_throttle_tests/res/disparity/memset/parsed_trace_memset_$i.txt > static_tests/memset/memset_$i.pdf
  gnuplot -c gnuplot.scr ../static_throttle_tests/res/disparity/memcpy/parsed_trace_memcpy_$i.txt > static_tests/memcpy/memcpy_$i.pdf
done


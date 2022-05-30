#! /usr/bin/env bash 

declare -a THROTTLE_AMOUNT=(0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16)

mkdir -p plots
mkdir -p plots/disparity_memset
mkdir -p plots/disparity_memcpy

for i in  "${THROTTLE_AMOUNT[@]}"
do 
  gnuplot -c gnuplot.scr disparity_memset_static_throttle/parsed_trace_memset_$i.txt > plots/disparity_memset/memset_$i.pdf
  gnuplot -c gnuplot.scr disparity_memcpy_static_throttle/parsed_trace_memcpy_$i.txt > plots/disparity_memcpy/memcpy_$i.pdf
done


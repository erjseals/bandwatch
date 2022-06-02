#! /usr/bin/env bash 

declare -a THROTTLE_AMOUNT=(0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16)

TEST="tracking"

mkdir -p plots_static/${TEST}/memset
mkdir -p plots_static/${TEST}/memcpy

for i in  "${THROTTLE_AMOUNT[@]}"
do 
  gnuplot -c gnuplot.scr ../tests_static_throttle/res/${TEST}/memset/parsed_trace_memset_$i.txt > plots_static/${TEST}/memset/memset_$i.pdf
  gnuplot -c gnuplot.scr ../tests_static_throttle/res/${TEST}/memcpy/parsed_trace_memcpy_$i.txt > plots_static/${TEST}/memcpy/memcpy_$i.pdf
done


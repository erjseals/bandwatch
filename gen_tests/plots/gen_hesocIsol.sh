#! /usr/bin/env bash 

mkdir -p plots_hesoc

gnuplot -c gnuplot.scr ../tests_baseline/res/hesoc_isolated/parsed_trace_memset.txt > plots_hesoc/memset_isol.pdf
gnuplot -c gnuplot.scr ../tests_baseline/res/hesoc_isolated/parsed_trace_memcpy.txt > plots_hesoc/memcpy_isol.pdf


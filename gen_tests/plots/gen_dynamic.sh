#! /usr/bin/env bash 

mkdir -p plots_dynamic

gnuplot -c gnuplot_dynamic.scr ../tests_dynamic/res/disparity/memset/parsed_trace_dynamic_memset.txt > plots_dynamic/memset.pdf
gnuplot -c gnuplot_dynamic.scr ../tests_dynamic/res/disparity/memcpy/parsed_trace_dynamic_memcpy.txt > plots_dynamic/memcpy.pdf

gnuplot -c gnuplot_dynamic_zoomed.scr ../tests_dynamic/res/disparity/memset/parsed_trace_dynamic_memset.txt > plots_dynamic/memset_zoomed.pdf
gnuplot -c gnuplot_dynamic_zoomed.scr ../tests_dynamic/res/disparity/memcpy/parsed_trace_dynamic_memcpy.txt > plots_dynamic/memcpy_zoomed.pdf

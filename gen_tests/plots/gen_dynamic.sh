#! /usr/bin/env bash 

gnuplot -c gnuplot.scr ../tests_dynamic/parsed_trace_dynamic_memset.txt > dynamicTests/memset.pdf
gnuplot -c gnuplot.scr ../tests_dynamic/parsed_trace_dynamic_memcpy.txt > dynamicTests/memcpy.pdf

#! /usr/bin/env bash 

gnuplot -c gnuplot.scr ../tests_hesocIsol/parsed_trace_memset.txt > staticTests/memsetIsol.pdf
gnuplot -c gnuplot.scr ../tests_hesocIsol/parsed_trace_memcpy.txt > staticTests/memcpyIsol.pdf


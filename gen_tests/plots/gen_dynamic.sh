#! /usr/bin/env bash 

TEST="disparity"

mkdir -p plots_dynamic/${TEST}

gnuplot -c gnuplot_dynamic.scr ../tests_dynamic/res/${TEST}/parsed_trace_isolated.txt > plots_dynamic/${TEST}/isolated.pdf

gnuplot -c gnuplot_dynamic.scr ../tests_dynamic/res/${TEST}/parsed_trace_memcpy.txt > plots_dynamic/${TEST}/memcpy.pdf

gnuplot -c gnuplot_dynamic.scr ../tests_dynamic/res/${TEST}/parsed_trace_memset.txt > plots_dynamic/${TEST}/memset.pdf

gnuplot -c gnuplot_dynamic.scr ../tests_dynamic/res/${TEST}/parsed_trace_${TEST}_vs_bandwidth_read.txt > plots_dynamic/${TEST}/bandwidth_read.pdf

gnuplot -c gnuplot_dynamic.scr ../tests_dynamic/res/${TEST}/parsed_trace_${TEST}_vs_bandwidth_write.txt > plots_dynamic/${TEST}/bandwidth_write.pdf

gnuplot -c gnuplot_dynamic.scr ../tests_dynamic/res/${TEST}/parsed_trace_${TEST}_vs_bandwidth_write_memset.txt > plots_dynamic/${TEST}/bandwidth_write_memset.pdf

gnuplot -c gnuplot_dynamic.scr ../tests_dynamic/res/${TEST}/parsed_trace_${TEST}_vs_bandwidth_read_memcpy.txt > plots_dynamic/${TEST}/bandwidth_read_memcpy.pdf


# Zoomed Sections

gnuplot -c gnuplot_dynamic_zoomed.scr ../tests_dynamic/res/${TEST}/parsed_trace_isolated.txt > plots_dynamic/${TEST}/zoomed_isolated.pdf

gnuplot -c gnuplot_dynamic_zoomed.scr ../tests_dynamic/res/${TEST}/parsed_trace_memcpy.txt > plots_dynamic/${TEST}/zoomed_memcpy.pdf

gnuplot -c gnuplot_dynamic_zoomed.scr ../tests_dynamic/res/${TEST}/parsed_trace_memset.txt > plots_dynamic/${TEST}/zoomed_memset.pdf

gnuplot -c gnuplot_dynamic_zoomed.scr ../tests_dynamic/res/${TEST}/parsed_trace_${TEST}_vs_bandwidth_read.txt > plots_dynamic/${TEST}/zoomed_bandwidth_read.pdf

gnuplot -c gnuplot_dynamic_zoomed.scr ../tests_dynamic/res/${TEST}/parsed_trace_${TEST}_vs_bandwidth_write.txt > plots_dynamic/${TEST}/zoomed_bandwidth_write.pdf

gnuplot -c gnuplot_dynamic_zoomed.scr ../tests_dynamic/res/${TEST}/parsed_trace_${TEST}_vs_bandwidth_write_memset.txt > plots_dynamic/${TEST}/zoomed_bandwidth_write_memset.pdf

gnuplot -c gnuplot_dynamic_zoomed.scr ../tests_dynamic/res/${TEST}/parsed_trace_${TEST}_vs_bandwidth_read_memcpy.txt > plots_dynamic/${TEST}/zoomed_bandwidth_read_memcpy.pdf

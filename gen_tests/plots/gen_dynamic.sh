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

gnuplot -c gnuplot_dynamic_zoomed.scr ../tests_dynamic/res/${TEST}/parsed_trace_isolated.txt > plots_dynamic/${TEST}/isolated_zoomed.pdf

gnuplot -c gnuplot_dynamic_zoomed.scr ../tests_dynamic/res/${TEST}/parsed_trace_memcpy.txt > plots_dynamic/${TEST}/memcpy_zoomed.pdf

gnuplot -c gnuplot_dynamic_zoomed.scr ../tests_dynamic/res/${TEST}/parsed_trace_memset.txt > plots_dynamic/${TEST}/memset_zoomed.pdf

gnuplot -c gnuplot_dynamic_zoomed.scr ../tests_dynamic/res/${TEST}/parsed_trace_${TEST}_vs_bandwidth_read.txt > plots_dynamic/${TEST}/bandwidth_read_zoomed.pdf

gnuplot -c gnuplot_dynamic_zoomed.scr ../tests_dynamic/res/${TEST}/parsed_trace_${TEST}_vs_bandwidth_write.txt > plots_dynamic/${TEST}/bandwidth_write_zoomed.pdf

gnuplot -c gnuplot_dynamic_zoomed.scr ../tests_dynamic/res/${TEST}/parsed_trace_${TEST}_vs_bandwidth_write_memset.txt > plots_dynamic/${TEST}/bandwidth_write_memset_zoomed.pdf

gnuplot -c gnuplot_dynamic_zoomed.scr ../tests_dynamic/res/${TEST}/parsed_trace_${TEST}_vs_bandwidth_read_memcpy.txt > plots_dynamic/${TEST}/bandwidth_read_memcpy_zoomed.pdf

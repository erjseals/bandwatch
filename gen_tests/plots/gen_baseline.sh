#! /usr/bin/env bash 

mkdir -p plots_baseline

gnuplot -c gnuplot_baseline.scr ../tests_baseline/res/disparity_baselines/parsed_trace_disparity_isolated.txt > plots_baseline/disparity.pdf
#gnuplot -c gnuplot_dynamic.scr ../tests_dynamic/res/disparity/memcpy/parsed_trace_dynamic_memcpy.txt > plots_dynamic/memcpy.pdf

gnuplot -c gnuplot_baseline_zoomed.scr ../tests_baseline/res/disparity_baselines/parsed_trace_disparity_isolated.txt > plots_baseline/disparity_zoomed.pdf
#gnuplot -c gnuplot_dynamic_zoomed.scr ../tests_dynamic/res/disparity/memcpy/parsed_trace_dynamic_memcpy.txt > plots_dynamic/memcpy_zoomed.pdf



gnuplot -c gnuplot_baseline.scr ../tests_baseline/res/disparity_baselines/parsed_trace_disparity_vs_memcpy.txt > plots_baseline/disparity_vs_memcpy.pdf
gnuplot -c gnuplot_baseline_zoomed.scr ../tests_baseline/res/disparity_baselines/parsed_trace_disparity_vs_memcpy.txt > plots_baseline/disparity_vs_memcpy_zoomed.pdf

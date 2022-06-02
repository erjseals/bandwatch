#! /usr/bin/env bash 

TEST="disparity"
mkdir -p plots_baseline/${TEST}
mkdir -p plots_baseline/hesoc

# Full Figures

gnuplot -c gnuplot_baseline.scr ../tests_baseline/res/${TEST}_baselines/parsed_trace_${TEST}_isolated.txt > plots_baseline/${TEST}/${TEST}_isolated.pdf

gnuplot -c gnuplot_baseline.scr ../tests_baseline/res/${TEST}_baselines/parsed_trace_${TEST}_vs_bandwidth_read_memcpy.txt > plots_baseline/${TEST}/${TEST}_vs_bandwidth_read_memcpy.pdf

gnuplot -c gnuplot_baseline.scr ../tests_baseline/res/${TEST}_baselines/parsed_trace_${TEST}_vs_bandwidth_read.txt > plots_baseline/${TEST}/${TEST}_vs_bandwidth_read.pdf

gnuplot -c gnuplot_baseline.scr ../tests_baseline/res/${TEST}_baselines/parsed_trace_${TEST}_vs_bandwidth_write_memset.txt > plots_baseline/${TEST}/${TEST}_vs_bandwidth_write_memset.pdf

gnuplot -c gnuplot_baseline.scr ../tests_baseline/res/${TEST}_baselines/parsed_trace_${TEST}_vs_bandwidth_write.txt > plots_baseline/${TEST}/${TEST}_vs_bandwidth_write.pdf

gnuplot -c gnuplot_baseline.scr ../tests_baseline/res/${TEST}_baselines/parsed_trace_${TEST}_vs_memcpy.txt > plots_baseline/${TEST}/${TEST}_vs_memcpy.pdf

gnuplot -c gnuplot_baseline.scr ../tests_baseline/res/${TEST}_baselines/parsed_trace_${TEST}_vs_memset.txt > plots_baseline/${TEST}/${TEST}_vs_memset.pdf


# Zoomed in Sections

gnuplot -c gnuplot_baseline_zoomed.scr ../tests_baseline/res/${TEST}_baselines/parsed_trace_${TEST}_isolated.txt > plots_baseline/${TEST}/${TEST}_isolated_zoomed.pdf


# Hesoc Benchmarks

gnuplot -c gnuplot_baseline.scr ../tests_baseline/res/hesoc_isolated/parsed_trace_memset.txt > plots_baseline/hesoc/memset_isolated.pdf

gnuplot -c gnuplot_baseline.scr ../tests_baseline/res/hesoc_isolated/parsed_trace_memcpy.txt > plots_baseline/hesoc/memcpy_isolated.pdf

gnuplot -c gnuplot_baseline.scr ../tests_baseline/res/hesoc_isolated/parsed_trace_memset_sync.txt > plots_baseline/hesoc/memset_isolated_sync.pdf

gnuplot -c gnuplot_baseline.scr ../tests_baseline/res/hesoc_isolated/parsed_trace_memcpy_sync.txt > plots_baseline/hesoc/memcpy_isolated_sync.pdf

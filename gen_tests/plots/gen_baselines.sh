#! /usr/bin/env bash 

TEST="mser"
mkdir -p plots_baseline/${TEST}
mkdir -p plots_baseline/hesoc
mkdir -p plots_baseline/bandwidth

# Full Figures

#  gnuplot -c gnuplot_baseline.scr ../tests_baseline/res/${TEST}_baselines/parsed_trace_${TEST}_isolated.txt > plots_baseline/${TEST}/${TEST}_isolated.pdf

TEST="disparity"
mkdir -p plots_baseline/${TEST}
gnuplot -c gnuplot_baseline.scr ../tests_baseline/res/${TEST}_baselines/parsed_trace_${TEST}_isolated.txt > plots_baseline/${TEST}/${TEST}_isolated.pdf
gnuplot -c gnuplot_baseline_zoomed.scr ../tests_baseline/res/${TEST}_baselines/parsed_trace_${TEST}_isolated.txt > plots_baseline/${TEST}/${TEST}_isolated_zoomed.pdf

TEST="mser"
mkdir -p plots_baseline/${TEST}
gnuplot -c gnuplot_baseline.scr ../tests_baseline/res/${TEST}_baselines/parsed_trace_${TEST}_isolated.txt > plots_baseline/${TEST}/${TEST}_isolated.pdf
gnuplot -c gnuplot_baseline_zoomed.scr ../tests_baseline/res/${TEST}_baselines/parsed_trace_${TEST}_isolated.txt > plots_baseline/${TEST}/${TEST}_isolated_zoomed.pdf

TEST="sift"
mkdir -p plots_baseline/${TEST}
gnuplot -c gnuplot_baseline.scr ../tests_baseline/res/${TEST}_baselines/parsed_trace_${TEST}_isolated.txt > plots_baseline/${TEST}/${TEST}_isolated.pdf
gnuplot -c gnuplot_baseline_zoomed.scr ../tests_baseline/res/${TEST}_baselines/parsed_trace_${TEST}_isolated.txt > plots_baseline/${TEST}/${TEST}_isolated_zoomed.pdf

TEST="texture_synthesis"
mkdir -p plots_baseline/${TEST}
gnuplot -c gnuplot_baseline.scr ../tests_baseline/res/${TEST}_baselines/parsed_trace_${TEST}_isolated.txt > plots_baseline/${TEST}/${TEST}_isolated.pdf
gnuplot -c gnuplot_baseline_zoomed.scr ../tests_baseline/res/${TEST}_baselines/parsed_trace_${TEST}_isolated.txt > plots_baseline/${TEST}/${TEST}_isolated_zoomed.pdf

TEST="tracking"
mkdir -p plots_baseline/${TEST}
gnuplot -c gnuplot_baseline.scr ../tests_baseline/res/${TEST}_baselines/parsed_trace_${TEST}_isolated.txt > plots_baseline/${TEST}/${TEST}_isolated.pdf
gnuplot -c gnuplot_baseline_zoomed.scr ../tests_baseline/res/${TEST}_baselines/parsed_trace_${TEST}_isolated.txt > plots_baseline/${TEST}/${TEST}_isolated_zoomed.pdf



if [[ 0 == 1 ]]
then

  gnuplot -c gnuplot_baseline.scr ../tests_baseline/res/${TEST}_baselines/parsed_trace_${TEST}_isolated.txt > plots_baseline/${TEST}/${TEST}_isolated.pdf

  gnuplot -c gnuplot_baseline.scr ../tests_baseline/res/${TEST}_baselines/parsed_trace_${TEST}_vs_bandwidth_read_memcpy.txt > plots_baseline/${TEST}/${TEST}_vs_bandwidth_read_memcpy.pdf

  gnuplot -c gnuplot_baseline.scr ../tests_baseline/res/${TEST}_baselines/parsed_trace_${TEST}_vs_bandwidth_read.txt > plots_baseline/${TEST}/${TEST}_vs_bandwidth_read.pdf

  gnuplot -c gnuplot_baseline.scr ../tests_baseline/res/${TEST}_baselines/parsed_trace_${TEST}_vs_bandwidth_write_memset.txt > plots_baseline/${TEST}/${TEST}_vs_bandwidth_write_memset.pdf

  gnuplot -c gnuplot_baseline.scr ../tests_baseline/res/${TEST}_baselines/parsed_trace_${TEST}_vs_bandwidth_write.txt > plots_baseline/${TEST}/${TEST}_vs_bandwidth_write.pdf

  gnuplot -c gnuplot_baseline.scr ../tests_baseline/res/${TEST}_baselines/parsed_trace_${TEST}_vs_memcpy.txt > plots_baseline/${TEST}/${TEST}_vs_memcpy.pdf

  gnuplot -c gnuplot_baseline.scr ../tests_baseline/res/${TEST}_baselines/parsed_trace_${TEST}_vs_memset.txt > plots_baseline/${TEST}/${TEST}_vs_memset.pdf


  # Zoomed in Sections

  gnuplot -c gnuplot_baseline_zoomed.scr ../tests_baseline/res/${TEST}_baselines/parsed_trace_${TEST}_isolated.txt > plots_baseline/${TEST}/${TEST}_isolated_zoomed.pdf

  gnuplot -c gnuplot_baseline_zoomed.scr ../tests_baseline/res/${TEST}_baselines/parsed_trace_${TEST}_vs_bandwidth_read_memcpy.txt > plots_baseline/${TEST}/${TEST}_vs_bandwidth_read_memcpy_zoomed.pdf

  gnuplot -c gnuplot_baseline_zoomed.scr ../tests_baseline/res/${TEST}_baselines/parsed_trace_${TEST}_vs_bandwidth_read.txt > plots_baseline/${TEST}/${TEST}_vs_bandwidth_read_zoomed.pdf

  gnuplot -c gnuplot_baseline_zoomed.scr ../tests_baseline/res/${TEST}_baselines/parsed_trace_${TEST}_vs_bandwidth_write_memset.txt > plots_baseline/${TEST}/${TEST}_vs_bandwidth_write_memset_zoomed.pdf

  gnuplot -c gnuplot_baseline_zoomed.scr ../tests_baseline/res/${TEST}_baselines/parsed_trace_${TEST}_vs_bandwidth_write.txt > plots_baseline/${TEST}/${TEST}_vs_bandwidth_write_zoomed.pdf

  gnuplot -c gnuplot_baseline_zoomed.scr ../tests_baseline/res/${TEST}_baselines/parsed_trace_${TEST}_vs_memcpy.txt > plots_baseline/${TEST}/${TEST}_vs_memcpy_zoomed.pdf

  gnuplot -c gnuplot_baseline_zoomed.scr ../tests_baseline/res/${TEST}_baselines/parsed_trace_${TEST}_vs_memset.txt > plots_baseline/${TEST}/${TEST}_vs_memset_zoomed.pdf
fi

# Hesoc Benchmarks

if [[ 0 == 1 ]]
then
  gnuplot -c gnuplot_baseline.scr ../tests_baseline/res/hesoc_isolated/parsed_trace_memset.txt > plots_baseline/hesoc/memset_isolated.pdf

  gnuplot -c gnuplot_baseline.scr ../tests_baseline/res/hesoc_isolated/parsed_trace_memcpy.txt > plots_baseline/hesoc/memcpy_isolated.pdf

  gnuplot -c gnuplot_baseline.scr ../tests_baseline/res/hesoc_isolated/parsed_trace_memset_sync.txt > plots_baseline/hesoc/memset_isolated_sync.pdf

  gnuplot -c gnuplot_baseline.scr ../tests_baseline/res/hesoc_isolated/parsed_trace_memcpy_sync.txt > plots_baseline/hesoc/memcpy_isolated_sync.pdf
fi

# Bandwidth Benchmarks

if [[ 1 == 1 ]]
then
  gnuplot -c gnuplot_baseline.scr ../tests_baseline/res/bandwidth_baselines/parsed_trace_bandwidth_read_isolated.txt > plots_baseline/bandwidth/bandwidth_read_isolated.pdf
  
  gnuplot -c gnuplot_baseline_zoomed.scr ../tests_baseline/res/bandwidth_baselines/parsed_trace_bandwidth_read_isolated.txt > plots_baseline/bandwidth/zoomed_bandwidth_read_isolated.pdf

  gnuplot -c gnuplot_baseline.scr ../tests_baseline/res/bandwidth_baselines/parsed_trace_bandwidth_write_isolated.txt > plots_baseline/bandwidth/bandwidth_write_isolated.pdf
  
  gnuplot -c gnuplot_baseline_zoomed.scr ../tests_baseline/res/bandwidth_baselines/parsed_trace_bandwidth_write_isolated.txt > plots_baseline/bandwidth/zoomed_bandwidth_write_isolated.pdf

  gnuplot -c gnuplot_baseline.scr ../tests_baseline/res/bandwidth_baselines/parsed_trace_bandwidth01_isolated.txt > plots_baseline/bandwidth/bandwidth01_isolated.pdf

  gnuplot -c gnuplot_baseline.scr ../tests_baseline/res/bandwidth_baselines/parsed_trace_bandwidth123_isolated.txt > plots_baseline/bandwidth/bandwidth123_isolated.pdf

  gnuplot -c gnuplot_baseline.scr ../tests_baseline/res/bandwidth_baselines/parsed_trace_bandwidth0_vs_memset.txt > plots_baseline/bandwidth/bandwidth0_vs_memset.pdf

  gnuplot -c gnuplot_baseline.scr ../tests_baseline/res/bandwidth_baselines/parsed_trace_bandwidth0_vs_memcpy.txt > plots_baseline/bandwidth/bandwidth0_vs_memcpy.pdf

  gnuplot -c gnuplot_baseline.scr ../tests_baseline/res/bandwidth_baselines/parsed_trace_bandwidth123_vs_memset.txt > plots_baseline/bandwidth/bandwidth123_vs_memset.pdf

  gnuplot -c gnuplot_baseline.scr ../tests_baseline/res/bandwidth_baselines/parsed_trace_bandwidth123_vs_memcpy.txt > plots_baseline/bandwidth/bandwidth123_vs_memcpy.pdf
fi

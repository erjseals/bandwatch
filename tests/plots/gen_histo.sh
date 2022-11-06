#! /usr/bin/env bash 

mkdir -p plots_evaluation/

gnuplot -c histoGPU.scr > plots_evaluation/vsGPU.pdf

gnuplot -c histoCPU.scr > plots_evaluation/vsCPU.pdf

gnuplot -c histoCPUGPU.scr > plots_evaluation/vsCPUGPU.pdf

gnuplot -c histomemset.scr > plots_evaluation/bwmemset.pdf

gnuplot -c histomemcpy.scr > plots_evaluation/bwmemcpy.pdf

gnuplot -c histoCPUBW.scr > plots_evaluation/bwCPU.pdf

gnuplot -c histogpu_CPUGPUBW.scr > plots_evaluation/bwgpu_CPUGPU.pdf

gnuplot -c histocpu_CPUGPUBW.scr > plots_evaluation/bwcpu_CPUGPU.pdf

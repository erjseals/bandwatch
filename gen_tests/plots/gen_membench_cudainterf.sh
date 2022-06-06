#! /usr/bin/env bash 

TEST="membench_cudainterf"

mkdir -p plots_membench_cudainterf/

gnuplot -c membench_cudainterf_seq.scr > plots_membench_cudainterf/seq.pdf

gnuplot -c membench_cudainterf_rnd.scr > plots_membench_cudainterf/rnd.pdf

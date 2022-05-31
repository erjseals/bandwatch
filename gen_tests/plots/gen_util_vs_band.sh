
TEST="disparity"
mkdir -p plots_baseline/${TEST}

gnuplot -c gnuplot.scr ../res/${TEST}_baselines/parsed_trace_${TEST}_isolated.txt > plots_baseline/${TEST}/${TEST}_isolated.pdf

gnuplot -c gnuplot.scr ../res/${TEST}_baselines/parsed_trace_${TEST}_vs_bandwidth_read_memcpy.txt > plots_baseline/${TEST}/${TEST}_vs_bandwidth_read_memcpy.pdf

gnuplot -c gnuplot.scr ../res/${TEST}_baselines/parsed_trace_${TEST}_vs_bandwidth_read.txt > plots_baseline/${TEST}/${TEST}_vs_bandwidth_read.pdf

gnuplot -c gnuplot.scr ../res/${TEST}_baselines/parsed_trace_${TEST}_vs_bandwidth_write_memset.txt > plots_baseline/${TEST}/${TEST}_vs_bandwidth_write_memset.pdf

gnuplot -c gnuplot.scr ../res/${TEST}_baselines/parsed_trace_${TEST}_vs_bandwidth_write.txt > plots_baseline/${TEST}/${TEST}_vs_bandwidth_write.pdf

gnuplot -c gnuplot.scr ../res/${TEST}_baselines/parsed_trace_${TEST}_vs_memcpy.txt > plots_baseline/${TEST}/${TEST}_vs_memcpy.pdf

gnuplot -c gnuplot.scr ../res/${TEST}_baselines/parsed_trace_${TEST}_vs_memset.txt > plots_baseline/${TEST}/${TEST}_vs_memset.pdf

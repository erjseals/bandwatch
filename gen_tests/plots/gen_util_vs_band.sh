gnuplot -c gnuplot.scr ../tests_uSec/parsed_trace_DisparityIsolated.txt > util_vs_band/DisparityIsolated.pdf

gnuplot -c gnuplot.scr ../tests_uSec/parsed_trace_Disparity_vs_bandwidthRead_hesocMemcpy.txt > util_vs_band/Disparity_vs_bandwidthRead_hesocMemcpy.pdf

gnuplot -c gnuplot.scr ../tests_uSec/parsed_trace_Disparity_vs_bandwidthRead.txt > util_vs_band/Disparity_vs_bandwidthRead.pdf

gnuplot -c gnuplot.scr ../tests_uSec/parsed_trace_Disparity_vs_bandwidthWrite_hesocMemset.txt > util_vs_band/Disparity_vs_bandwidthWrite_hesocMemset.pdf

gnuplot -c gnuplot.scr ../tests_uSec/parsed_trace_Disparity_vs_bandwidthWrite.txt > util_vs_band/Disparity_vs_bandwidthWrite.pdf

gnuplot -c gnuplot.scr ../tests_uSec/parsed_trace_Disparity_vs_hesocMemcpy.txt > util_vs_band/Disparity_vs_hesocMemcpy.pdf

gnuplot -c gnuplot.scr ../tests_uSec/parsed_trace_Disparity_vs_hesocMemset.txt > util_vs_band/Disparity_vs_hesocMemset.pdf

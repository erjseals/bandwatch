# This conducts all baseline (motivation) tests with disparity benchmark

# Baseline
sudo insmod ../patched_memguard/memguard.ko
sudo echo 0 > /sys/kernel/debug/tracing/trace

taskset -c 0 ../benchmarks/sd-vbs/benchmarks/disparity/data/fullhd/disparity ../benchmarks/sd-vbs/benchmarks/disparity/data/fullhd/. | grep "Cycles elapsed" >> SoloRunCycles.txt & PID_TO_WAIT=$!

wait $PID_TO_WAIT
sudo rmmod memguard
sudo cat /sys/kernel/debug/tracing/trace > SoloRunTrace.txt 

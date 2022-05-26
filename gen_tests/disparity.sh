# This conducts all baseline (motivation) tests with disparity benchmark


TEST="Disparity"
INTERF=" "

sysctl -w kernel.sched_rt_runtime_us=-1

# Make Memguard (if necessary)
echo "Building Memguard"
(cd ../patched_memguard && make)


echo "$TEST in Isolation"
INTERF="Isolated"

sudo insmod ../patched_memguard/memguard.ko g_hw_counter_id=0x17
sudo echo 0 > /sys/kernel/debug/tracing/trace

taskset -c 0 ../benchmarks/sd-vbs/benchmarks/disparity/data/fullhd/disparity ../benchmarks/sd-vbs/benchmarks/disparity/data/fullhd/. | grep "Cycles elapsed" > cycles_${TEST}${INTERF}.txt & PID_TO_WAIT=$!

wait $PID_TO_WAIT
sudo rmmod memguard
sudo cat /sys/kernel/debug/tracing/trace > trace_${TEST}${INTERF}.txt 

python3 splitftrace.py trace_${TEST}${INTERF}.txt




sleep 5





INTERF="hesocMemcpy"
echo "$TEST against $INTERF"

sudo insmod ../patched_memguard/memguard.ko g_hw_counter_id=0x17

sudo taskset -c 0 ../benchmarks/hesoc-mark/cuda/cudainterf -d 102400 -i 20000 -m memcpy & PID_TO_KILL0=$!

sudo echo 0 > /sys/kernel/debug/tracing/trace

taskset -c 0 ../benchmarks/sd-vbs/benchmarks/disparity/data/fullhd/disparity ../benchmarks/sd-vbs/benchmarks/disparity/data/fullhd/. | grep "Cycles elapsed" > cycles_${TEST}_vs_${INTERF}.txt & PID_TO_WAIT=$!

wait $PID_TO_WAIT
sudo rmmod memguard

sudo killall -9 cudainterf
sudo kill -9 $PID_TO_KILL0

sudo cat /sys/kernel/debug/tracing/trace > trace_${TEST}_vs_${INTERF}.txt 

python3 splitftrace.py trace_${TEST}_vs_${INTERF}.txt





sleep 5




INTERF="hesocMemset"
echo "$TEST against $INTERF"

sudo insmod ../patched_memguard/memguard.ko g_hw_counter_id=0x17

sudo taskset -c 0 ../benchmarks/hesoc-mark/cuda/cudainterf -d 102400 -i 20000 -m memset & PID_TO_KILL0=$!

sudo echo 0 > /sys/kernel/debug/tracing/trace

taskset -c 0 ../benchmarks/sd-vbs/benchmarks/disparity/data/fullhd/disparity ../benchmarks/sd-vbs/benchmarks/disparity/data/fullhd/. | grep "Cycles elapsed" > cycles_${TEST}_vs_${INTERF}.txt & PID_TO_WAIT=$!

wait $PID_TO_WAIT
sudo rmmod memguard

sudo killall -9 cudainterf
sudo kill -9 $PID_TO_KILL0

sudo cat /sys/kernel/debug/tracing/trace > trace_${TEST}_vs_${INTERF}.txt 

python3 splitftrace.py trace_${TEST}_vs_${INTERF}.txt




sleep 5





INTERF="bandwidthRead"
echo "$TEST against $INTERF"

sudo insmod ../patched_memguard/memguard.ko g_hw_counter_id=0x17

for c in 1 2 3; do bandwidth -c $c -t 1000 & done

sudo echo 0 > /sys/kernel/debug/tracing/trace

taskset -c 0 ../benchmarks/sd-vbs/benchmarks/disparity/data/fullhd/disparity ../benchmarks/sd-vbs/benchmarks/disparity/data/fullhd/. | grep "Cycles elapsed" > cycles_${TEST}_vs_${INTERF}.txt & PID_TO_WAIT=$!

wait $PID_TO_WAIT
sudo rmmod memguard

sudo killall -9 bandwidth

sudo cat /sys/kernel/debug/tracing/trace > trace_${TEST}_vs_${INTERF}.txt 

python3 splitftrace.py trace_${TEST}_vs_${INTERF}.txt





sleep 5





INTERF="bandwidthWrite"
echo "$TEST against $INTERF"

sudo insmod ../patched_memguard/memguard.ko g_hw_counter_id=0x17

for c in 1 2 3; do bandwidth -a write -c $c -t 1000 & done

sudo echo 0 > /sys/kernel/debug/tracing/trace

taskset -c 0 ../benchmarks/sd-vbs/benchmarks/disparity/data/fullhd/disparity ../benchmarks/sd-vbs/benchmarks/disparity/data/fullhd/. | grep "Cycles elapsed" > cycles_${TEST}_vs_${INTERF}.txt & PID_TO_WAIT=$!

wait $PID_TO_WAIT
sudo rmmod memguard

sudo killall -9 bandwidth

sudo cat /sys/kernel/debug/tracing/trace > trace_${TEST}_vs_${INTERF}.txt 

python3 splitftrace.py trace_${TEST}_vs_${INTERF}.txt






sleep 5






INTERF="bandwidthRead_hesocMemcpy"
echo "$TEST against $INTERF"

sudo insmod ../patched_memguard/memguard.ko g_hw_counter_id=0x17

for c in 1 2 3; do bandwidth -c $c -t 1000 & done
sudo taskset -c 0 ../benchmarks/hesoc-mark/cuda/cudainterf -d 102400 -i 20000 -m memcpy & PID_TO_KILL0=$!

sudo echo 0 > /sys/kernel/debug/tracing/trace

taskset -c 0 ../benchmarks/sd-vbs/benchmarks/disparity/data/fullhd/disparity ../benchmarks/sd-vbs/benchmarks/disparity/data/fullhd/. | grep "Cycles elapsed" > cycles_${TEST}_vs_${INTERF}.txt & PID_TO_WAIT=$!

wait $PID_TO_WAIT
sudo rmmod memguard

sudo killall -9 bandwidth
sudo killall -9 cudainterf
sudo kill -9 $PID_TO_KILL0

sudo cat /sys/kernel/debug/tracing/trace > trace_${TEST}_vs_${INTERF}.txt 

python3 splitftrace.py trace_${TEST}_vs_${INTERF}.txt





sleep 5







INTERF="bandwidthWrite_hesocMemset"
echo "$TEST against $INTERF"

sudo insmod ../patched_memguard/memguard.ko g_hw_counter_id=0x17

for c in 1 2 3; do bandwidth -a write -c $c -t 1000 & done 
sudo taskset -c 0 ../benchmarks/hesoc-mark/cuda/cudainterf -d 102400 -i 20000 -m memset & PID_TO_KILL0=$!

sudo echo 0 > /sys/kernel/debug/tracing/trace

taskset -c 0 ../benchmarks/sd-vbs/benchmarks/disparity/data/fullhd/disparity ../benchmarks/sd-vbs/benchmarks/disparity/data/fullhd/. | grep "Cycles elapsed" > cycles_${TEST}_vs_${INTERF}.txt & PID_TO_WAIT=$!

wait $PID_TO_WAIT
sudo rmmod memguard

sudo killall -9 bandwidth
sudo killall -9 cudainterf
sudo kill -9 $PID_TO_KILL0

sudo cat /sys/kernel/debug/tracing/trace > trace_${TEST}_vs_${INTERF}.txt 

python3 splitftrace.py trace_${TEST}_vs_${INTERF}.txt










mv *.txt tests/
echo "done"

(cd ../patched_memguard && make clean)

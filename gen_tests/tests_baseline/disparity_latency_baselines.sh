# This conducts all baseline (motivation) tests with disparity benchmark


TEST="disparity"
INTERF=" "

mkdir -p res/${TEST}_latency_baselines

sysctl -w kernel.sched_rt_runtime_us=-1

# Set GPU priority higher
sudo /bin/busybox devmem 0x700193ac 32 0x00800004
sudo /bin/busybox devmem 0x700193e8 32 0x00800004


INTERF="isolated"
echo "$TEST in $INTERF"

sudo insmod ../../patched_memguard/memguard.ko g_hw_counter_id=0x17
sleep 2

sudo echo 0 > /sys/kernel/debug/tracing/trace

taskset -c 0 ../../benchmarks/sd-vbs/benchmarks/${TEST}/data/fullhd/${TEST} ../../benchmarks/sd-vbs/benchmarks/${TEST}/data/fullhd/. | grep "Cycles elapsed" > cycles_${TEST}_${INTERF}.txt & PID_TO_WAIT=$!

wait $PID_TO_WAIT
sudo cat /sys/kernel/debug/tracing/trace > trace_${TEST}_${INTERF}.txt 
sudo rmmod memguard

python3 splitftrace.py trace_${TEST}_${INTERF}.txt

echo "done"
sleep 10





INTERF="memcpy"
echo "$TEST against $INTERF"

sudo insmod ../../patched_memguard/memguard.ko g_hw_counter_id=0x17
sleep 2

sudo echo 0 > /sys/kernel/debug/memguard/throttle
sleep 2

sudo taskset -c 2 ../../benchmarks/hesoc-mark/cuda/cudainterf -d 102400 -s --iterations=20000 --mode=memcpy & PID_TO_KILL0=$!
sleep 1

PID_TO_KILL=$(pgrep cudainterf)
sudo kill -s SIGUSR1 $PID_TO_KILL
sudo echo 0 > /sys/kernel/debug/tracing/trace

taskset -c 0 ../../benchmarks/sd-vbs/benchmarks/${TEST}/data/fullhd/${TEST} ../../benchmarks/sd-vbs/benchmarks/${TEST}/data/fullhd/. | grep "Cycles elapsed" > cycles_${TEST}_vs_${INTERF}_high.txt & PID_TO_WAIT=$!

wait $PID_TO_WAIT
sudo cat /sys/kernel/debug/tracing/trace > trace_${TEST}_vs_${INTERF}_high.txt 
sudo kill -s SIGUSR2 $PID_TO_KILL
sudo rmmod memguard

sudo kill -9 $PID_TO_KILL

python3 splitftrace.py trace_${TEST}_vs_${INTERF}_high.txt

echo "done"
sleep 10




INTERF="memset"
echo "$TEST against $INTERF"

sudo insmod ../../patched_memguard/memguard.ko g_hw_counter_id=0x17
sleep 2

sudo echo 0 > /sys/kernel/debug/memguard/throttle
sleep 2

sudo taskset -c 2 ../../benchmarks/hesoc-mark/cuda/cudainterf -s -d 102400 -i 20000 -m memset & PID_TO_KILL0=$!
sleep 1

PID_TO_KILL=$(pgrep cudainterf)
sudo kill -s SIGUSR1 $PID_TO_KILL

sudo echo 0 > /sys/kernel/debug/tracing/trace

taskset -c 0 ../../benchmarks/sd-vbs/benchmarks/${TEST}/data/fullhd/${TEST} ../../benchmarks/sd-vbs/benchmarks/${TEST}/data/fullhd/. | grep "Cycles elapsed" > cycles_${TEST}_vs_${INTERF}_high.txt & PID_TO_WAIT=$!

wait $PID_TO_WAIT
sudo cat /sys/kernel/debug/tracing/trace > trace_${TEST}_vs_${INTERF}_high.txt 

sudo kill -s SIGUSR2 $PID_TO_KILL
sudo rmmod memguard

sudo kill -9 $PID_TO_KILL

python3 splitftrace.py trace_${TEST}_vs_${INTERF}_high.txt

echo "done"
sleep 10

# Set GPU priority higher
sudo /bin/busybox devmem 0x700193ac 32 0x00800019
sudo /bin/busybox devmem 0x700193e8 32 0x00800019

INTERF="memcpy"
echo "$TEST against $INTERF"

sudo insmod ../../patched_memguard/memguard.ko g_hw_counter_id=0x17
sleep 2

sudo echo 0 > /sys/kernel/debug/memguard/throttle
sleep 2

sudo taskset -c 2 ../../benchmarks/hesoc-mark/cuda/cudainterf -d 102400 -s --iterations=20000 --mode=memcpy & PID_TO_KILL0=$!
sleep 1

PID_TO_KILL=$(pgrep cudainterf)
sudo kill -s SIGUSR1 $PID_TO_KILL
sudo echo 0 > /sys/kernel/debug/tracing/trace

taskset -c 0 ../../benchmarks/sd-vbs/benchmarks/${TEST}/data/fullhd/${TEST} ../../benchmarks/sd-vbs/benchmarks/${TEST}/data/fullhd/. | grep "Cycles elapsed" > cycles_${TEST}_vs_${INTERF}.txt & PID_TO_WAIT=$!

wait $PID_TO_WAIT
sudo cat /sys/kernel/debug/tracing/trace > trace_${TEST}_vs_${INTERF}.txt 
sudo kill -s SIGUSR2 $PID_TO_KILL
sudo rmmod memguard

sudo kill -9 $PID_TO_KILL

python3 splitftrace.py trace_${TEST}_vs_${INTERF}.txt

echo "done"
sleep 10




INTERF="memset"
echo "$TEST against $INTERF"

sudo insmod ../../patched_memguard/memguard.ko g_hw_counter_id=0x17
sleep 2

sudo echo 0 > /sys/kernel/debug/memguard/throttle
sleep 2

sudo taskset -c 2 ../../benchmarks/hesoc-mark/cuda/cudainterf -s -d 102400 -i 20000 -m memset & PID_TO_KILL0=$!
sleep 1

PID_TO_KILL=$(pgrep cudainterf)
sudo kill -s SIGUSR1 $PID_TO_KILL

sudo echo 0 > /sys/kernel/debug/tracing/trace

taskset -c 0 ../../benchmarks/sd-vbs/benchmarks/${TEST}/data/fullhd/${TEST} ../../benchmarks/sd-vbs/benchmarks/${TEST}/data/fullhd/. | grep "Cycles elapsed" > cycles_${TEST}_vs_${INTERF}.txt & PID_TO_WAIT=$!

wait $PID_TO_WAIT
sudo cat /sys/kernel/debug/tracing/trace > trace_${TEST}_vs_${INTERF}.txt 

sudo kill -s SIGUSR2 $PID_TO_KILL
sudo rmmod memguard

sudo kill -9 $PID_TO_KILL

python3 splitftrace.py trace_${TEST}_vs_${INTERF}.txt

echo "done"
sleep 10


mv *.txt res/${TEST}_latency_baselines
echo "done"


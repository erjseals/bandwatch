# /usr/bin/env bash 

TEST="disparity"

mkdir -p res 
mkdir -p res/${TEST}/memset
mkdir -p res/${TEST}/memcpy

sysctl -w kernel.sched_rt_runtime_us=-1 

#sudo insmod ../../patched_memguard/memguard.ko g_hw_counter_id=0x17
#sleep 1
#sudo echo 0 > /sys/kernel/debug/memguard/throttle
#sleep 1

#sudo rmmod memguard

#sudo /usr/local/cuda/bin/nvprof -f --profile-from-start off --device-buffer-size 32 --profiling-semaphore-pool-size 1131072 -o prof.nvvp ../../benchmarks/hesoc-mark/cuda/cudainterf -d 102400 --iterations=100000 --mode=memcpy &

#sleep 5
#PID_TO_KILL=$(pgrep cudainterf)
#sudo kill -s SIGUSR1 $PID_TO_KILL
#sleep .1
#sudo kill -s SIGUSR2 $PID_TO_KILL

#sleep 5

sudo ../../benchmarks/hesoc-mark/cuda/cudainterf -d 102400 --iterations=10000 --mode=memcpy &

sleep 5
PID_TO_KILL=$(pgrep cudainterf)
sudo kill -s SIGUSR1 $PID_TO_KILL
sleep 5
sudo kill -s SIGUSR2 $PID_TO_KILL


#sudo /usr/local/cuda/bin/nvprof --profile-from-start off --device-buffer-size 32 --profiling-semaphore-pool-size 1131072 --print-gpu-trace ../../benchmarks/hesoc-mark/cuda/cudainterf -d 102400 -s --iterations=100 --mode=memset



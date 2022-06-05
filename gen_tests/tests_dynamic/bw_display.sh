TEST="tracking"
BASE=5694343
echo "$TEST"

echo " "
cat ./res/${TEST}/bw_memset.txt
echo " "
cat ./res/${TEST}/bw_memcpy.txt
echo " "
cat ./res/${TEST}/bw_bandwidth_read.txt | grep "CPU1:" | awk '{ printf "%.1f\n", $4 }'
cat ./res/${TEST}/bw_bandwidth_read.txt | grep "CPU2:" | awk '{ printf "%.1f\n", $4 }'
cat ./res/${TEST}/bw_bandwidth_read.txt | grep "CPU3:" | awk '{ printf "%.1f\n", $4 }'
echo " "
cat ./res/${TEST}/bw_bandwidth_write.txt | grep "CPU1:" | awk '{ printf "%.1f\n", $4 }'
cat ./res/${TEST}/bw_bandwidth_write.txt | grep "CPU2:" | awk '{ printf "%.1f\n", $4 }'
cat ./res/${TEST}/bw_bandwidth_write.txt | grep "CPU3:" | awk '{ printf "%.1f\n", $4 }'
echo " "
echo "GPU for memcpy bwread"
cat ./res/${TEST}/bw_bandwidth_read_memcpy_gpu.txt
echo "CPU for memcpy bwread"
cat ./res/${TEST}/bw_bandwidth_read_memcpy_cpu.txt | grep "CPU1:" | awk '{ printf "%.1f\n", $4 }'
cat ./res/${TEST}/bw_bandwidth_read_memcpy_cpu.txt | grep "CPU2:" | awk '{ printf "%.1f\n", $4 }'
cat ./res/${TEST}/bw_bandwidth_read_memcpy_cpu.txt | grep "CPU3:" | awk '{ printf "%.1f\n", $4 }'
echo " "
echo "GPU for memset bwwrite"
cat ./res/${TEST}/bw_bandwidth_write_memset_gpu.txt
echo "CPU for memset bwwrite"
cat ./res/${TEST}/bw_bandwidth_write_memset_cpu.txt | grep "CPU1:" | awk '{ printf "%.1f\n", $4 }'
cat ./res/${TEST}/bw_bandwidth_write_memset_cpu.txt | grep "CPU2:" | awk '{ printf "%.1f\n", $4 }'
cat ./res/${TEST}/bw_bandwidth_write_memset_cpu.txt | grep "CPU3:" | awk '{ printf "%.1f\n", $4 }'

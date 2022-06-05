TEST="interf"

echo " "
cat ./res/${TEST}/bw_memset.txt
echo " "
cat ./res/${TEST}/bw_memcpy.txt
echo " "
cat ./res/${TEST}/bw_read.txt | grep "CPU1:" | awk '{ printf "%.1f\n", $4 }'
cat ./res/${TEST}/bw_read.txt | grep "CPU2:" | awk '{ printf "%.1f\n", $4 }'
cat ./res/${TEST}/bw_read.txt | grep "CPU3:" | awk '{ printf "%.1f\n", $4 }'
echo " "
cat ./res/${TEST}/bw_write.txt | grep "CPU1:" | awk '{ printf "%.1f\n", $4 }'
cat ./res/${TEST}/bw_write.txt | grep "CPU2:" | awk '{ printf "%.1f\n", $4 }'
cat ./res/${TEST}/bw_write.txt | grep "CPU3:" | awk '{ printf "%.1f\n", $4 }'

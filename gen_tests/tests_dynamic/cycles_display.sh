TEST="disparity"
BASE=5694343
echo "$TEST"

cat ./res/${TEST}/cycles_memset.txt | awk '{ printf "%.2f\n", $4 / 5694343 }'
cat ./res/${TEST}/cycles_memcpy.txt | awk '{ printf "%.2f\n", $4 / 5694343 }'
cat ./res/${TEST}/cycles_${TEST}_vs_bandwidth_read.txt | awk '{ printf "%.2f\n", $4 / 5694343 }'
cat ./res/${TEST}/cycles_${TEST}_vs_bandwidth_write.txt | awk '{ printf "%.2f\n", $4 / 5694343 }'
cat ./res/${TEST}/cycles_${TEST}_vs_bandwidth_read_memcpy.txt | awk '{ printf "%.2f\n", $4 / 5694343 }'
cat ./res/${TEST}/cycles_${TEST}_vs_bandwidth_write_memset.txt | awk '{ printf "%.2f\n", $4 / 5694343 }'

TEST="mser"
BASE=1530432
echo "$TEST"

cat ./res/${TEST}/cycles_memset.txt | awk '{ printf "%.2f\n", $4 / 1530432 }'
cat ./res/${TEST}/cycles_memcpy.txt | awk '{ printf "%.2f\n", $4 / 1530432 }'
cat ./res/${TEST}/cycles_${TEST}_vs_bandwidth_read.txt | awk '{ printf "%.2f\n", $4 / 1530432 }'
cat ./res/${TEST}/cycles_${TEST}_vs_bandwidth_write.txt | awk '{ printf "%.2f\n", $4 / 1530432 }'
cat ./res/${TEST}/cycles_${TEST}_vs_bandwidth_read_memcpy.txt | awk '{ printf "%.2f\n", $4 / 1530432 }'
cat ./res/${TEST}/cycles_${TEST}_vs_bandwidth_write_memset.txt | awk '{ printf "%.2f\n", $4 / 1530432 }'


TEST="sift"
BASE=5732308
echo "$TEST"

cat ./res/${TEST}/cycles_memset.txt | awk '{ printf "%.2f\n", $4 / 5732308 }'
cat ./res/${TEST}/cycles_memcpy.txt | awk '{ printf "%.2f\n", $4 / 5732308 }'
cat ./res/${TEST}/cycles_${TEST}_vs_bandwidth_read.txt | awk '{ printf "%.2f\n", $4 / 5732308 }'
cat ./res/${TEST}/cycles_${TEST}_vs_bandwidth_write.txt | awk '{ printf "%.2f\n", $4 / 5732308 }'
cat ./res/${TEST}/cycles_${TEST}_vs_bandwidth_read_memcpy.txt | awk '{ printf "%.2f\n", $4 / 5732308 }'
cat ./res/${TEST}/cycles_${TEST}_vs_bandwidth_write_memset.txt | awk '{ printf "%.2f\n", $4 / 5732308 }'


TEST="texture_synthesis"
BASE=42017746
echo "$TEST"

cat ./res/${TEST}/cycles_memset.txt | awk '{ printf "%.2f\n", $4 / 42017746 }'
cat ./res/${TEST}/cycles_memcpy.txt | awk '{ printf "%.2f\n", $4 / 42017746 }'
cat ./res/${TEST}/cycles_${TEST}_vs_bandwidth_read.txt | awk '{ printf "%.2f\n", $4 / 42017746 }'
cat ./res/${TEST}/cycles_${TEST}_vs_bandwidth_write.txt | awk '{ printf "%.2f\n", $4 / 42017746 }'
cat ./res/${TEST}/cycles_${TEST}_vs_bandwidth_read_memcpy.txt | awk '{ printf "%.2f\n", $4 / 42017746 }'
cat ./res/${TEST}/cycles_${TEST}_vs_bandwidth_write_memset.txt | awk '{ printf "%.2f\n", $4 / 42017746 }'


TEST="tracking"
BASE=1512741
echo "$TEST"

cat ./res/${TEST}/cycles_memset.txt | awk '{ printf "%.2f\n", $4 / 1512741 }'
cat ./res/${TEST}/cycles_memcpy.txt | awk '{ printf "%.2f\n", $4 / 1512741 }'
cat ./res/${TEST}/cycles_${TEST}_vs_bandwidth_read.txt | awk '{ printf "%.2f\n", $4 / 1512741 }'
cat ./res/${TEST}/cycles_${TEST}_vs_bandwidth_write.txt | awk '{ printf "%.2f\n", $4 / 1512741 }'
cat ./res/${TEST}/cycles_${TEST}_vs_bandwidth_read_memcpy.txt | awk '{ printf "%.2f\n", $4 / 1512741 }'
cat ./res/${TEST}/cycles_${TEST}_vs_bandwidth_write_memset.txt | awk '{ printf "%.2f\n", $4 / 1512741 }'

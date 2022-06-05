TEST="disparity"
BASE=5694343
echo "$TEST"

cat ./res/${TEST}/memset/cycles_memset.txt | awk '{ printf "%.2f\n", $4 }'
#cat ./res/${TEST}/memset/cycles_memset.txt | awk '{ printf "%.2f\n", $4 / 5694343 }'

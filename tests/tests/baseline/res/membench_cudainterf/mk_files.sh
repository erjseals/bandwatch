#! /usr/bin/env bash

declare -a THROTTLE=(9 10 11 12 13 14 15 17 18 19 20 21 22 23 25 26 27 28 29 30)

for i in "${THROTTLE[@]}"
do
  rm seq/seq_${i}.txt
  rm rnd/rnd_${i}.txt
done

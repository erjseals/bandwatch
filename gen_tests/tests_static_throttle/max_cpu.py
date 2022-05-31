#!/usr/bin/env python3

import sys

lines = []

with open( sys.argv[1] ) as fp:
    lines = fp.readlines()

# File Format
# time mc_all mc_cpu events throttle
max_all = 0
max_cpu = 0
for number, line in enumerate(lines):
    line = line.strip()
    line = " ".join(line.split())
    words = line.split()

    mc_all = int(words[1])
    mc_cpu = int(words[2])

    if (number == 0):
        max_all = mc_all
        max_cpu = mc_cpu 
    else:
        if (max_all < mc_all):
            max_all = mc_all 
        if (max_cpu < mc_cpu):
            max_cpu = mc_cpu 

print(max_all)
print(max_cpu)

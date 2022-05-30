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

    if (number == 0):
        max_all = words[1]
        max_cpu = words[2]
    else:
        if (max_all < words[1]):
            max_all = words[1]
        if (max_cpu < words[2]):
            max_cpu = words[2]

print(max_all)
print(max_cpu)

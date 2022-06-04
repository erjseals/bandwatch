#!/usr/bin/env python3

import sys

lines = []

with open( sys.argv[1] ) as fp:
    lines = fp.readlines()

# File Format
# time mc_all mc_cpu events throttle
max_all = 0
max_cpu = 0

count   = 0
avg_all = 0
avg_cpu = 0

max_events = 0
avg_events = 0

for number, line in enumerate(lines):
    line = line.strip()
    line = " ".join(line.split())
    words = line.split()

    mc_all = int(words[1])
    mc_cpu = int(words[2])

    temp   = int(words[3])

    avg_all += mc_all
    avg_cpu += mc_cpu
    avg_events += temp

    if (number == 0):
        max_all = mc_all
        max_cpu = mc_cpu 
        max_events = temp
    else:
        if (max_all < mc_all):
            max_all = mc_all 
        if (max_cpu < mc_cpu):
            max_cpu = mc_cpu 
        if (max_events < temp):
            max_events = temp
    count += 1

avg_all = avg_all / count
avg_cpu = avg_cpu / count
avg_events = avg_events / count

print("max_all:", max_all)
print("max_cpu:", max_cpu)
print("avg_all:", avg_all)
print("avg_cpu:", avg_cpu)

print("")
print("max_events:", max_events)

divisor = 1000*1024*1024
mb = max_events*64*1000000 + (divisor - 1)
mb = mb / divisor

avg_mb = avg_events*64*1000000 + (divisor - 1)
avg_mb = avg_mb / divisor

print("max_bw:", mb) 
print("avg_bw:", avg_mb)

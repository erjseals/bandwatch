#!/usr/bin/env python3

import sys

lines = []

with open( sys.argv[1] ) as fp:
    lines = fp.readlines()

with open( 'parsed_' + sys.argv[1], 'w' ) as fp:
    for number, line in enumerate(lines):
        # 11 lines of preamble for ftrace file
        if number > 11:
            line = line.strip()
            line = " ".join(line.split())
            words = line.split()
            fp.write(words[5])
            fp.write(" ")
            fp.write(words[6])
            fp.write(" ")
            fp.write(words[8])
            fp.write('\n')

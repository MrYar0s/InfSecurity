import sys
import os
import re
import argparse
import random

def convert_file(input, out):
    in_file = open(input, 'r')
    o_file = open(out, 'w')
    for line in in_file:
        line = line.strip()
        if line == '':
            continue
        sz = len(line)
        byte_to_invert = random.randrange(sz)
        val = line[byte_to_invert]
        val = ord(val)
        val = val ^ 1
        o_file.write(line[:byte_to_invert] + chr(val) + line[byte_to_invert+1:] + '\n')
    in_file.close()
    o_file.close()

if __name__ == '__main__' :
    parser = argparse.ArgumentParser(description='Collect metrics after processing dataset')
    parser.add_argument('--input', dest='input', help='input file', required=True)
    parser.add_argument('--output', dest='out', help='output file', required=True)
    args = parser.parse_args()
    convert_file(args.input, args.out)
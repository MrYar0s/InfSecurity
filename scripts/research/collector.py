import sys
import os
import re
import argparse
from multiprocessing import Pool

all_functions = ["sha256", "keccak", "lazy", "blake256", "gost"]

def collect_stats(program, file_path, func):
    print(f'Collecting files from {file_path} using {func}')
    os.system(f"{program} --in {file_path} --hash {func}")

def collect_stats_parallel(program, file_path):
    agents = len(all_functions)
    chunksize = 1
    arguments = [(program, file_path, func) for func in all_functions]
    with Pool(processes=agents) as pool:
        result = pool.starmap(collect_stats, arguments)

if __name__ == '__main__' :
    parser = argparse.ArgumentParser(description='Collect metrics after processing dataset')
    parser.add_argument('--dataset-file', dest='file_path', help='path to dataset', required=True)
    parser.add_argument('--name-of-program', dest='name', help='name of executed program', required=True)
    parser.add_argument('--execute-in-parallel', dest='is_parallel', help='specify if you want to collect data in parallel mode', default=False)
    args = parser.parse_args()
    if (args.is_parallel):
        collect_stats_parallel(args.name, args.file_path)
    else:
        for func in all_functions:
            collect_stats(args.name, args.file_path, func)
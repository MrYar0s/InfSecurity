import matplotlib.pyplot as plt
import argparse

all_functions = ["sha256", "keccak", "lazy", "blake256", "gost"]

if __name__ == '__main__' :
    parser = argparse.ArgumentParser(description='Collect metrics after processing dataset')
    parser.add_argument('--base-dataset-name', dest='dataset', help='path to raw dataset', required=True)
    parser.add_argument('--hash-stat-folder', dest='hash_folder', help='path to hash stat folder', required=True)
    args = parser.parse_args()
    for hash in all_functions:
        first_hash_path = args.hash_folder + '/hashes_for_' + hash + '_' + args.dataset + '.txt'
        second_hash_path = args.hash_folder + '/hashes_for_' + hash + '_converted_' + args.dataset + '.txt'
        first_hash = open(first_hash_path)
        second_hash = open(second_hash_path)
        list = []
        while True:
            line1 = first_hash.readline().strip()
            line2 = second_hash.readline().strip()
            if not line1 or not line2:
                break
            if len(line1) != len(line2):
                print("Error in size of hash function")
            num_of_diff_bits = 0
            for i in range(len(line1)):
                char1 = int(line1[i],16)
                char2 = int(line2[i],16)
                res = char1 ^ char2
                num_of_diff_bits += bin(res).count('1')
            list.append(num_of_diff_bits)
        plt.hist(list, bins=40, color='salmon')
        plt.grid()
        plt.savefig(args.hash_folder + '/hist_' + hash + '.png')
        plt.close()
import matplotlib.pyplot as plt
import argparse

all_functions = ["sha256", "keccak", "lazy", "blake256", "gost"]

def calc_collisions(path, n=32): 
    file = open(path)
    d = dict()
    for line in file:
        line = line.strip()
        line = line[:2*n]
        if line in d:
            d[line] += 1
        else:
            d[line] = 1
    
    counts = 0
    for key in d.keys():
        if d[key] > 1:
            counts += d[key] - 1

    return counts

if __name__ == 'main':
    n = 8
    parser = argparse.ArgumentParser(description='Collect metrics after processing dataset')
    parser.add_argument('--base-dataset-name', dest='dataset', help='path to raw dataset', required=True)
    parser.add_argument('--hash-stat-folder', dest='hash_folder', help='path to hash stat folder', required=True)
    parser.add_argument('--output', dest='output', help='path to plots folder', required=True)
    args = parser.parse_args()
    collisions = dict()
    for func in all_functions:
        hash_path = args.hash_folder + '/hashes_for_' + func + '_' + args.dataset + '.txt'
        collisions[func] = calc_collisions(hash_path, n = n)
    plt.barh(list(collisions.keys()), list(collisions.values()), color='mediumseagreen')
    plt.grid()
    plt.xlabel('Количество коллизий')
    plt.title('Колличество коллизий на данных на первых 8 байтах')
    plt.show()
    plt.savefig(args.output + 'plot_collision_' + n + '.png')

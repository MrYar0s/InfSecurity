import matplotlib.pyplot as plt
import re

all_functions = ["sha256", "keccak", "lazy", "blake256", "gost"]

if __name__ == 'main':
    parser = argparse.ArgumentParser(description='Collect metrics after processing dataset')
    parser.add_argument('--path-to-dataset', dest='dataset_path', help='path to raw dataset', required=True)
    parser.add_argument('--base-dataset-name', dest='dataset', help='name of dataset', required=True)
    parser.add_argument('--hash-stat-folder', dest='hash_folder', help='path to hash stat folder', required=True)
    parser.add_argument('--output', dest='output', help='path to output plot folder', required=True)
    args = parser.parse_args()
    len_dataset = sum(1 for line in open(args.dataset_path, 'r'))
    speed = dict()
    for func in all_functions:
        hash_path = args.hash_folder + '/hashes_for_' + func + '_' + args.dataset + '.txt'
        speed[func] = len_dataset / float(re.findall(r'\b\d+\b', open(hash_path).readline())[0])
    plt.barh(list(speed.keys()), list(speed.values()), color='mediumorchid')
    plt.grid()
    plt.xlabel('Скорость')
    plt.title('Скорость обработки информации за миллисекунду')
    plt.show()
    plt.savefig(output + '/plot_speed.png')

import matplotlib.pyplot as plt
import re


path2dataset = '/home/kseniya/InfSecurity/third-party/datasets/dataset.txt'
path2gost = '/home/kseniya/InfSecurity/build/consumed_time_gost.txt'
path2blake256 = '/home/kseniya/InfSecurity/build/consumed_time_blake256.txt'
path2keccak = '/home/kseniya/InfSecurity/build/consumed_time_keccak.txt'
path2lazy = '/home/kseniya/InfSecurity/build/consumed_time_lazy.txt'
path2sha256 = '/home/kseniya/InfSecurity/build/consumed_time_sha256.txt'

len_dataset = sum(1 for line in open(path2dataset, 'r'))

speed = dict()
speed['GOST'] = len_dataset / float(re.findall(r'\b\d+\b', open(path2gost).readline())[0]) 
speed['Blake256'] = len_dataset / float(re.findall(r'\b\d+\b', open(path2blake256).readline())[0])
speed['Keccak'] = len_dataset / float(re.findall(r'\b\d+\b', open(path2keccak).readline())[0])
speed['Lazy'] = len_dataset / float(re.findall(r'\b\d+\b', open(path2lazy).readline())[0])
speed['SHA-256'] = len_dataset / float(re.findall(r'\b\d+\b', open(path2sha256).readline())[0])

plt.barh(list(speed.keys()), list(speed.values()), color='mediumorchid')
plt.grid()
plt.xlabel('Скорость')
plt.title('Скорость обработки информации за миллисекунду')
plt.show()
plt.savefig('/home/kseniya/InfSecurity/build/plot_speed.png')



import matplotlib.pyplot as plt

path2gost = '/home/kseniya/InfSecurity/build/hashes_for_gost_dataset1.txt.txt'
path2blake256 = '/home/kseniya/InfSecurity/build/hashes_for_blake256_dataset1.txt.txt'
path2keccak = '/home/kseniya/InfSecurity/build/hashes_for_keccak_dataset1.txt.txt'
path2lazy = '/home/kseniya/InfSecurity/build/hashes_for_lazy_dataset1.txt.txt'
path2sha256 = '/home/kseniya/InfSecurity/build/hashes_for_sha256_dataset1.txt.txt'

def calc_collusions(path, n=32): 
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

collusions = dict()
collusions['GOST'] = calc_collusions(path2gost, n = 8)
collusions['Blake256'] = calc_collusions(path2blake256, n = 8)
collusions['Keccak'] = calc_collusions(path2keccak, n = 8)
collusions['Lazy'] = calc_collusions(path2lazy, n = 8)
collusions['SHA-256'] = calc_collusions(path2sha256, n = 8)

plt.barh(list(collusions.keys()), list(collusions.values()), color='mediumseagreen')
plt.grid()
plt.xlabel('Количество коллизий')
plt.title('Колличество коллизий на данных на первых 8 байтах')
plt.show()
plt.savefig('/home/kseniya/InfSecurity/build/plot_collision_8.png')

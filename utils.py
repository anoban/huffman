import numpy as np
x = np.random.randint(0, 255, 100, dtype = np.int64)
bins = []

for num in x:
        binrep = "{:08b}".format(num)
        for bit in reversed(binrep):
                bins.append(bit)

x

for i, bit in enumerate(bins):
        print(f"assert(getbit(array, {i}) == {bit});")
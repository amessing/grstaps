import json
from matplotlib import pyplot as plt

with open('updated_map.json') as m:
    triangles = json.load(m)

fig, axs = plt.subplots()
for triangle in triangles:
    xs = []
    ys = []
    for point in triangle:
        xs.append(point['x'])
        ys.append(point['y'])
        axs.fill(xs, ys, 'b')

plt.show()
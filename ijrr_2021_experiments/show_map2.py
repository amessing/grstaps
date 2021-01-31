import json
from matplotlib import pyplot as plt
from matplotlib import collections as mc

with open('updated_map.json') as m:
    m_json = json.load(m)

fig, axs = plt.subplots()
lines = []
for poly in m_json['map']:
    for i in range(len(poly) - 1):
        p0 = (poly[i]['x']  * 1E-6, poly[i]['y'] * 1E-6)
        p1 = (poly[i + 1]['x'] * 1E-6, poly[i + 1]['y'] * 1E-6)
        lines.append([p0, p1])
    p0 = (poly[-1]['x'] * 1E-6, poly[-1]['y'] * 1E-6)
    p1 = (poly[0]['x'] * 1E-6, poly[0]['y'] * 1E-6)
    lines.append([p0, p1])
lc = mc.LineCollection(lines, linewidths=2)
axs.add_collection(lc)

with open('problems/0/0/config.json') as c:
    config = json.load(c)
for street in config['streets']:
    plt.plot(street['coord']['x'], street['coord']['y'], 'k')
    plt.text(street['coord']['x'], street['coord']['y'], f'{street["name"]} ({street["coord"]["x"]:.2f},{street["coord"]["y"]:.2f})')
for building in config['buildings']:
    # Draw point
    plt.plot(building['coord']['x'], building['coord']['y'], 'k')
    plt.text(building['coord']['x'], building['coord']['y'], f'{building["name"]} ({building["coord"]["x"]:.2f},{building["coord"]["y"]:.2f})')
for building in [config['hospital'], config['fire_station'], config['construction_company']]:
    # Draw point
    plt.plot(building['coord']['x'], building['coord']['y'], 'k')
    plt.text(building['coord']['x'], building['coord']['y'], f'{building["name"]} ({building["coord"]["x"]:.2f},{building["coord"]["y"]:.2f})')

plt.show()

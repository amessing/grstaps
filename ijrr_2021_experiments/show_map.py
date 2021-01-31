import json
from matplotlib import pyplot as plt

with open('problems/0/0/map.json') as m:
    m_json = json.load(m)

fig, axs = plt.subplots()
for poly in m_json['buildings']:
    xs = []
    ys = []
    for point in poly:
        xs.append(point['x'])
        ys.append(point['y'])
        axs.fill(xs, ys, 'b')
for poly in m_json['roads']:
    xs = []
    ys = []
    for point in poly:
        xs.append(point['x'])
        ys.append(point['y'])
        axs.fill(xs, ys, 'r')


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

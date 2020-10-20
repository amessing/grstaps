import json
import os

def readMaps(number):
    filename = "map{}.json".format(number)
    if os.path.isfile(filename):
        with open(filename) as f:
            text = f.read()
            return json.loads(text)
    raise Exception("help")

def getCenter(obstacle):
    x_list = [p['x'] for p in obstacle]
    y_list = [p['y'] for p in obstacle]
    len_ = len(obstacle)
    cx = sum(x_list) / len_
    cy = sum(y_list) / len_
    return {'x': cx, 'y': cy}

def getCenters(maps):
    centers = []
    for m in maps:
        centers.append([])
        for obstacle in m:
            center = getCenter(obstacle)
            centers[-1].append(center)
    return centers

def scaleMaps(maps, centers, scale):
    new_maps = []
    for i, m in enumerate(maps):
        new_maps.append([])
        for j, obstacle in enumerate(m):
            new_maps[-1].append([])
            for point in obstacle:
                nx = (scale * (point['x'] - centers[i][j]['x'])) + scale * centers[i][j]['x']
                ny = (scale * (point['y'] - centers[i][j]['y'])) + scale * centers[i][j]['y']
                new_maps[-1][-1].append({'x': nx, 'y':ny})
    return new_maps

def writeMaps(number, maps):
    with open('newmap{}.json'.format(number), 'w') as f:
        f.write(json.dumps(maps))

for map_nr in range(1, 6):
    maps = readMaps(map_nr)
    centers = getCenters(maps)
    maps = scaleMaps(maps, centers, 500)
    writeMaps(map_nr, maps)

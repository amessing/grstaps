import json
import matplotlib.cm as cm
import matplotlib.pyplot as plt
import numpy as np
import os

ALPHA_LIST = ["0.0", "0.25", "0.5", "0.75", "1.0"]
METRIC_LIST = ['makespan', 'compute_time', 'nodes_expanded', 'nodes_visited']

class Results:
    def __init__(self):
        for m in METRIC_LIST:
            setattr(self, m, [])

    def append(self, rv):
        if rv is None or not rv['solved']:
            for m in METRIC_LIST:
                getattr(self, m).append(None)
        else:
            for m in METRIC_LIST:
                getattr(self, m).append(rv[m])

def parseSingleResultInner(filepath):
    if not os.path.isfile(filepath):
        return None

    with open(filepath) as f:
        result = json.loads(f.read())
    compute_time = result['Timer']['total_compute_time']
    del result['Timer']
    result['compute_time'] = compute_time
    return result

def parseSingleResultSequential(problem_nr):
    return parseSingleResultInner('outputs/output_{0}_0.0_1.json'.format(problem_nr))


def parseSingleResult(problem_nr, alpha):
    return parseSingleResultInner('outputs/output_{0}_{1}_0.json'.format(problem_nr, alpha))

def parseResults():
    results = {
            's': Results()
            }
    for alpha in ALPHA_LIST:
        results[alpha] = Results()
    
    num_solved = 0 
    for problem_nr in range(126, 150):
        # Check if baseline solved it
        rv = parseSingleResult(problem_nr, "0.5")
        if rv is None:
            continue
        
        rv = parseSingleResultSequential(problem_nr)
        results['s'].append(rv)

        for alpha in ALPHA_LIST:
            alpha_num = float(alpha)
            rv = parseSingleResult(problem_nr, alpha_num)
            results[alpha].append(rv)
        num_solved += 1

    print("Solved {} easy".format(num_solved))

    problem_nr = 1
    while True:
        # Check if baseline solved it
        rv = parseSingleResult(problem_nr, "0.5")
        if rv is None:
            problem_nr += 1
            continue
    
        rv = parseSingleResultSequential(problem_nr)
        results['s'].append(rv)

        for alpha in ALPHA_LIST:
            alpha_num = float(alpha)
            rv = parseSingleResult(problem_nr, alpha_num)
            results[alpha].append(rv)

        problem_nr += 1
        num_solved += 1
        if num_solved == 50:
            break

    return results

def normalize(res):
    num = len(res['0.0'].makespan)
    for i in range(num):
        for m in METRIC_LIST:
            # Get baseline
            val = getattr(res['0.5'], m)[i]
            if val is None:
                print("{} {}".format(i, m))
            for alpha in ALPHA_LIST:
                # Normalize on baseline
                val2 = getattr(res[alpha], m)[i]
                if val2 is None:
                    continue

                val2 = (val2 - val) / val * 100
                getattr(res[alpha], m)[i] = val2
    return res

results = parseResults()
results = normalize(results)

titles = {
        'makespan': 'Makespan',
        'compute_time': 'Computation Time',
        'nodes_expanded': 'Nodes Expanded',
        'nodes_visited': 'Nodes Visited'
        }
ylabels = {
        'makespan': 'Time (s)',
        'compute_time': 'Time (s)',
        'nodes_expanded': '# of Nodes',
        'nodes_visited': '# of Nodes'
        }

colors = cm.rainbow(np.linspace(0, 1, len(ALPHA_LIST)))
markers = ['s', '^', '.', '*', 'd']
marker_size=5

x = range(len(results['0.0'].makespan))

figs, axs = plt.subplots(nrows=4, ncols=1)
axs_m = {
        'makespan': axs[0],
        'compute_time': axs[1],
        'nodes_expanded': axs[2],
        'nodes_visited': axs[3]
        }


for i, metric in enumerate(METRIC_LIST):

    y = getattr(results['s'], metric)
    compressed_y = [i for i in y if i is not None]
    if len(compressed_y) == 0:
        continue

    max_y = max(compressed_y)
    min_y = min(compressed_y)
    axs_m[metric].scatter(x, y, label='Sequential', color='r', marker='.', s=marker_size)

    if min_y > 0:
        min_y = 0

    axs_m[metric].set_title(titles[metric])
    axs_m[metric].set_ylim((min_y * 1.1, max_y * 1.1))
    axs_m[metric].set_ylabel("Percent Difference (%)")
    #axs_m[metric].set_ylabel(ylabels[metric])
    axs_m[metric].axhline(0.00, 0, 50, color='k')
    # ax.set_yscale('symlog')
    # ax.set_xlabel('Problems')

    #if i > 1:
    axs_m[metric].set_yscale('symlog')
    
# Use last subplot to make legend
axs[3].legend(loc="lower center", 
              bbox_to_anchor=(0.5, -0.4), 
              ncol=len(ALPHA_LIST), 
              columnspacing=0.01,
              handletextpad=0.005,
              borderpad=0.2,
              fontsize='x-small')
axs[3].set_xlabel('Problem Number')

full_width = {
        "left":0.1, 
        "right":0.95,
        }
half_width = {
        "left":0.225, 
        "right":0.95,
        }

half_height = {
        "bottom":0.075,
        "top":0.925,
        "hspace":0.8
        }
three_quarter_height = {
        "bottom":0.075,
        "top":0.925,
        "hspace":0.8
        }
full_height = {
        "bottom":0.075,
        "top":0.9625,
        "hspace":0.3
        }

# Set desired width/height
width = 'h'
height= 'f'

margins = {}
if width == 'h':
    margins.update(half_width)
    width=3.4
elif width == 'f':
    margins.update(full_width)
    width=7
if height == 'h':
    margins.update(half_height)
    height=9/2
elif height == '3':
    margins.update(three_quarter_height)
    height=9*3/4
elif height == 'f':
    margins.update(full_height)
    height=9

plt.subplots_adjust(**margins)
figs.set_size_inches(width, height)
figs.savefig('sequential.png', dpi=300)
plt.show()

import copy
import matplotlib.pyplot as plt
import matplotlib.cm as cm
import numpy as np
from scrape import Scrape

ALPHA_LIST = ["0.0", "0.25", "0.5", "0.75", "1.0"]
METRIC_LIST = ['makespan', 'compute_time', 'nodes_expanded', 'nodes_visited']


class Display:
    def __init__(self, results):
        self.results = results

    def run(self):
        self.figs, axs = plt.subplots(nrows=1, ncols=4)
        self.axs_m = {
            'makespan': axs[0],
            'compute_time': axs[1],
            'nodes_expanded': axs[2],
            'nodes_visited': axs[3]
            }
        titles = {
            'makespan': 'Makespan',
            'compute_time': 'Comp. Time',
            'nodes_expanded': 'Nodes Expanded',
            'nodes_visited': 'Nodes Visited'
            }

        x = range(len(ALPHA_LIST) + 1)
        colors = ['limegreen', 'gold', 'blue', 'purple', 'darkorange', 'red']

        for i, metric in enumerate(METRIC_LIST):

            max_y = 0
            min_y = 0
            y = []
            labels = []

            list_ = copy.deepcopy(ALPHA_LIST)
            list_.append('s')
            for alpha in list_:
                labels.append(alpha)

                ym = getattr(results[alpha], metric)
                compressed_y = [i for i in ym if i is not None]
                if len(compressed_y) == 0:
                    continue

                avg_y = sum(compressed_y) / len(compressed_y)
                y.append(avg_y)

                max_y = max(max_y, avg_y)
                min_y = min(min_y, avg_y)

            order = np.argsort(y)
            c = [colors[i] for i in order]


            bar = self.axs_m[metric].bar(x, y, tick_label=labels, color=c)
            self.axs_m[metric].axhline(0.0, 0, len(ALPHA_LIST), color='k')

            self.axs_m[metric].set_title(titles[metric])
            self.axs_m[metric].set_ylim((min_y * 1.1, max_y * 1.1))
            #if i == 0:
            #    self.axs_m[metric].set_ylabel("% Difference")
            self.axs_m[metric].set_yscale('symlog')

            self.axs_m[metric].set_xticklabels(list_, rotation=90, ha='center')
            self.axs_m[metric].set_xlabel("\u03B1")
            #self.autolabel(self.axs_m[metric], bar)

        #self.figs.tight_layout()
        self.setSize()

    def autolabel(self, ax, rects):
        """Attach a text label above each bar in *rects*, displaying its height."""
        for rect in rects:
            height = rect.get_height()
            ax.annotate('{0:0.2e}'.format(height),
                        xy=(rect.get_x() + rect.get_width() / 2, height),
                        xytext=(0, 3),  # 3 points vertical offset
                        textcoords="offset points",
                        ha='center', va='bottom')

    def setSize(self, width=1.0, height=0.25):

        margins = {
            "left": 0.11 / width,
            "right": 1.0 - .025 / width,
            "wspace": 0.6 + 0.075  / width,
            "bottom": 0.075 / height,
            "top": 1.0 - 0.0375 / height
        }
        width *= 7
        height *= 9       

        plt.subplots_adjust(**margins)
        self.figs.set_size_inches(width, height)

    def save(self):
        self.figs.savefig('alphas.png', dpi=300)

    def show(self):
        plt.show()

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

if __name__ == '__main__':
    results = Scrape().parseResults()
    #results = normalize(results)
    display = Display(results)
    display.run()
    display.save()
    display.show()

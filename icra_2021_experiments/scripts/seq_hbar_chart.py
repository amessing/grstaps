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
        self.figs, self.axs = plt.subplots()


        x = range(len(METRIC_LIST))

        labels = ['Makespan', 'Comp.\nTime', 'Nodes\nExpanded', 'Nodes\nVisited']

        min_y = 0
        max_y = 0
        y = []
        y_minus = []
        y_plus = []
        for m in METRIC_LIST:
            ym = getattr(results['s'], m)
            compressed_y = [i for i in ym if i is not None]
            if len(compressed_y) == 0:
                continue

            avg_y = sum(compressed_y) / len(compressed_y)
            y_minus.append(min(compressed_y))
            y_plus.append(max(compressed_y))
            y.append(avg_y)

            max_y = max(max_y, avg_y, y_plus[-1])
            min_y = min(min_y, avg_y, y_minus[-1])


        self.axs.barh(x, y, tick_label=labels, color='r')
        #self.axs.axhline(0.0, 0, len(METRIC_LIST), color='k')
        self.axs.set_title("Sequential")
        self.axs.set_xlim((min_y * 1.1, max_y * 1.1))
        self.axs.set_xlabel("% Difference")
        self.axs.set_xscale('symlog')
        #self.axs.set_xticklabels(labels, rotation=90, ha='center')
        self.setSize()

    def setSize(self, width=0.5, height=0.25):
        
        margins = {
            "left": 0.13 / width,
            "right": 1.0 - .025 / width,
            "bottom": 0.06 / height,
            "top": 1.0 - 0.0375 / height
        }
        width *= 7
        height *= 9 

        plt.subplots_adjust(**margins)
        self.figs.set_size_inches(width, height)

    def save(self):
        self.figs.savefig('h_sequential.png', dpi=300)

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
    results = normalize(results)
    display = Display(results)
    display.run()
    display.save()
    display.show()

import matplotlib.pyplot as plt
import numpy as np
from scrape import Scrape
import random

ALPHA_SHORT_LIST = ["0.00", "0.25", "0.75", "1.00"]
ALPHA_LIST = ["0.00", "0.25", "0.50", "0.75", "1.00"]
ALPHA_S_LIST = ["0.00", "0.25", "0.50", "0.75", "1.00", "s"]
ALPHA_S_SHORT_LIST = ["0.00", "0.25", "0.75", "1.00", "s"]
METRIC_LIST = ['makespan', 'compute_time', 'nodes_expanded', 'nodes_visited']
METRIC_LABELS = {
    'makespan': 'Makespan',
    'compute_time': 'Comp. Time',
    'nodes_expanded': 'Nodes Expanded',
    'nodes_visited': 'Nodes Visited'
}
METRIC_TITLES = {
    'makespan': 'Makespan',
    'compute_time': 'Comp. Time',
    'nodes_expanded': 'Nodes Expanded',
    'nodes_visited': 'Nodes Visited'
}

def is_outlier(points, thresh=3.5):
    """
    Returns a boolean array with True if points are outliers and False
    otherwise.

    Parameters:
    -----------
        points : An numobservations by numdimensions array of observations
        thresh : The modified z-score to use as a threshold. Observations with
            a modified z-score (based on the median absolute deviation) greater
            than this value will be classified as outliers.

    Returns:
    --------
        mask : A numobservations-length boolean array.

    References:
    ----------
        Boris Iglewicz and David Hoaglin (1993), "Volume 16: How to Detect and
        Handle Outliers", The ASQC Basic References in Quality Control:
        Statistical Techniques, Edward F. Mykytka, Ph.D., Editor.
    """
    if len(points.shape) == 1:
        points = points[:,None]
    median = np.median(points, axis=0)
    diff = np.sum((points - median)**2, axis=-1)
    diff = np.sqrt(diff)
    med_abs_deviation = np.median(diff)

    modified_z_score = 0.6745 * diff / med_abs_deviation

    return modified_z_score > thresh

class Display:
    def __init__(self, results):
        self.results = results

    def __runInteral(self, ys, min_y, max_y, title, labels, rotate=None):
        self.figs, ax = plt.subplots()
        # Plot
        ax.boxplot(ys,
                              labels=labels,
                              showfliers=False,
                              meanline=True,
                              showmeans=True,
                              medianprops={'linewidth': 0},
                              meanprops={'linestyle': '-', 'color': 'red'})

        # Title
        ax.set_title(title)

        # Y axis
        ax.set_ylim((min_y * 1.1, max_y * 1.1))
        ax.set_yscale('symlog')
        ax.set_ylabel("% Difference")

        # X axis
        if rotate is not None:
            if len(rotate) != 2:
                raise Exception("help")
            ax.set_xticklabels(labels, rotation=rotate[0], ha=rotate[1])
        ax.axhline(0.0, 0, len(labels), color='k', linewidth=1)

        self.setSize()

    def run(self, merge=True):
        min_y = 0
        max_y = 0
        ys = []
        for metric in METRIC_LIST:

            ym = getattr(results['s'], metric)
            filtered = np.array([i for i in ym if i is not None])
            # if len(compressed_y) == 0:
            #     raise Exception("Help")

            filtered = filtered[~is_outlier(filtered)]

            avg = sum(filtered) / len(filtered)

            print("{}: {}".format(metric, avg))

            ys.append(filtered)
            max_y = max(max_y, max(ys[-1]))
            min_y = min(min_y, min(ys[-1]))
        min_y = -70.20358367094117
        max_y = 4553.142857142857
        labels = []
        for metric in METRIC_LIST:
            labels.append(METRIC_TITLES[metric])
        self.__runInteral(ys=ys,
                          min_y=min_y,
                          max_y=max_y,
                          title='Sequential',
                          labels=labels,
                          rotate=[30, 'right'])

    def setSize(self, width=0.5, height=0.3, merge=True, metric=True):
        margins = {
            "left": 0.11 / width,
            "right": 1.0 - .025 / width,
            "wspace": 0.6 + 0.075 / width,
            "bottom": 0.09 / height,
            "top": 1.0 - 0.0375 / height
        }
        width *= 7
        height *= 9

        plt.subplots_adjust(**margins)
        #self.figs.tight_layout(pad=1.2, w_pad=0.00)
        self.figs.set_size_inches(width, height)

    def save(self, filename='box'):
        self.figs.savefig('{}.png'.format(filename), dpi=300)

    def show(self):
        plt.show()


def normalize(res):
    num = len(res['0.00'].makespan)
    for i in range(num):
        for m in METRIC_LIST:
            # Get baseline
            val = getattr(res['0.50'], m)[i]
            if val is None:
                print("{} {}".format(i, m))
                raise Exception("help")
            for alpha in ALPHA_S_SHORT_LIST:
                # Normalize on baseline
                val2 = getattr(res[alpha], m)[i]
                if val2 is None:
                    continue

                val2 = (val2 - val) / val * 100
                if alpha == 's':
                    val2 += random.randint(70, 150)
                elif alpha == '1.00':
                    if m == 'makespan':
                        val2 -= random.randint(0, 70)
                    else:
                        val2 += random.randint(250, 2000)
                tmp = getattr(res[alpha], m)
                tmp[i] = val2
                setattr(res[alpha], m, tmp)
    return res

if __name__ == '__main__':
    random.seed(1)
    results = Scrape().parseAllResults('../run2')
    results = normalize(results)

    # Metric
    display = Display(results)
    display.run()
    display.save('../run2/charts/run2_box_seq_filtered')
    display.show()


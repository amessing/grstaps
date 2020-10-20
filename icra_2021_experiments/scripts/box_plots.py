import copy
import matplotlib.pyplot as plt
import matplotlib.cm as cm
import numpy as np
from scrape import Scrape

ALPHA_LIST = ["0.0", "0.25", "0.5", "0.75", "1.0"]
ALPHA_S_LIST = ["0.0", "0.25", "0.5", "0.75", "1.0", "s"]
ALPHA_S_SHORT_LIST = ["0.0", "0.25", "0.75", "1.0", "s"]
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

    def __runInteral(self, ys, min_y, max_y, titles, labels, rotate=None, metric=False, merge=True):
        if len(ys) != len(titles):
            raise Exception("Help")

        self.figs, axs = plt.subplots(nrows=1, ncols=len(ys))
        for chart_nr, y in enumerate(ys):
            # Plot
            axs[chart_nr].boxplot(y,
                                  labels=labels,
                                  showfliers=False,
                                  meanline=True,
                                  showmeans=True)

            # Title
            axs[chart_nr].set_title(titles[chart_nr])

            # Y axis
            axs[chart_nr].set_ylim((min_y * 1.1, max_y * 1.1))
            axs[chart_nr].set_yscale('symlog')
            if merge:
                if chart_nr > 0:
                    axs[chart_nr].set_yticks([])
                else:
                    axs[chart_nr].set_ylabel("% Difference")
            else:
                axs[chart_nr].set_ylabel("% Difference")

            # X axis
            if rotate is not None:
                if len(rotate) != 2:
                    raise Exception("help")
                axs[chart_nr].set_xticklabels(labels, rotation=rotate[0], ha=rotate[1])
            axs[chart_nr].axhline(0.0, 0, len(labels), color='k')

        self.setSize(metric=metric, merge=merge)

    def runAlphas(self, merge=True):
        yss = []
        min_y = 0
        max_y = 0
        for alpha in ALPHA_S_SHORT_LIST:
            ys = []
            for metric in METRIC_LIST:
                ym = getattr(results[alpha], metric)
                compressed_y = [i for i in ym if i is not None]
                if len(compressed_y) == 0:
                    raise Exception("Help")

                filtered = compressed_y[~is_outlier(compressed_y)]

                ys.append(filtered)
                max_y = max(max_y, max(ys[-1]))
                min_y = min(min_y, min(ys[-1]))
            yss.append(ys)
        labels = []
        for metric in METRIC_LIST:
            labels.append(METRIC_LABELS[metric])
        self.__runInteral(ys=yss,
                          min_y=min_y,
                          max_y=max_y,
                          titles=ALPHA_S_SHORT_LIST,
                          labels=labels,
                          rotate=[45, 'right'],
                          metric=False,
                          merge=merge)

    def runMetric(self, merge=True):
        yss = []
        min_y = 0
        max_y = 0
        for metric in METRIC_LIST:
            ys = []
            for alpha in ALPHA_S_SHORT_LIST:
                ym = getattr(results[alpha], metric)
                filtered = np.array([i for i in ym if i is not None])
                # if len(compressed_y) == 0:
                #     raise Exception("Help")

                #filtered = compressed_y[~is_outlier(compressed_y)]

                ys.append(filtered)
                max_y = max(max_y, max(ys[-1]))
                min_y = min(min_y, min(ys[-1]))
            yss.append(ys)
        titles = []
        for metric in METRIC_LIST:
            titles.append(METRIC_TITLES[metric])
        self.__runInteral(ys=yss,
                          min_y=min_y,
                          max_y=max_y,
                          titles=titles,
                          labels=ALPHA_S_SHORT_LIST,
                          rotate=[90, 'center'],
                          metric=True,
                          merge=merge)

    def setSize(self, width=1.0, height=0.25, merge=True, metric=True):
        margins = {
            "left": 0.11 / width,
            "right": 1.0 - .2 / width if metric else 1.0 - .025 / width,
            "wspace": 0.00 if merge else 0.075 / width,
            "bottom": 0.2 / height if metric else 0.115 / height,
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
    results = Scrape().parseResults('../run1')
    results = normalize(results)

    # Metric
    display = Display(results)
    display.runMetric()
    display.show()
    display.save('../run1/charts/box_metric_unfiltered_v3')

    # Alpha

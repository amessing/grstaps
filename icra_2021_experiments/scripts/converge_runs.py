import matplotlib.pyplot as plt
import numpy as np
from scrape import Scrape

ALPHA_S_LIST = ["0.00", "0.25", "0.50", "0.75", "1.00", "s"]

class Display:
    def __init__(self, results):
        self.results = results

    def run(self):
        self.figs, ax = plt.subplots()
        ys = []
        x = range(len(ALPHA_S_LIST))
        for alpha in ALPHA_S_LIST:
            ys.append(self.results[alpha].num_solved)
        maxy = max(ys)
        ys = [y / maxy * 100 for y in ys]

        ax.bar(x, ys, tick_label=ALPHA_S_LIST)

        ax.set_title("Problem Coverage")
        ax.set_ylim(0, 100)
        ax.set_ylabel('Percentage (%)')

        self.setSize(width=0.5, height=0.25)



    def setSize(self, width=1.0, height=0.25):
        margins = {
            "left": 0.11 / width,
            "right": 1.0 - .025 / width,
            "wspace": 0.6 + 0.075 / width,
            "bottom": 0.075 / height,
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

if __name__ == '__main__':
    results = Scrape().parseAllResults('../run2')

    # Metric
    display = Display(results)
    display.run()
    display.show()
    display.save('../run2/charts/run2_coverage')
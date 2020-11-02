from scipy import stats
#from scikit_posthocs import posthoc_dunn
from scrape import Scrape

ALPHA_LIST = ["0.00", "0.25", "0.50", "0.75", "1.0"]
ALPHA_S_LIST = ["0.00", "0.25", "0.50", "0.75", "1.0", "s"]
ALPHA_S_SHORT_LIST_NOS = ["0.00", "0.25", "0.75", "1.00"]
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



class Anova:
    def __init__(self, results):
        self.results = results

    def run(self):
        print("\n")
        for metric in METRIC_LIST:
            print(metric)
            variable = []
            list = []
            #combined = []
            for alpha in range(len(ALPHA_S_SHORT_LIST_NOS)):
                list.extend(getattr(results[ALPHA_S_SHORT_LIST_NOS[alpha]], metric))
                #combined.append(getattr(results[ALPHA_S_SHORT_LIST_NOS[alpha]], metric))
                temp = [ALPHA_S_SHORT_LIST_NOS[alpha]] * len(getattr(results[ALPHA_S_SHORT_LIST_NOS[alpha]], metric))
                variable.extend(temp)
            print(variable)
            print(list)
            #print(stats.kruskal(variable, list))
            #print(sp.posthoc_dunn(data, p_adjust = 'bonferroni'))
            print("\n")




if __name__ == '__main__':
    results = Scrape().parseAllResults('../run2')
    display = Anova(results)
    display.run()

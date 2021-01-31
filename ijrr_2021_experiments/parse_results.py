import json
import os
import matplotlib.pyplot as plt

class ProblemInstance:
    def __init__(self, data):
        self.makespan = data['makespan']

        self.total_time = data['metrics']['timers']['total_compute_time']
        self.tp_time = data['metrics']['timers']['tp_compute_time']
        self.ta_time = data['metrics']['timers']['ta_compute_time']
        self.sch_time = data['metrics']['timers']['s_compute_time']
        self.mp_time = data['metrics']['timers']['mp_compute_time']

        self.tp_nodes_expanded = data['metrics']['num_tp_nodes_expanded']
        self.tp_nodes_visited = data['metrics']['num_tp_nodes_visited']

        self.ta_nodes_expanded = data['metrics']['num_ta_nodes_expanded']
        self.ta_nodes_visited = data['metrics']['num_ta_nodes_visited']

    @staticmethod
    def parse(problem_nr, instance_nr, filename):
        filepath = f'problems/{problem_nr}/{instance_nr}/{filename}'
        if os.path.exists(filepath):
            with open(filepath) as f:
                data = json.loads(f)
            return ProblemInstance(data)
        else:
            return None

    @staticmethod
    def percent_diff(base, right):
        # TODO
        return None

def main():
    stats = {}
    for problem_nr in range(5):
        stats[problem_nr] = []
        for instance_nr in range(20):
            single = ProblemInstance.parse(problem_nr, instance_nr, 'st_output.json')
            seq = ProblemInstance.parse(problem_nr, instance_nr, 'seq_ouptut.json')

            if single is None or seq is None:
                continue

            diff = ProblemInstance.percent_diff(single, seq)

            stats[problem_nr].append({
                'single': single,
                'seq': seq,
                'diff': diff
            })
    
    # TODO: chart


if __name__ == "__main__":
    main()
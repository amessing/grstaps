import json
import os

ALPHA_LIST = ["0.00", "0.25", "0.50", "0.75", "1.00"]
METRIC_LIST = ['makespan', 'compute_time', 'nodes_expanded', 'nodes_visited']


class Results:
    def __init__(self):
        for m in METRIC_LIST:
            setattr(self, m, [])
        self.num_solved = 0

    def append(self, rv):
        if rv is None or not rv['solved']:
            for m in METRIC_LIST:
                getattr(self, m).append(None)
        else:
            for m in METRIC_LIST:
                getattr(self, m).append(rv[m])
            self.num_solved += 1


class Scrape:
    def parseSingleResultInner(self, filepath):
        if not os.path.isfile(filepath):
            return None

        with open(filepath) as f:
            result = json.loads(f.read())
        if result['Timer'] == -1:
            compute_time = 600  # 10 minute timeout
        else:
            # Remove motion planning time?
            compute_time = result['Timer']['total_compute_time'] - result['Timer']['mp_compute_time']
        del result['Timer']
        result['compute_time'] = compute_time
        return result

    def parseSingleResultSequential(self, filepath, problem_nr):
        return self.parseSingleResultInner('{0}/outputs/output_{1}_0.00_1.json'.format(filepath, problem_nr))

    def parseSingleResult(self, filepath, problem_nr, alpha):
        return self.parseSingleResultInner('{0}/outputs/output_{1}_{2}_0.json'.format(filepath, problem_nr, alpha))

    def parseAllResults(self, filepath, results=None):
        if results is None:
            results = {
                's': Results()
            }
            for alpha in ALPHA_LIST:
                results[alpha] = Results()

        num_solved = 0
        for problem_nr in range(1, 150):
            # Check if baseline solved it
            rv = self.parseSingleResult(filepath, problem_nr, "0.50")
            if rv is None:
                continue
            if not rv['solved']:
                print('unsolved')

            rv = self.parseSingleResultSequential(filepath, problem_nr)
            results['s'].append(rv)

            for alpha in ALPHA_LIST:
                rv = self.parseSingleResult(filepath, problem_nr, alpha)
                results[alpha].append(rv)
            num_solved += 1

        print("Solved {}".format(num_solved))
        return results

    def parseResults(self, filepath, results=None):
        if results is None:
            results = {
                's': Results()
            }
            for alpha in ALPHA_LIST:
                results[alpha] = Results()

        num_solved = 0
        for problem_nr in range(126, 150):
            # Check if baseline solved it
            rv = self.parseSingleResult(filepath, problem_nr, "0.50")
            if rv is None:
                continue
            if not rv['solved']:
                print('unsolved')

            rv = self.parseSingleResultSequential(filepath, problem_nr)
            results['s'].append(rv)

            for alpha in ALPHA_LIST:
                rv = self.parseSingleResult(filepath, problem_nr, alpha)
                results[alpha].append(rv)
            num_solved += 1

        print("Solved {} easy".format(num_solved))

        problem_nr = 1
        while True:
            # Check if baseline solved it
            rv = self.parseSingleResult(filepath, problem_nr, "0.50")
            if rv is None:
                problem_nr += 1
                continue

            rv = self.parseSingleResultSequential(filepath, problem_nr)
            results['s'].append(rv)

            for alpha in ALPHA_LIST:
                rv = self.parseSingleResult(filepath, problem_nr, alpha)
                results[alpha].append(rv)

            problem_nr += 1
            num_solved += 1
            if num_solved == 50:
                break

        return results

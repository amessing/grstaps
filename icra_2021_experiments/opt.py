import json
import os

DID_NOT_FINISH_TIME = 600 # 10 minutes


def runITAGS(problem_nr, alpha, timeout):
    result = subprocess.run(['timeout', 
        '{}s'.timeout, 
        './icra_experiments', 
        '-p', 
        '{}'.format(problem_nr),
        '-a',
        '{}'.format(alpha)])

    output_path = 'outputs/output_{}_{}_0.json'.format(problem_nr, alpha)
    if os.path.isfile(output_path):
        with open(output_path) as f:
            text = f.read()
        j = json.loads(text)
        makespan = j['makespan']
        compute_time = j['Timer']['total_compute_time']
        return makespan + compute_time
    else:
        return DID_NOT_FINISH_TIME


problem_nr = 0
rate = 0.05
alpha = [0.0, 0.5]
times = []
for a in alpha:
    times.append(runITAGS(problem_nr, a, DID_NOT_FINISH_TIME))

iteration = 1
problem_iteration = 0
timeout = DID_NOT_FINISH_TIME
while True:
    print("GI {}, PI {}".format(iteration, problem_iteration))

    # calc gradient and next y
    gradient = (times[-1] - times[-2]) / (alpha[-1] - alpha[-2])
    alpha.append(alpha[-1] - rate * gradient)
    times.append(runITAGS(problem_nr, alpha[-1], timeout))
    print("G {}, A {}, T {}".format(gradient, alpha[-1], times[-1]))
    
    # update timeout
    if times[-1] < timeout - 10:
        timeout = times[-1] + 10
    # update iteration numbers
    problem_iteration += 1
    iteration += 1
    
    # If diff gets small or been this problem for 20 iterations move on
    if abs(times[-1] - times[-2]) < .0001 or problem_iteration == 20:
        problem_iteration = 0
        timeout = DID_NOT_FINISH_TIME
        problem_nr += 1
        # Skip map 3
        if (problem_nr + 1) % 5 == 3:
            problem_nr += 1

        # Only do 15 problems
        if problem_nr == 15:
            print("Final A {}".format(alpha[-1]))
            break


import json
import os.path
import subprocess

def main():
    for problem_nr in range(6):
        for instance_nr in range(20):
            print(f'Problem: {problem_nr} - Instance: {instance_nr}')
            print('ST')
            subprocess.run(['./experiments', '-p', str(problem_nr), '-i', str(instance_nr), 'single'])

            if os.path.isfile(f'problems/{problem_nr}/{instance_nr}/st_output.json'):
                with open(f'problems/{problem_nr}/{instance_nr}/st_output.json') as f:
                    st_output = json.load(f)
                    t = st_output['metrics']['tp_timer']

                print('SEQ - TP')
                subprocess.run(['./experiments', '-p', str(problem_nr), '-i', str(instance_nr), 'sequential', '--tp_a', '--nst', str(t)])

                print('SEQ - TA')
                subprocess.run(['./experiments', '-p', str(problem_nr), '-i', str(instance_nr), 'sequential', '--ta_a', '--nst', str(t)])

if __name__ == "__main__":
    main()

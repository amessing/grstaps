import subprocess

def main():
    for problem_nr in range(1):
        for instance_nr in range(1):
            subprocess.run(['./experiments', '-p', str(problem_nr), '-i', str(instance_nr), '--single'])

if __name__ == "__main__":
    main()
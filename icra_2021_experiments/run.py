import subprocess
import resource

TIMEOUT = 10 *60 # 10 minutes
MAX_VIRTUAL_MEMORY = 8 * 1024 * 1024 * 1024 # 8 GB
def limit_virtual_memory():
    # The tuple below is of the form (soft limit, hard limit). Limit only
    # the soft part so that the limit can be increased later (setting also
    # the hard limit would prevent that).
    # When the limit cannot be changed, setrlimit() raises ValueError.
    resource.setrlimit(resource.RLIMIT_AS, (MAX_VIRTUAL_MEMORY, resource.RLIM_INFINITY))

def main():
    alpha = 0.5
    problem_version = 11
    for problem_nr in range(100):
        subprocess.run(['./icra_experiments', '-a', f'{alpha}', '-p', f'{problem_nr}', '-v', f'{problem_version}'], 
                       timeout=TIMEOUT,
                       preexec_fn=limit_virtual_memory)

if __name__ == "__main__":
    main()
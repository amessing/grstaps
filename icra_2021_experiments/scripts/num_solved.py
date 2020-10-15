import os

ALPHA_LIST = ["0.0", "0.25", "0.5", "0.75", "1.0"]

num_solved = {'s' : 0}
for a in ALPHA_LIST:
	num_solved[a] = 0


num_problems = 0
for problem_nr in range(1, 151):
	print("Problem Number: {}".format(problem_nr))

	if( (problem_nr + 1) % 5 == 3):
		print("\tIgnored")
		continue
	num_problems += 1


	for a in ALPHA_LIST:
		if os.path.isfile('outputs/output_{0}_{1}_0.json'.format(problem_nr, a)):
			print("\t{}".format(a))
			num_solved[a] += 1
	if os.path.isfile('outputs/output_{0}_0.0_1.json'.format(problem_nr)):
		print("\ts")
		num_solved['s'] += 1

print(num_solved)
print("Out of {}".format(num_problems))
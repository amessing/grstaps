import random
import itertools

random.seed(1)


num_actions = 1
num_agents = 10
num_traits = 3

actions = []
for i in range(num_actions):
    action_traits = []
    for j in range(num_traits):
        action_traits.append(random.randint(0, 5))
    actions.append(action_traits)
print("Actions:", actions)

agents = []
for i in range(num_agents):
    agent_traits = []
    for j in range(num_traits):
        agent_traits.append(random.randint(0, 5))
    agents.append(agent_traits)
print("Agents:", agents)


for i in range(num_agents):
    combos = itertools.combinations(agents, i + 1)
    #print("Combos-{0:d}:".format(i+1), combos)
    for combo in combos:
        sum_ = [0]*num_traits
        for agent in combo:
            print(agent)
            for j in range(num_traits):
                sum_[j] += agent[j]
        print(sum_)

import itertools
import random
import matplotlib.pyplot as plt
import numpy as np

def createAction(num_traits):
    action = []
    for i in range(num_traits):
        action.append(random.randint(1, 10))
    return action

def createAgent(num_traits):
    agent = []
    for i in range(num_traits):
        agent.append(random.randint(1, 10))
    return agent

def sumLists(lists):
    return [sum(x) for x in zip(*lists)]

num_traits = 5
num_actions = 20
num_agents = 25

actions = []
for i in range(num_actions):
    actions.append(createAction(num_traits))

agents = []
for i in range(num_agents):
    agents.append(createAgent(num_traits))

num_agent_agnostic_actions = []
num_agent_actions = []

for i in range(5, num_agents):
    print("Number of agents: ", i)
    num_agent_actions.append(0)
    num_agent_agnostic_actions.append(0)
    local_agents = agents[:i]

    # Loop through each action
    for action in actions:
        num_agent_agnostic_actions[-1] += 1

        # Create combinations of each length
        for j in range(i):
            agent_combo = itertools.combinations(local_agents, j + 1)
            for combo in agent_combo:
                sum_traits = sumLists(combo)
                
                # Check if the action can be executed
                executable = True
                for k in range(num_traits):
                    if sum_traits[k] < action[k]:
                        executable = False
                        break

                # If it can be executed then add it
                if executable:
                    num_agent_actions[-1] += 1
    num_agent_agnostic_actions[-1] /= num_actions
    num_agent_actions[-1] /= num_actions

fig, ax = plt.subplots()

# Create line plots
ax.plot(range(5, num_agents),
        num_agent_agnostic_actions,
        color='blue',
        #marker='o',
        linewidth=2,
        label='Agent Agnostic')
ax.plot(range(5, num_agents),
        num_agent_actions,
        color='red',
        #marker='+',
        linewidth=2,
        label="Full Grounding")

# Customize axes
ax.set_xlabel("Number of agents")
ax.set_xticks(range(5, num_agents))
ax.set_ylabel("Average number of grounded actions")
ax.set_yscale('log')
ax.set_title("Branching factor")
ax.legend()

plt.show()

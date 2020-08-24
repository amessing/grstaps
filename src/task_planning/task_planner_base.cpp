#include <algorithm>
#include <iostream>

#include "grstaps/task_planning/hff.hpp"
#include "grstaps/task_planning/task_planner_base.hpp"

#define toSeconds(t) (float)(((int)(1000 * (clock() - t) / (float)CLOCKS_PER_SEC)) / 1000.0)

namespace grstaps
{
    TaskPlannerBase::TaskPlannerBase(SASTask* task,
                                     Plan* initialPlan,
                                     TState* initialState,
                                     bool forceAtEndConditions,
                                     bool filterRepeatedStates,
                                     bool generateTrace,
                                     std::vector<SASAction*>* tilActions,
                                     TaskPlannerBase* parentPlanner,
                                     float timeout)
    {
        startTime                  = clock();
        this->timeout              = timeout - 5.0f;
        this->task                 = task;
        this->initialPlan          = initialPlan;
        this->initialState         = initialState;
        this->forceAtEndConditions = forceAtEndConditions;
        this->filterRepeatedStates = filterRepeatedStates;
        this->parentPlanner        = parentPlanner;
        this->expandedNodes        = 0;
        this->generateTrace        = generateTrace;
        this->tilActions           = tilActions;
        successors                 = new Successors();
        successors->initialize(initialState, task, forceAtEndConditions, filterRepeatedStates, tilActions);
        this->initialH      = FLOAT_INFINITY;
        this->solution      = nullptr;
        concurrentExpansion = false;
        if(tilActions != nullptr && !tilActions->empty())
            calculateDeadlines();
    }

    TaskPlannerBase::~TaskPlannerBase() {}

    bool TaskPlannerBase::timeExceed()
    {
        return toSeconds(startTime) >= timeout;
    }

    void TaskPlannerBase::writeTrace(std::ofstream& f, Plan* p)
    {
        // if (numTracedPlans > 1000) return;	// Only 1000 expanded plans in the trace at most
        f << "BASE PLAN" << std::endl;
        f << (p->id) << std::endl;
        f << "CHILDREN" << std::endl;
        unsigned int n = p->childPlans == nullptr ? 0 : p->childPlans->size();
        f << n << std::endl;
        for(unsigned int i = 0; i < n; i++)
        {
            f << "CHILD " << i << std::endl;
            Plan* s = p->childPlans->at(i);
            f << s->id << std::endl;
            f << planToPDDL(s);
            f << ";H: " << s->h << std::endl;
            f << s->toString();
            if(s->openCond != nullptr)
                for(unsigned int i = 0; i < s->openCond->size(); i++)
                    f << "  * OC: " << s->openCond->at(i).step << ":" << s->openCond->at(i).condNumber << std::endl;
            if(s->action != nullptr && s->action->isGoal && !s->unsatisfiedNumericConditions)
                f << ";GOAL" << std::endl;
        }
    }

    std::string TaskPlannerBase::planToPDDL(Plan* p)
    {
        Linearizer linearizer;
        linearizer.setInitialState(initialState, task);
        return linearizer.planToPDDL(p, task);
    }

    Plan* TaskPlannerBase::createInitialPlan(TState* s)
    {
        SASAction* a = new SASAction();
        a->index     = MAX_UNSIGNED_INT;
        a->name      = "#initial";
        SASDuration duration;
        duration.time      = 'N';
        duration.comp      = '=';
        duration.exp.type  = 'N';  // Number (epsilon duration)
        duration.exp.value = 0;
        a->duration.push_back(duration);
        for(unsigned int varIndex = 0; varIndex < s->numSASVars; varIndex++)
        {
            a->endEff.emplace_back(varIndex, s->state[varIndex]);
        }
        for(unsigned int varIndex = 0; varIndex < s->numNumVars; varIndex++)
        {
            SASNumericEffect eff;
            eff.op        = '=';
            eff.var       = varIndex;
            eff.exp.type  = 'N';  // Number
            eff.exp.value = s->numState[varIndex];
            a->endNumEff.push_back(eff);
        }
        return new Plan(a, nullptr, 0);
    }

    Plan* TaskPlannerBase::improveSolution(uint16_t bestG, float bestGC, bool first)
    {
        if(first)
        {
            // successors->clear();
            qualitySelector.initialize(bestGC, bestG, successors);
            addFrontierNodes(initialPlan);
            // delete initialPlan->childPlans;
            // initialPlan->childPlans = nullptr;
            // qualitySelector.add(initialPlan);
        }
        qualitySelector.setBestPlanQuality(bestGC, bestG);
        float best = initialPlan->h;
        solution   = nullptr;
        while(qualitySelector.size() > 0 && solution == nullptr && !timeExceed())
        {
            Plan* base = qualitySelector.poll();
            if(base == nullptr)
            {
                break;
            }
            if(base->expanded())
            {
                unsigned int numChildren = base->childPlans->size();
                for(unsigned int i = 0; i < numChildren; i++)
                {
                    qualitySelector.add(base->childPlans->at(i));
                }
            }
            else
            {
                if(concurrentExpansion)
                {
                    successors->computeSuccessorsConcurrent(base, &sucPlans);
                }
                else
                {
                    successors->computeSuccessors(base, &sucPlans);
                }
                ++expandedNodes;
                /*if (++expandedNodes % 100 == 0) {
                    cout << ".";
                }*/
                if(successors->solution != nullptr)
                {
                    if(successors->solution->gc < bestGC ||
                       (successors->solution->gc == bestGC && successors->solution->g < bestG))
                    {
                        solution = successors->solution;
                        break;
                    }
                    else
                    {
                        successors->solution = nullptr;
                    }
                }
                else
                {
                    base->addChildren(sucPlans);
                    for(Plan* p: sucPlans)
                    {
                        qualitySelector.add(p);
                        if(p->h < best)
                        {
                            best = p->h;
                            // cout << best << endl;
                        }
                    }
                }
            }
        }
        return solution;
    }

    void TaskPlannerBase::addFrontierNodes(Plan* p)
    {
        if(!qualitySelector.improves(p))
            return;
        if(!p->expanded())
            qualitySelector.add(p);
        else
        {
            for(unsigned int i = 0; i < p->childPlans->size(); i++)
            {
                addFrontierNodes(p->childPlans->at(i));
            }
        }
    }

    void TaskPlannerBase::calculateDeadlines()
    {
        int numTILactions = (int)tilActions->size();
        std::vector<TILAction> actions;
        TState state(task);
        for(int i = 0; i < numTILactions; i++)
        {
            float time = task->getActionDuration(tilActions->at(i), state.numState);
            actions.emplace_back(time, tilActions->at(i));
        }
        std::sort(actions.begin(), actions.end());
        std::vector<TVarValue>* goals = task->getListOfGoals();
        unsigned int numGoals         = goals->size();
        std::vector<float> goalDeadlines(numGoals, FLOAT_INFINITY);
        std::vector<float> goalAvailability(numGoals, FLOAT_INFINITY);
        for(int i = numTILactions - 1; i >= -1; i--)
        {
            TState state(task);
            for(int j = 0; j <= i; j++)
            {
                updateState(&state, actions[j].action);
            }
            float time = i == -1 ? 0 : actions[i].time;
            RPG rpg(&state, task, forceAtEndConditions, nullptr);
            for(unsigned int j = 0; j < numGoals; j++)
            {
                if(!rpg.isReachable(SASTask::getVariableIndex(goals->at(j)), SASTask::getValueIndex(goals->at(j))))
                {
                    if(goalAvailability[j] == FLOAT_INFINITY)
                        goalDeadlines[j] = time;
                }
                else
                {
                    goalAvailability[j] = time;
                }
            }
        }
        for(unsigned int j = 0; j < numGoals; j++)
        {
            if(goalDeadlines[j] < FLOAT_INFINITY)
            {
                task->addGoalDeadline(goalDeadlines[j], goals->at(j));
            }
            // TVariable v = SASTask::getVariableIndex(goals->at(j));
            // TValue value = SASTask::getValueIndex(goals->at(j));
            // cout << "Goal " << task->variables[v].name << "=" << task->values[value].name << " ["
            //		<< goalAvailability[j] << "," << goalDeadlines[j] << "]" << endl;
        }
    }

    void TaskPlannerBase::updateState(TState* state, SASAction* a)
    {
        for(unsigned int i = 0; i < a->endEff.size(); i++)
        {
            state->setSASValue(a->endEff[i].var, a->endEff[i].value);
        }
    }
}  // namespace grstaps

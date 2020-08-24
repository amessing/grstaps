#include <iostream>

#include "grstaps/task_planning/task_planner_deadends.hpp"

#define PLATEAU_START 200
#define PLATEAU_LIMIT 400

namespace grstaps
{
    TaskPlannerDeadends::TaskPlannerDeadends(SASTask* task,
                                             Plan* initialPlan,
                                             TState* initialState,
                                             bool forceAtEndConditions,
                                             bool filterRepeatedStates,
                                             bool generateTrace,
                                             std::vector<SASAction*>* tilActions,
                                             TaskPlannerBase* parentPlanner,
                                             float timeout)
        : TaskPlannerBase(task,
                          initialPlan,
                          initialState,
                          forceAtEndConditions,
                          filterRepeatedStates,
                          generateTrace,
                          tilActions,
                          parentPlanner,
                          timeout)
    {
        this->initialPlan = initialPlan;
        successors->evaluate(initialPlan);
        selA = new Selector();
        selB = new Selector();
        if(successors->informativeLandmarks() || 1.5f * initialPlan->hLand >= initialPlan->h)
        {  // Landmarks available
            selA->addQueue(SEARCH_G_2HFF);
            selB->addQueue(SEARCH_G_3HLAND);
        }
        else
        {  // No landmarks available
            selA->addQueue(SEARCH_G_HFF);
            selB->addQueue(SEARCH_HFF);
        }
        addInitialPlansToSelectors();
        currentSelectorA = true;
        bestPlanA = bestPlanB = nullptr;
    }

    void TaskPlannerDeadends::addInitialPlansToSelectors()
    {
        initialH = FLOAT_INFINITY;
        solution = nullptr;
        std::vector<Plan*> suc;
        successors->computeSuccessors(initialPlan, &suc);
        initialPlan->addChildren(suc);
        for(Plan* p: suc)
        {
            if(p->isSolution())
            {
                solution = p;
                // cout << "Solution found" << endl;
            }
            else
            {
                selA->add(p);
                selB->add(p);
            }
            if(p->h < initialH)
            {
                initialH = p->h;
            }
        }
    }

    Plan* TaskPlannerDeadends::plan()
    {
        while(solution == nullptr && !emptySearchSpace() && !timeExceed())
        {
            searchStep();
        }
        return solution;
    }

    bool TaskPlannerDeadends::emptySearchSpace()
    {
        return selA->size() == 0 && selB->size() == 0;
    }

    Plan* TaskPlannerDeadends::poll()
    {
        setCurrentSelector();
        return currentSelector->poll();
    }

    void TaskPlannerDeadends::setCurrentSelector()
    {
        if((currentSelectorA && selA->size() == 0) || (!currentSelectorA && selB->size() == 0))
        {
            currentSelectorA = !currentSelectorA;
        }
        if(currentSelectorA)
        {
            currentSelector = selA;
            otherSelector   = selB;
        }
        else
        {
            currentSelector = selB;
            otherSelector   = selA;
        }
    }

    bool TaskPlannerDeadends::expandBasePlan(Plan* base)
    {
        if(base->expanded())
        {
            for(unsigned int i = 0; i < base->childPlans->size(); i++)
            {
                currentSelector->add(base->childPlans->at(i));
            }
            return false;
        }
        successors->computeSuccessors(base, &sucPlans);
        ++expandedNodes;
        if(successors->solution != nullptr)
        {
            solution = successors->solution;
            return false;
        }
        Plan* bestPlan = currentSelectorA ? bestPlanA : bestPlanB;
        if(bestPlan == nullptr || base->h < bestPlan->h || (base->h == bestPlan->h && base->g <= bestPlan->g))
        {
            if(currentSelectorA)
            {
                bestPlanA = base;
            }
            else
            {
                bestPlanB = base;
            }
        }
        return true;
    }

    void TaskPlannerDeadends::addSuccessors(Plan* base)
    {
        base->addChildren(sucPlans);
        for(Plan* p: sucPlans)
        {
            if(currentSelector->add(p))
            {
                if(currentSelectorA && currentSelector->getBestH() < otherSelector->getBestH())
                {
                    otherSelector->add(p);
                }
            }
        }
    }

    Plan* TaskPlannerDeadends::searchStep()
    {
        setCurrentSelector();
        Plan* base = currentSelector->poll();
        if(!expandBasePlan(base))
        {
            return nullptr;
        }
        addSuccessors(base);
        currentSelectorA = !currentSelectorA;
        return base;
    }

    void TaskPlannerDeadends::update(grstaps::Plan* base, std::vector<grstaps::Plan*>& successors)
    {
        base->addChildren(successors);
        for(Plan* p: successors)
        {
            if(currentSelector->add(p))
            {
                if(currentSelectorA && currentSelector->getBestH() < otherSelector->getBestH())
                {
                    otherSelector->add(p);
                }
            }
        }
        currentSelectorA = !currentSelectorA;
    }

    std::vector<Plan*> TaskPlannerDeadends::getNextSuccessors(Plan* base)
    {
        expandBasePlan(base);
        return sucPlans;
    }

}  // namespace grstaps

#include <iostream>

#include "grstaps/task_planning/task_planner_concurrent.hpp"

#define PLATEAU_START 100
#define PLATEAU_LIMIT 500

namespace grstaps
{
    TaskPlannerConcurrent::TaskPlannerConcurrent(SASTask *task,
                                                 Plan *initialPlan,
                                                 TState *initialState,
                                                 bool forceAtEndConditions,
                                                 bool filterRepeatedStates,
                                                 bool generateTrace,
                                                 std::vector<SASAction *> *tilActions,
                                                 TaskPlannerBase *parentPlanner,
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
        sel = new Selector();
        if(successors->informativeLandmarks() || 1.5f * initialPlan->hLand >= initialPlan->h)
        {  // Landmarks available
            // selA->addQueue(SEARCH_G_2HFF);
            // selA->addQueue(SEARCH_G_3HLAND);
            sel->addQueue(SEARCH_HFF);
            sel->addQueue(SEARCH_HLAND);
        }
        else
        {  // No landmarks available
            sel->addQueue(SEARCH_G_3HFF);
            // selB->addQueue(SEARCH_HFF);
        }
        addInitialPlansToSelectors();
        plateau  = nullptr;
        bestPlan = nullptr;
    }

    void TaskPlannerConcurrent::addInitialPlansToSelectors()
    {
        initialH = FLOAT_INFINITY;
        solution = nullptr;
        std::vector<Plan *> suc;
        successors->computeSuccessors(initialPlan, &suc);
        initialPlan->addChildren(suc);
        for(Plan *p: suc)
        {
            if(p->isSolution())
            {
                solution = p;
                // cout << "Solution found" << endl;
            }
            else
            {
                sel->add(p);
            }
            if(p->h < initialH)
                initialH = p->h;
        }
    }

    Plan *TaskPlannerConcurrent::plan()
    {
        while(solution == nullptr && !emptySearchSpace() && !timeExceed())
        {
            searchStep();
        }
        return solution;
    }

    bool TaskPlannerConcurrent::emptySearchSpace()
    {
        return sel->size() == 0;
    }

    void TaskPlannerConcurrent::update(Plan *base, std::vector<Plan *> &successors)
    {
        base->addChildren(successors);
        for(Plan *p: successors)
        {
            if(sel->add(p))
            {
                if(plateau != nullptr)
                {
                    cancelPlateauSearch(true);
                }
            }
        }
        checkPlateau();
    }

    std::vector<Plan *> TaskPlannerConcurrent::getNextSuccessors(Plan *base)
    {
        expandBasePlan(base);
        return sucPlans;
    }

    Plan *TaskPlannerConcurrent::poll()
    {
        return sel->poll();
    }

    bool TaskPlannerConcurrent::expandBasePlan(Plan *base)
    {
        if(base->expanded())
        {
            for(unsigned int i = 0; i < base->childPlans->size(); i++)
            {
                if(sel->add(base->childPlans->at(i)) && plateau != nullptr)
                {
                    cancelPlateauSearch(true);
                }
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
        if(bestPlan == nullptr || base->h < bestPlan->h || (base->h == bestPlan->h && base->g <= bestPlan->g))
        {
            bestPlan = base;
        }
        return true;
    }

    void TaskPlannerConcurrent::addSuccessors(Plan *base)
    {
        base->addChildren(sucPlans);
        for(Plan *p: sucPlans)
        {
            if(sel->add(p))
            {
                if(plateau != nullptr)
                {
                    cancelPlateauSearch(true);
                }
            }
        }
    }

    void TaskPlannerConcurrent::cancelPlateauSearch(bool improve)
    {
        plateau->exportOpenNodes(sel);
        delete plateau;
        plateau = nullptr;
    }

    void TaskPlannerConcurrent::checkPlateau()
    {
        if(sel->inPlateau(PLATEAU_START))
        {
            if(plateau == nullptr && bestPlan != nullptr && bestPlan->h <= sel->getBestH() + 1)
            {
                plateau  = new Plateau(task, bestPlan, successors, sel->getBestH(), 1);
                bestPlan = nullptr;
            }
            if(plateau != nullptr)
            {
                bool improve = plateau->searchStep(sel->getBestH() <= 6);
                if(improve || sel->inPlateau(PLATEAU_LIMIT) || plateau->empty())
                {
                    if(improve)
                    {
                        sel->setBestPlan(plateau->getBestPlan());
                    }
                    else
                    {
                        sel->setIterationsWithoutImproving(PLATEAU_START);
                    }
                    cancelPlateauSearch(improve);
                }
            }
        }
    }

    Plan *TaskPlannerConcurrent::searchStep()
    {
        Plan *base = sel->poll();
        if(!expandBasePlan(base))
        {
            return nullptr;
        }
        addSuccessors(base);
        checkPlateau();
        return base;
    }
}  // namespace grstaps

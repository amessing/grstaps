#include <iostream>

#include "grstaps/task_planning/task_planner_reversible.hpp"

#define PLATEAU_START 100
#define PLATEAU_LIMIT 500

namespace grstaps
{
    TaskPlannerReversible::TaskPlannerReversible(SASTask* task,
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
        if(tilActions != nullptr && tilActions->empty())
        {
            this->tilActions = tilActions = nullptr;
        }
        selector = new Selector();
        if(successors->informativeLandmarks() || 1.5f * initialPlan->hLand >= initialPlan->h)
        {  // Landmarks available
            if(tilActions != nullptr)
            {
                selector->addQueue(SEARCH_G_2HFF + SEARCH_PLATEAU);
                selector->addQueue(SEARCH_G_3HLAND + SEARCH_PLATEAU);
            }
            else
            {
                selector->addQueue(SEARCH_HFF);
                selector->addQueue(SEARCH_HLAND);
            }
        }
        else
        {  // No landmarks available
            selector->addQueue(SEARCH_G_3HFF);
        }
        addInitialPlansToSelectors();
        plateau  = nullptr;
        bestPlan = nullptr;
    }

    void TaskPlannerReversible::addInitialPlansToSelectors()
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
                selector->add(p);
            }
            if(p->h < initialH)
                initialH = p->h;
        }
    }

    Plan* TaskPlannerReversible::plan()
    {
        std::ofstream traceFile;
        if(generateTrace)
        {
            traceFile.open("trace.txt");
            writeTrace(traceFile, initialPlan);
        }
        while(solution == nullptr && !emptySearchSpace() && !timeExceed())
        {
            searchStep();
            if(generateTrace)
            {
                writeTrace(traceFile, base);
                if(plateau != nullptr)
                    break;
            }
        }
        if(generateTrace)
        {
            traceFile.close();
            exit(0);
        }
        return solution;
    }

    bool TaskPlannerReversible::emptySearchSpace()
    {
        return selector->size() == 0;
    }

    bool TaskPlannerReversible::expandBasePlan(Plan* base)
    {
        if(base->expanded())
        {
            for(unsigned int i = 0; i < base->childPlans->size(); i++)
            {
                if(selector->add(base->childPlans->at(i)) && plateau != nullptr)
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

    void TaskPlannerReversible::addSuccessors(Plan* base)
    {
        base->addChildren(sucPlans);
        for(Plan* p: sucPlans)
        {
            if(selector->add(p))
            {
                if(plateau != nullptr)
                {
                    cancelPlateauSearch(true);
                }
            }
        }
    }

    void TaskPlannerReversible::cancelPlateauSearch(bool improve)
    {
        (void)improve;
        plateau->exportOpenNodes(selector);
        delete plateau;
        plateau = nullptr;
    }

    void TaskPlannerReversible::checkPlateau()
    {
        if(selector->inPlateau(PLATEAU_START))
        {
            if(plateau == nullptr && bestPlan != nullptr && bestPlan->h <= selector->getBestH() + 1 &&
               tilActions == nullptr)
            {
                plateau  = new Plateau(task, bestPlan, successors, selector->getBestH(), 1);
                bestPlan = nullptr;
            }
            if(plateau != nullptr)
            {
                bool improve = plateau->searchStep(selector->getBestH() <= 6);
                if(improve || selector->inPlateau(PLATEAU_LIMIT) || plateau->empty())
                {
                    if(improve)
                    {
                        selector->setBestPlan(plateau->getBestPlan());
                    }
                    else
                    {
                        selector->setIterationsWithoutImproving(PLATEAU_START);
                    }
                    cancelPlateauSearch(improve);
                }
            }
        }
    }

    Plan* TaskPlannerReversible::searchStep()
    {
        base = selector->poll();
        if(!expandBasePlan(base))
        {
            return nullptr;
        }
        addSuccessors(base);
        checkPlateau();
        return base;
    }

    void TaskPlannerReversible::update(Plan* base, std::vector<Plan*>& successors)
    {
        base->addChildren(successors);
        for(Plan* p: successors)
        {
            if(selector->add(p))
            {
                if(plateau != nullptr)
                {
                    cancelPlateauSearch(true);
                }
            }
        }
        checkPlateau();
    }

    std::vector<Plan*> TaskPlannerReversible::getNextSuccessors(Plan* base)
    {
        expandBasePlan(base);
        return sucPlans;
    }

    Plan* TaskPlannerReversible::poll()
    {
        return selector->poll();
    }
}  // namespace grstaps

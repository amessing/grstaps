//#define DEBUG_PLATEAU_ON

#include <iostream>
#include <time.h>

#include "grstaps/task_planning//hff.hpp"
#include "grstaps/task_planning//state.hpp"
#include "grstaps/task_planning/plateau.hpp"
#include "grstaps/task_planning/selector.hpp"
#include "grstaps/task_planning/successors.hpp"
#include "grstaps/task_planning/utils.hpp"

namespace grstaps
{
    Plateau::Plateau(SASTask* sTask, Plan* initPlan, Successors* s, float h, int selectorIndex)
    {
        this->task          = sTask;
        this->initialPlan   = initPlan;
        this->successors    = s;
        this->selectorIndex = selectorIndex;
        hToImprove          = h;
        calculatePriorityGoals();
        selector = new PlateauSelector(SEARCH_G_2HAUX /* + SEARCH_PLATEAU*/);
        addOpenNodes(initPlan);
        bestPlan = nullptr;
        // cout << "|" << initPlan->h << "->" << hToImprove << ".";
    }

    Plateau::~Plateau()
    {
        delete selector;
        // cout << "X";
        /*
        if (bestPlan == nullptr) {
            if (task->numGoalsInPlateau < (int)task->getListOfGoals()->size() && task->numGoalsInPlateau < 3) {
                task->numGoalsInPlateau++;
                cout << "G" << task->numGoalsInPlateau << endl;
            }
        }*/
    }

    void Plateau::calculatePriorityGoals()
    {
        std::vector<TVarValue>* goals = task->getListOfGoals();
        std::vector<uint16_t> hValue(goals->size(), 0);
        TState* state = successors->getFrontierState(initialPlan);
        RPG rpg(state, task, successors->getForceAtEndConditions(), successors->getTILActions());
        delete state;
        for(unsigned int i = 0; i < goals->size(); i++)
        {
            hValue[i] = rpg.evaluate(goals->at(i), false);
            // cout << task->values[SASTask::getValueIndex(goals->at(i))].name << ", h = " << hValue[i] << endl;
        }
        for(unsigned int i = 0; i < (unsigned int)task->numGoalsInPlateau; i++)
        {
            unsigned int max = 0;
            for(unsigned int j = 0; j < goals->size(); j++)
            {
                if(hValue[j] > hValue[max])
                {
                    max = j;
                }
            }
            priorityGoals.push_back(goals->at(max));
            hValue[max] = 0;
        }
    }

    bool Plateau::searchStep(bool concurrent)
    {
        if(selector->size() == 0)
            return false;
        bool improve = false;
        Plan* base   = selector->poll();
        while(base->expanded())
        {
            if(selector->size() == 0)
                return false;
            base = selector->poll();
        }
        successors->setPriorityGoals(&priorityGoals);
        // if (concurrent)
        successors->computeSuccessorsConcurrent(base, &suc);
        // else successors->computeSuccessors(base, &suc);
        base->addChildren(suc);
        for(Plan* p: suc)
        {
            selector->add(p);
            if(p->h < hToImprove)
            {
                improve    = true;
                bestPlan   = p;
                hToImprove = p->h;
                if(task->numGoalsInPlateau > 1)
                    task->numGoalsInPlateau--;
                // cout << "E" << hToImprove << ".";
            }
        }
        successors->setPriorityGoals(nullptr);
        return improve;
    }

    void Plateau::addOpenNodes(Plan* p)
    {
        if(p->expanded())
        {
            if(p->childPlans->empty())
            {
                delete p->childPlans;
                p->childPlans = nullptr;
            }
            else
            {
                for(unsigned int i = 0; i < p->childPlans->size(); i++)
                {
                    addOpenNodes(p->childPlans->at(i));
                }
                return;
            }
        }
        p->hAux = p->h;
        selector->add(p);
    }
}  // namespace grstaps